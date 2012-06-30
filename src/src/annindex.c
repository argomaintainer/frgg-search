#include "frgg.h"

char *g_text;
extern off_t g_pos;
extern off_t g_len;

/* global varibles for Announce indexing */
unsigned int g_docid = 1;
unsigned int g_size = 400000;
struct dict_t **g_bucket; 
DB *g_dbp;
DBT g_key, g_data;


int
index_file(char *filename)
{
	char *word;
	struct postinglist *p;

	
	ansi_filter(filename);
	if (g_len != 0) {
		fprintf(stderr, "%d indexing %s\n", g_docid, filename);
		/* @TODO: save to cache file */
		g_pos = 0;
		while ((word = next_token())) {
			//DEBUG1("%s", word);
			p = get_postinglist(g_bucket, g_size, word);
			if (p->freq == p->size) /* is full */
				double_postinglist(p);
			addto_postinglist(p, g_docid);
		}
		g_data.size = strlen(filename) + 1;
		g_data.data = filename;		
		/* write_docid2path */
		g_dbp->put(g_dbp, NULL, &g_key, &g_data, 0);
		g_docid++;
	}
	return 0;
}


void
ann_traverse(char *directory, int (*process_func)(char *))
{
	char dotdir[PATH_MAX], sdir[PATH_MAX], filename[PATH_MAX];
	struct annheader header;
	FILE *fpdir;

	snprintf(dotdir, sizeof(dotdir), "%s/.DIR", directory);
	if ((fpdir = fopen(dotdir, "r")) == NULL)      /* no .DIR found */
		return;

	while (fread(&header, 1, sizeof(header), fpdir) == sizeof(header)) {
		//　目录/#或留言本#/, 递归 
		if ((header.flag & ANN_DIR) /* || (header.flag & ANN_GUESTBOOK) */) {
			/* TODO: save title to stack */
			snprintf(sdir, sizeof(sdir), "%s/%s", directory, header.filename);
			ann_traverse(sdir, process_func);
		} else {
			snprintf(filename, sizeof(filename), "%s/%s", directory, header.filename);
			//fprintf(stderr, "indexing %u %s %s\n ", g_docid, filename, header.title);
			process_func(filename);
		}
	}
	fclose(fpdir);
}


/**
 * @TODO
 */
int
build_ann_index(char *bname)
{
	char docid2path[PATH_MAX], indexfile[PATH_MAX];
	char annpath[PATH_MAX];
	char ndocsfile[PATH_MAX];
	int ret;
	int result = -1;
	FILE *fp;
	
	
	snprintf(annpath, sizeof(annpath), "data/0Announce/%s", bname);
	set_anndocid2path_file(docid2path, bname);
	set_annindex_file(indexfile, bname);

	/* Initialize the DB structure.*/
	ret = db_create(&g_dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		goto RETURN;
	}

	if (dbopen(g_dbp, docid2path, 1) != 0) {
		ERROR1("open db %s failed", docid2path);
		goto RETURN;		
	}

	g_bucket = new_postinglist_bucket(g_size);
	if (g_bucket == NULL) {
		ERROR1("new_postinglist_bucket size=%u failed", g_size);
		goto CLEAN_DB;
	}

	g_text = malloc(MAX_FILE_SIZE);
	if (g_text == NULL) {
		ERROR("malloc failed");
		goto CLEAN_MEM;
	}

	/* Zero out the DBTs before using them. */
	memset(&g_key, 0, sizeof(DBT));
	memset(&g_data, 0, sizeof(DBT));
	
	g_key.size = sizeof(unsigned int);
	g_key.data = &g_docid;

	/* TODO: cache, ensure the cache directory exists */
	ann_traverse(annpath, index_file);
	write_index_file(g_bucket, g_size, indexfile);
	calc_doc_weight(bname, ANNOUNCE, g_docid - 1);
	set_annndocs_file(ndocsfile, bname);
	fp = fopen(ndocsfile, "w");
	if (fp == NULL) {
		ERROR1("fopen %s failed", ndocsfile);
		goto CLEAN;
	}
	fprintf(fp, "%u", g_docid - 1);
	fclose(fp);

	/* it's ok */
	result = 0;
CLEAN:	
	free(g_text);	
CLEAN_MEM:
	free(g_bucket);
CLEAN_DB:
	if (g_dbp != NULL)
		g_dbp->close(g_dbp, 0);
RETURN:
	return result;
}


int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s boardname\n", argv[0]);
		return 0;
	}
	chdir(FRGGHOME);
	init_segment();		/* load dict */
	build_ann_index(argv[1]);
	cleanup_segment();
	return 0;
}
