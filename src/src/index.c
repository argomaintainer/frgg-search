#include "frgg.h"


struct dict_t **
new_postinglist_bucket(size_t size)
{
	struct dict_t **bucket = calloc(size, sizeof(struct dict_t *));
	if (bucket == NULL) 
		ERROR("calloc bucket failed");
	return bucket;
}


struct postinglist *
new_postinglist(size_t size)
{
	struct postinglist *p = malloc(sizeof(struct postinglist));
	if (p != NULL) {
		p->docs = calloc(size, sizeof(unsigned int));
		p->tf = calloc(size, sizeof(unsigned short));
		if (p->docs == NULL || p->tf == NULL) {
			ERROR("calloc p->docs or p->tf failed");
			return NULL;
		}
		p->size = size;
		p->freq = 0;
	} else {
		ERROR("malloc error");
	}
	return p;
}


/**
 * if term not in dict, add it
 */
struct postinglist *
get_postinglist(struct dict_t **bucket, size_t size, char *term)
{
	char *t = term;
	/* convert English word to lowercase */
	if (isalpha(*t)) {
		while (*t) {
			*t = tolower(*t);
			t++;
		}
	}
	
	unsigned int h = hash(term) % size;
	struct dict_t *entry = bucket[h];
	while (entry != NULL) {
		//DEBUG2("%s, %s", term, entry->term);
		if (strcmp(term, entry->term) == 0)
			return entry->p;
		entry = entry->next;
	}
	if (entry == NULL) {
		entry = malloc(sizeof(struct dict_t));
		if (entry == NULL) {
			ERROR("malloc failed");
			return NULL;
		}
		/* g_nterms++; */
		entry->term = term;
		entry->p = new_postinglist(INIT_POSTINGLIST_SIZE);
		entry->next = bucket[h];
		bucket[h] = entry;
	}
	return entry->p;
}


int
double_postinglist(struct postinglist *p)
{
	p->docs = realloc(p->docs, 2 * sizeof(unsigned int) * p->size);
	p->tf = realloc(p->tf, 2 * sizeof(unsigned short) * p->size);
	
	if (p->docs == NULL || p->tf == NULL) {
		ERROR("Oh shit! realloc failed");
		return -1;
	}
	p->size *= 2;
	return 0;
}


int
full_postinglist(struct postinglist *p)
{
	return p->freq == p->size;
}


void
addto_postinglist(struct postinglist *p, unsigned int id)
{
	if (p->freq != 0 && p->docs[p->freq - 1] == id) {
		p->tf[p->freq - 1]++;
	} else {
		p->docs[p->freq] = id;
		p->tf[p->freq++] = 1;
	}
}


/**
 * varible byte codes
 */
int
vb_encode(unsigned int n, unsigned char *bytes)
{
	int i;
	int largest_bit = 0;
	for (i = 0; i < sizeof(unsigned int) * 8; ++i) 
		largest_bit = ((1 << i) & n) ? i : largest_bit;
	
	int nbyte = largest_bit / 7 + 1;
	i = nbyte;
	while (n) {
		bytes[--i] = (unsigned char)(n % 128);
		n /= 128;
	}
	/* set the first bit of last byte to 1 */
	bytes[nbyte - 1] |= 0x80;
	return nbyte;
}



/**
 * pack data from postinglist into buf (marshalling)
 * do compressing using varible byte encoding
 */
size_t
pack_index_data(struct postinglist *p, char *buf)
{
	int i;
	size_t buflen = 0;
	
	memcpy(buf, &(p->freq), sizeof(p->freq));
	buflen += sizeof(p->freq);

	if (p->freq > 0) {
		buflen += vb_encode(p->docs[0], (unsigned char *)(buf + buflen));
		buflen += vb_encode(p->tf[0], (unsigned char *)(buf + buflen));
	}
	
	for (i = 1; i < p->freq; i++) {
		buflen += vb_encode(p->docs[i] - p->docs[i - 1], (unsigned char *)(buf + buflen));
		buflen += vb_encode(p->tf[i], (unsigned char *)(buf + buflen));
	}
	return buflen;
}


int
write_index_file(struct dict_t **dict, size_t size, const char *indexfile)
{
	DB *dbp;
	DBT key, value;
	struct dict_t *ent, *tmp;
	int ret, i, result = -1;
	char *databuf;
	
	
	/* Initialize the structure. This
	 * database is not opened in an environment, 
	 * so the environment pointer is NULL. */
	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		goto RETURN;
	}

	if (dbopen(dbp, indexfile, 1) != 0) {
		ERROR1("open db %s failed", indexfile);
		goto RETURN;
	}

	memset(&key, 0, sizeof(key));
	memset(&value, 0, sizeof(value));

	databuf = malloc(MAX_POSTLING_MEMORY_SIZE);
	if (databuf == NULL) {
		ERROR("malloc failed");
		goto CLEAN_DB;
	}
	/* tranverse entries */
	for (i = 0; i < size; ++i) {
		ent = dict[i];
		while (ent != NULL) {
			/* write_to_db */
			key.size = strlen(ent->term) + 1;
			key.data = ent->term;
			value.size = pack_index_data(ent->p, databuf);
			value.data = databuf;
			dbp->put(dbp, NULL, &key, &value, 0);

			/* free entry */
			free(ent->p->docs);
			free(ent->p->tf);
			free(ent->p);
			tmp = ent;
			ent = ent->next;
			free(tmp);
		}
	}
	free(databuf);
	result = 0;
CLEAN_DB:	
	if (dbp != NULL)
		dbp->close(dbp, 0);
RETURN:	
	return result;
}


/**
 *   open index db
 *   for each term:
 *	for doc,tf in term.postinglist:
 *	   W[doc] += sq(weight_d_t(tf))
 *   W[doc] = sqrt(W[doc])
 *   save W[doc] to file
 */
int
calc_doc_weight(const char *bname, int type, unsigned int ndocs)
{
	DB *dbp;
	DBC *cursorp;
	DBT key, value;
	int ret, i, j;
	float w;
	float *weight;
	char indexfile[PATH_MAX], weightfile[PATH_MAX];
	FILE *fp;
	unsigned int freq;
	unsigned int docid = 0;
	unsigned short tf;
	size_t buflen;
	unsigned char *bytes;
	unsigned int last_docid;
	unsigned int n = 0;


	if (type == ANNOUNCE) {
		set_annindex_file(indexfile, bname);
	} else {
		set_brdindex_file(indexfile, bname);
	}
	
	
	/* Initialize the DB structure. */
	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		return -1;
	}

	if (dbopen(dbp, indexfile, 0) != 0) {
		ERROR1("open db %s failed", indexfile);
		return -1;
	}

	/* Get a cursor */
	dbp->cursor(dbp, NULL, &cursorp, 0);
	
	memset(&key, 0, sizeof(key));
	memset(&value, 0, sizeof(value));

	weight = calloc(ndocs + 1, sizeof(float));
	if (weight == NULL) {
		ERROR("calloc failed");
		if (dbp != NULL)
			dbp->close(dbp, 0);
		return -1;
	}
	unsigned int pst_cnt=0;

	/* Iterate over the database, retrieving each record in turn. */
	while ((ret = cursorp->get(cursorp, &key, &value, DB_NEXT)) == 0) {
		freq = *((unsigned int *)value.data);
		buflen = sizeof(freq);
		
		/* decompress data encoded by varible byte encoding, see vb_encode in index.c */
		bytes = (unsigned char *)(value.data + buflen);
		i =  j = n = docid = last_docid = 0;
		while (j < freq) {
			if (bytes[i] & 0x80) {
				n = 128 * n + (bytes[i] & ~0x80); /* remove flag bit */
				/* got a value */
				if (docid == 0) {
					docid = n + last_docid;
				} else {
					tf = n;
					w = weight_d_t(tf);
					weight[docid] += w * w;
					/* reset docid, indicate calc docid next time */
					last_docid = docid;
					docid = 0;
					j++;
				}
				n = 0;
			} else {
				n = 128 * n + bytes[i];
			}
			i++;
		}
		pst_cnt++;
	}
	cursorp->close(cursorp);
	dbp->close(dbp, 0);

	fprintf(stderr, "%u postinglists\n", pst_cnt);
	/* write document weight to file */
	if (type == ANNOUNCE) {
		set_annweight_file(weightfile, bname);
	} else {
		set_brdweight_file(weightfile, bname);
	}
	
	fp = fopen(weightfile, "w");
	if (fp == NULL) {
		ERROR("fopen failed");
		free(weight);
		return -1;
	}
	for (i = 1; i <= ndocs; ++i) {
		fwrite(&i, sizeof(i), 1, fp);
		weight[i] = sqrt(weight[i]);
		fwrite(weight + i, sizeof(weight[0]), 1, fp);
		//fprintf(stderr, "%d %f\n", i, weight[i]);  // DEBUG
	}
	free(weight);
	fclose(fp);
	return 0;
}
