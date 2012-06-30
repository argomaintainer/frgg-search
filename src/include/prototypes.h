/* ansifileter.c */
char *ansi_fgets(char *buf, size_t size, FILE *fp);
void ansi_filter(char *filename);


/* dict.c */
void load_dict();
void dict_add_word(char *word);
char *dict_get_word(char *str, size_t len);
void dict_add_char(char *ch, unsigned short freq);
unsigned int dict_get_char_freq(char *ch);
void destroy_dict();


/* log.c */
void do_log(int log_type, const char *fmt, ...);
void do_err(const char *func, const char *file, int line, const char *fmt, ...);
void do_debug(const char *func, const char *file, int line, const char *fmt, ...);


/* segment.c */
int chunk_len(struct chunk *chk);
double chunk_avglen(struct chunk *chk);
double chunk_variance(struct chunk *chk);
double chunk_mor_free_degree(struct chunk *chk);
double mm_cmp(struct chunk *c1, struct chunk *c2);
double lawl_cmp(struct chunk *c1, struct chunk *c2);
double svwl_cmp(struct chunk *c1, struct chunk *c2);
double lsdmfocw_cmp(struct chunk *c1, struct chunk *c2);
void choose_best_chunk(double (*cmp)(struct chunk *c1, struct chunk *c2));
void create_chunks();

void init_segment();
void cleanup_segment();
int get_next_words(char *words[MAX_NEXT_WORDS]);
int addto_stopword(char *stop);
char *get_ch_word();
char *get_ascii_word();
char *next_token();



/* formula.c */
float weight_d_t(unsigned short tf);
float weight_q_t(unsigned int df, unsigned int ndocs);


/* index.c */
struct dict_t **new_postinglist_bucket(size_t size);
struct postinglist *new_postinglist(size_t size);
struct postinglist *get_postinglist(struct dict_t **bucket, size_t size, char *term);
int double_postinglist(struct postinglist *p);
int full_postinglist(struct postinglist *p);
void addto_postinglist(struct postinglist *p, unsigned int id);
int vb_encode(unsigned int n, unsigned char *bytes);
size_t pack_index_data(struct postinglist *p, char *buf);
int write_index_file(struct dict_t **dict, size_t size, const char *indexfile);
int calc_doc_weight(const char *bname, int type, unsigned int ndocs);


/* db.c */
int dbopen(DB *dbp, const char *db_name, int create);


/* search.c */
void search(char *bname, char *query_str, int type, int start);


/* fileop.c */
int f_mkdir(char *path, int omode);

/* string.c */
unsigned int hash(const char *str);
unsigned int nhash(const char *str, int len);
size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);
