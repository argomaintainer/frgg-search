#include "frgg.h"

char *g_text;
extern off_t g_pos;
extern off_t g_len;


// single pass in-memory index
/**1 output_file = new file()
  2 dict =  new hash()
  3 while (free memory available)
  4 do token = next_token()
  5     if token not in dict
  6  	   postinglist = addtodict(dict, token)
  7     else postinglist = getpostinglist(dict, token)
  8     if full(postinglist)
  9   	   postinglist = doublepostinglist(dict, token)
  10    addtopostinglist(postinglist, docid(token))
  11 sorted_terms = sortterm(dict)	// for merge purpose 
 *12 writeblock(sorted_terms, dict, output_file)
 */
int
build_board_index(char *bname)
{
	char *word;
	char dirfile[PATH_MAX], docid2path[PATH_MAX], indexfile[PATH_MAX];
	char filepath[PATH_MAX]; /* article file path */
	char filename[20];
	char cachedir[PATH_MAX];
	char cachefile[PATH_MAX];
	char ndocsfile[PATH_MAX];
	DB *dbp;
	DBT key, data;
	int ret;
	int result = -1;
	FILE *filelist, *fp;
	struct postinglist *p;
	unsigned int docid = 1;
	gzFile cachefp;
	int gzerr;

	
	setboardfile(dirfile, bname, bname);
	set_brddocid2path_file(docid2path, bname);
	set_brdindex_file(indexfile, bname);

	/* Initialize the  DB structure.*/
	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		goto RETURN;
	}

	if (dbopen(dbp, docid2path, 1) != 0) {
		ERROR1("open db %s failed", docid2path);
		goto RETURN;		
	}
		
	if (!(filelist = fopen(dirfile, "r"))) {
		ERROR1("open file %s failed", dirfile);
		goto CLEAN_DB;
	}
	
	size_t size = 300000;	/* TODO: define this constant */
	struct dict_t **bucket = new_postinglist_bucket(size);
	if (bucket == NULL) {
		ERROR1("new_dict size=%u failed", size);
		goto CLEAN_FP;
	}

	g_text = malloc(MAX_FILE_SIZE);
	if (g_text == NULL) {
		ERROR("malloc failed");
		goto CLEAN_MEM;
	}

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	
	key.size = sizeof(unsigned int);
	key.data = &docid;
	
	/* ensure the cache directory exists */
	setcachepath(cachedir, bname);
	f_mkdir(cachedir, 0755);

	while (fgets(filename, sizeof(filename), filelist)) {
		filename[strlen(filename) - 1] = '\0';
	
		data.size = strlen(filename) + 1;
		data.data = filename;
		
		setboardfile(filepath, bname, filename);
		ansi_filter(filepath);
		
		if (g_len != 0) {
			fprintf(stderr, "%d indexing %s\n", docid, filename);
			/* save to cache file */
			setcachefile(cachefile, bname, filename);
			cachefp = gzopen(cachefile, "wb");
			if (cachefp != NULL) {
				if (gzwrite(cachefp, g_text, g_len) != g_len) 
					ERROR(gzerror(cachefp, &gzerr));
				gzclose(cachefp);
			}
			
			g_pos = 0;
			while ((word = next_token())) {
				//DEBUG1("%s", word);
				p = get_postinglist(bucket, size, word);
				if (p->freq == p->size) /* is full */
					double_postinglist(p);
				addto_postinglist(p, docid);
			}
			
			/* write_docid2path */
			dbp->put(dbp, NULL, &key, &data, 0);
			docid++;
		}
	}

	write_index_file(bucket, size, indexfile);
	calc_doc_weight(bname, BOARD, docid - 1);
	set_brdndocs_file(ndocsfile, bname);
	fp = fopen(ndocsfile, "w");
	if (fp == NULL) {
		ERROR1("fopen %s failed", ndocsfile);
		goto CLEAN;
	}
	fprintf(fp, "%u", docid - 1);
	fclose(fp);

	/* it's ok */
	result = 0;
CLEAN:	
	free(g_text);	
CLEAN_MEM:
	free(bucket);
CLEAN_FP:
	fclose(filelist);	
CLEAN_DB:
	if (dbp != NULL)
		dbp->close(dbp, 0);
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
	build_board_index(argv[1]);
	cleanup_segment();
	return 0;
}
