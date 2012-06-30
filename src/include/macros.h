/* general purposed marcos */

#define Ctrl(ch)	(ch & 037)

#define isprint2(c) (((c) & 0xe0) && ((c)!=0x7f))


/* bbs purposed marcos */
#define clear_line(l) { move(l, 0); clrtoeol(); }



#define setboardfile(buf, bname, filename) \
	sprintf(buf, "data/boards/%s/%s", bname, filename);

#define setannfile(buf, bname, filename) \
	sprintf(buf, "data/0Announce/%s/%s", bname, filename);

#define setcachepath(buf, bname) \
	sprintf(buf, "cache/%s", bname);

#define setcachefile(buf, bname, filename) \
	sprintf(buf, "cache/%s/%s.gz", bname, filename);

#define set_brddocid2path_file(buf, bname) \
	sprintf(buf, "index/%s.brd.docid2path",  bname);	/* docid to disk location */

#define set_anndocid2path_file(buf, bname) \
	sprintf(buf, "index/%s.ann.docid2path",  bname);	/* docid to disk location */

#define set_brdindex_file(buf, bname) \
	sprintf(buf, "index/%s.brdidx", bname);

#define set_annindex_file(buf, bname) \
	sprintf(buf, "index/%s.annidx", bname);

#define set_brdweight_file(buf, bname) \
	sprintf(buf, "index/%s.brd.weight",  bname);		/* document weight, unsigned int -> float */

#define set_annweight_file(buf, bname) \
	sprintf(buf, "index/%s.ann.weight",  bname);		/* document weight, unsigned int -> float */

#define set_brdndocs_file(buf, bname) \
	sprintf(buf, "index/%s.brdndocs", bname);

#define set_annndocs_file(buf, bname) \
	sprintf(buf, "index/%s.annndocs", bname);

#define set_config_file(buf, filename) \
	sprintf(buf, "etc/%s", filename);



/* C99 provides __func__ as the name of the current function.
   Before this, GCC had the extension __FUNCTION__.   Otherwise,
   we don't attempt to determine it. */
#if __STDC_VERSION__ >= 199901L
   #define FUNC_NAME __func__
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
   #define FUNC_NAME __FUNCTION__
#else
   #define FUNC_NAME "(unknown function)"
#endif


/* What C89 provides */
#define ERROR(format) \
	do_err(FUNC_NAME, __FILE__, __LINE__, format)

#define ERROR1(format, arg1)	\
	do_err(FUNC_NAME, __FILE__, __LINE__, format, arg1)

#define ERROR2(format, arg1, arg2)	\
	do_err(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2)

#define ERROR3(format, arg1, arg2, arg3)	\
	do_err(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2, arg3)

#define ERROR4(format, arg1, arg2, arg3, arg4)	\
	do_err(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2, arg3, arg4)



#define DEBUG(format) \
	do_debug(FUNC_NAME, __FILE__, __LINE__, format)

#define DEBUG1(format, arg1)	\
	do_debug(FUNC_NAME, __FILE__, __LINE__, format, arg1)

#define DEBUG2(format, arg1, arg2)	\
	do_debug(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2)

#define DEBUG3(format, arg1, arg2, arg3)	\
	do_debug(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2, arg3)

#define DEBUG4(format, arg1, arg2, arg3, arg4)	\
	do_debug(FUNC_NAME, __FILE__, __LINE__, format, arg1, arg2, arg3, arg4)

