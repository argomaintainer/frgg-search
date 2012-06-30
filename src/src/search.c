#include "frgg.h"

char *g_text;
extern size_t g_len;

char *g_termlist[256];	/* query terms after sementation */
int g_nterm = 0;	/* number of query terms */


size_t
get_number_of_docs(const char *bname, int type)
{
	char ndocsfile[PATH_MAX];
	FILE *fp;
	unsigned int n_docs;

	if (type == BOARD) 
		snprintf(ndocsfile, sizeof(ndocsfile), "index/frgg.%s.brdndocs", bname);
	else
		snprintf(ndocsfile, sizeof(ndocsfile), "index/frgg.%s.annndocs", bname);

	fp = fopen(ndocsfile, "r");
	if (fp == NULL) {
		ERROR1("fopen %s failed", ndocsfile);
		return -1;
	}
	fscanf(fp, "%u", &n_docs);
	fclose(fp);
	return n_docs;
}


static unsigned int g_docidlist[MAX_RETURN_DOCS];
static int g_cnt;
float *g_score;		/* score for each document */
short *g_match;		/* number of terms match for each document */


int
cmpdoc(const void *ptr1, const void *ptr2)
{
	unsigned int doc1 = *(unsigned int*)ptr1;
	unsigned int doc2 = *(unsigned int*)ptr2;
	
	float match_diff = g_match[doc2] - g_match[doc1];
	float score_diff = g_score[doc2] - g_score[doc1];
	if (match_diff != 0) 
		return match_diff;
	else if (score_diff < 0)
		return -1;
	else if (score_diff > 0)
		return 1;
	return 0;
}


/**
 * 1. allocate a accumator A(d) for each document d, set A(d) <- 0
 * 2. for each query term t in q,
 * 	(a). calculate w(q, t), and fetch postinglist for t.
 *	(b). For each pair <d, tf(d, t)> in postinglist
 * 		calc w(d, t), and
 *		Ad = Ad + w(d, t) * w(q, t)
 * 3. Read the array of W(d) values.
 * 4. For each A(d) > 0, set Ad = A(d) / W(d)
 * 5. Identify the r greatest S(d) values and return the correspoding documents
 */
int
ranking(const char *bname, int type)
{
	DB *dbp;
	DBT key, value;
	int ret, i, j, b;
	int result = -1;	/* return value */
	size_t buflen;
	unsigned int freq;
	unsigned int docid;
	unsigned int last_docid;
	unsigned int n;
	unsigned short tf;
	char indexfile[PATH_MAX];
	char weightfile[PATH_MAX];
	size_t n_docs;
	unsigned char *bytes;
	FILE *fp;
	float wqt;

	if (type == BOARD)
		snprintf(indexfile, sizeof(indexfile), "index/frgg.%s.brdidx", bname);
	else
		snprintf(indexfile, sizeof(indexfile), "index/frgg.%s.annidx", bname);

	
	/* Initialize the DB structure.*/
	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		goto RETURN;
	}

	if (dbopen(dbp, indexfile, 0) != 0) {
		ERROR1("open db %s failed", indexfile);
		goto RETURN;
	}

	memset(&key, 0, sizeof(key));
	memset(&value, 0, sizeof(value));

	n_docs = get_number_of_docs(bname, type);
	if (n_docs <= 0) {
		ERROR1("ndocs = %u\n", n_docs);
		goto CLEAN_DB;
	}
	
	g_score = calloc(n_docs + 1, sizeof(float));
	g_match = calloc(n_docs + 1, sizeof(short));
	if (g_score == NULL || g_match == NULL) {
		ERROR("calloc failed");
		goto CLEAN_DB;
	}
	
	for (i = 0; i < g_nterm; i++) {
		do_log(LOG_TRACE, "%u fetching postlist for %s", time(NULL), g_termlist[i]);
		key.size = strlen(g_termlist[i]) + 1;
		key.data = g_termlist[i];
		ret = dbp->get(dbp, NULL, &key, &value, 0);
		if (ret != 0) {
			do_log(LOG_TRACE, "no postlints found for %s", g_termlist[i]);
			continue;
		}
		
		freq = *((unsigned int *)value.data);
		buflen = sizeof(freq);
		//fprintf(stderr, "%s doc freq: %u\n", term, freq);

		wqt = weight_q_t(freq, n_docs);
		/* decompress data encoded by varible byte encoding, see vb_encode@index.c */
		bytes = (unsigned char *)(value.data + buflen);
		n = 0, b = 0, j = 0, docid = 0, last_docid = 0;
		while (j < freq) {
			if (bytes[b] & 0x80) {	/* last byte, so we got a value */
				n = 128 * n + (bytes[b] & ~0x80); /* remove flag bit */
				if (docid == 0) {
					docid = last_docid + n;
					last_docid = docid;
				} else {
					tf = n;
					g_score[docid] += weight_d_t(tf) * wqt;
					g_match[docid]++;
					/* reset docid to indicate calc docid next time */
					docid = 0;
					j++;
				}
				n = 0;
			} else {
				n = 128 * n + bytes[b];
			}
			b++;
		}
	}

	/* document weight */
	if (type == BOARD)
		snprintf(weightfile, sizeof(weightfile), "index/frgg.%s.brd.weight",  bname);		/* document weight, unsigned int -> float */
	else
		snprintf(weightfile, sizeof(weightfile), "index/frgg.%s.ann.weight",  bname);		/* document weight, unsigned int -> float */
	
	fp = fopen(weightfile, "r");
	if (fp == NULL) {
		ERROR1("fopen %s failed", weightfile);
		goto CLEAN_MEM;
	}
	
	float *Weight = malloc(sizeof(float) * (n_docs + 1));
	if (Weight == NULL) {
		ERROR("malloc failed");
		goto CLEAN_FP;
	}
	
	/* read document weight */
	while (fread(&docid, sizeof(docid), 1, fp)) 
		fread(Weight + docid, sizeof(float), 1, fp);

	g_cnt = 0;
	for (i = n_docs; i >= 1; --i) {
		if (g_match[i] == g_nterm) {
			g_score[i] /= Weight[i];
			if (g_cnt == MAX_RETURN_DOCS)
				break;
			g_docidlist[g_cnt++] = i;
		}
	}
	if (g_cnt == 0 && g_nterm > 3) {
		for (i = n_docs; i >= 1; --i) {
			if (g_match[i] == g_nterm - 1) {
				g_score[i] /= Weight[i];
				if (g_cnt == MAX_RETURN_DOCS)
					break;
				g_docidlist[g_cnt++] = i;
			}
		}
	}

	/* sorting */
	qsort(g_docidlist, g_cnt, sizeof(unsigned int), cmpdoc);
	
	/* DEBUG */
//	for (i = 0; i < g_cnt; ++i) {
//		printf("%d %f\n", g_docidlist[i], g_score[g_docidlist[i]]);
//	}
//	/* ^^^ DEBUG ^^^^ */

	result = 0;
	
	free(Weight);
CLEAN_FP:	
	fclose(fp);
CLEAN_MEM:	
	free(g_match);
	free(g_score);
CLEAN_DB:
	if (dbp != NULL)
		dbp->close(dbp, 0);
RETURN:	
	return result;
}

	
int g_fname_cnt = 0;
char *g_filename[MAX_PAGE_SIZE];		


int
gen_result(char *bname, int type, int start)
{
	DB *dbp;
	DBT key, value;
	int ret, i;
	char docid2path[PATH_MAX];

	if (type == BOARD)
		snprintf(docid2path, sizeof(docid2path), "index/frgg.%s.brd.docid2path",  bname);	/* docid to disk location */
	else
		snprintf(docid2path, sizeof(docid2path), "index/frgg.%s.ann.docid2path",  bname);	/* docid to disk location */
	
	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
		ERROR("create db hanldle failed");
		return -1;
	}

	if (dbopen(dbp, docid2path, 0) != 0) {
		ERROR1("open db %s failed", docid2path);
		return -1;
	}

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));
	memset(&value, 0, sizeof(DBT));
	
	key.size = sizeof(unsigned int);
	g_fname_cnt = 0;

	for (i = start; i < g_cnt; ++i) {
		key.data = &g_docidlist[i];
		dbp->get(dbp, NULL, &key, &value, 0);
		g_filename[g_fname_cnt++] = strdup((char *)value.data);
		if (g_fname_cnt == MAX_PAGE_SIZE)
			break;
		//printf("%u %s\n", g_docidlist[i], g_filename[g_fname_cnt-1]);
	}
	if (dbp != NULL) 
		dbp->close(dbp, 0);
	return 0;
}


/* TODO: read it from html file */
void
html_header(char *query_str)
{
	printf("Content-type: text/html; charset=gbk\n\n");
	puts("<html><head>");
	puts("<meta http-equiv='content-type' content='text/html;charset=gbk'>");
	puts("<STYLE><!--");
	puts("body,td,.p1,.p2,.i{font-family:arial}");
	puts("body{margin:6px 0 0 0;background-color:#fff;color:#000;}");
	puts("table{border:0}");
	puts("TD{FONT-SIZE:9pt;LINE-HEIGHT:18px;}");
	puts(".f14{FONT-SIZE:14px}");
	puts(".f10{font-size:10.5pt}");
	puts(".f16{font-size:16px;font-family:Arial}");
	puts(".c{color:#7777CC;}");
	puts(".p1{LINE-HEIGHT:120%;margin-left:-12pt}");
	puts(".p2{width:100%;LINE-HEIGHT:120%;margin-left:-12pt}");
	puts(".i{font-size:16px}");
	puts(".t{COLOR:#0000cc;TEXT-DECORATION:none}");
 	puts("a.t:hover{TEXT-DECORATION:underline}");
	puts(".p{padding-left:18px;font-size:14px;word-spacing:4px;}");
	puts(".f{line-height:120%;font-size:100%;width:35em;padding-left:15px;word-break:break-all;word-wrap:break-word;}");
	puts(".h{margin-left:8px;width:100%}");
	puts(".s{width:8%;padding-left:10px; height:25px;}");
	puts(".m,a.m:link{COLOR:#77c;font-size:100%;}");
	puts("a.m:visited{COLOR:#77c;}");
	puts(".g{color:#008000; font-size:12px;}");
	puts(".r{ word-break:break-all;cursor:hand;width:238px;}");
	puts(".bi {background-color:#D9E1F7;height:20px;margin-bottom:12px}");
	puts(".pl{padding-left:3px;height:8px;padding-right:2px;font-size:14px;}");
	puts(".Tit{height:21px; font-size:14px;}");
	puts(".fB{ font-weight:bold;}");
	puts(".mo,a.mo:link,a.mo:visited{COLOR:#666666;font-size:100%;line-height:10px;}");
	puts(".htb{margin-bottom:5px;}");
	puts("#ft{clear:both;line-height:20px;background:#E6E6E6;text-align:center}");
	puts("#ft,#ft *{color:#77C;font-size:12px;font-family:Arial}");
	puts("#ft span{color:#666}");
	puts("form{margin:0;position:relative;z-index:9}");
	/* <!--navbar > */
	puts("#navbar A:link{	COLOR: blue;		TEXT-DECORATION: underline}");
	puts("#navbar A:visited	{COLOR: blue;	TEXT-DECORATION: underline}");
	puts("#navbar DIV {DISPLAY: block;padding-top: 0.1em;WIDTH: 2.2em;HEIGHT: 1.4em;TEXT-ALIGN: center}");
	puts("#navbar DIV.navbarpagenumber {BORDER: #c4e4e4 1px solid; CURSOR: pointer;	BACKGROUND-COLOR: #f4f4f4;}");
	puts("#navbar DIV.navbarcurrentpagenumber {BORDER: #c4e4e4 1px solid;	FONT-WEIGHT: bold;COLOR: #a90a08;BACKGROUND-COLOR: white}");
        puts("--></STYLE>");
	printf("<title>%s - frgg搜索</title>", query_str);
	printf("</head>");
}


char *
urlencode(char *str)
{
	char *s = str;
	while (*s) {
		if (*s == ' ')
			*s = '+';
		s++;
	}
	return str;
}


void
html_footer(char *query_str, char *bname, int type, int start)
{
	int i;
	int curr_page = start / MAX_PAGE_SIZE + 1;
	int total_page = g_cnt / MAX_PAGE_SIZE + (g_cnt % MAX_PAGE_SIZE != 0);
	int start_page = curr_page - 10;
	if (start_page < 1)
		start_page = 1;
	
	int end_page = curr_page + 9;
	if (end_page > total_page)
		end_page = total_page;
	
	
	puts("<br><center> <!-- navi bar -->");
	puts("<script type='text/javascript'>");
	puts("function setHoverColor(e){ e.style.backgroundColor='#c4e4e4';");
	puts("}");
	puts("function unsetHoverColor(e)");
	puts("{	e.style.backgroundColor='#f4f4f4';}");
	puts("</script>");
	puts("<table id='navbar' style='font-size:83%' cellspacing='0' border='0'>");
	puts("<tr>");
	if (curr_page > 1) { /* last page */
		puts("<td><div style='width:5em; border:none'>");
		printf("<nobr><a href='/cgi-bin/search?q=%s&b=%s&t=%s&start=%d'>上一页</a>", urlencode(query_str), bname, (type == BOARD) ? "b" : "a", (curr_page - 2) * MAX_PAGE_SIZE);
		puts("</div></td>");
	}
	for (i = start_page; i <= end_page; ++i) {
		if (i == curr_page)
			printf("<td><div class='navbarcurrentpagenumber'>%d</div></td>", i);
		else
			printf("<td><a href='/cgi-bin/search?q=%s&b=%s&t=%s&start=%d'><div class='navbarpagenumber' onmouseover='setHoverColor(this)' onmouseout='unsetHoverColor(this)'>%d</div></a></td>", urlencode(query_str), bname, (type == BOARD) ? "b" : "a", (i - 1) * MAX_PAGE_SIZE, i);
	}
	if (curr_page != end_page && end_page != 0) {
		printf("<td><div style='width:5em; border:none'>");
		printf("<nobr><a href='/cgi-bin/search?q=%s&b=%s&t=%s&start=%d'>下一页</a></nobr>", urlencode(query_str), bname, (type == BOARD) ? "b" : "a", curr_page * MAX_PAGE_SIZE);
		printf("</div></td>");
	}
	puts("</tr></table><br></center><br>");
		
	puts("<center><p style='font-size:75%'><a href='/'>首页</a> | <a href='/help.html'>帮助</a> | <a href='/about.html'>关于</a> | <a href='http://bbs.sysu.edu.cn'>逸仙时空</a><br><font color='#949494'>&copy; 2009 <a href='mailto:kofreestyler@gmail.com' style='text-decoration: none; color:#949494''>freestyler@argo</a></font></p></center>");

	puts("</body>");
	puts("</html>");
}


int
garbage_line(char *str)
{
	while (*str) {
		if (*str == ' ' || *str == '\t') {
			str++;
		} else if (*(str + 1) && strncmp(str, "　", 2) == 0) {
			str += 2;
		} else {
			break;
		}
	}
	if (*str == '\n' || *str == '\r' || *str == '\0')
		return 1;
	else
		return 0;
}


/* strip leading white space */
char *
lstrip(char *str)
{
	while (*str) {
		if (*str == ' ' || *str == '\t') {
			str++;
		} else if (strncmp(str, "　", 2) == 0) { /* Chinese whitespace character */
			str += 2;
		} else {
			break;
		}
	}
	return str;
}


char *
emphasize(char *line, char *output)
{
	/* <font color=#C60A00>%s</font>, word */
	int len = strlen(line);
	int i = 0, j;
	int candidate = 0;
	
	output[0] = '\0';
	while (i < len) {
		candidate = 0;
		for (j = 0; j < g_nterm; ++j) {
			int term_len = strlen(g_termlist[j]);
			if (strncasecmp(g_termlist[j], line + i, term_len) == 0) {
				if (term_len > candidate)
					candidate = term_len;
			}
		}
		if (candidate > 0) {
			strcat(output, "<font color=#C60A00>");
			strncat(output, line + i, candidate);
			strcat(output, "</font>");
			i += candidate;
		} else if (line[i] & 0x80) {
			if (i + 1 < len) {
				strncat(output, line + i, 2);
				i += 2;
			} else {
				break;
			}
		} else {
			strncat(output, line + i, 1);
			i++;
		}
	}
	return output;
}


int
no_queryterm_line(char *line)
{
	int i;
	for (i = 0; i < g_nterm; ++i) {
		if (strcasestr(line, g_termlist[i]) != NULL)
			return 0;
	}
	return 1;
}



void show_board_result(char *bname)
{
	char filename[PATH_MAX];
	int i, len;
	int nline;
	char line[512], output[1024];
	char title[TITLELEN + 1], author[IDLEN + 1];
	char *end;
	gzFile fp;

	
	for (i = 0; i < g_fname_cnt; i++) {
		puts("<table border='0' cellpadding='0' cellspacing='0' id='1'><tr><td class=f>");
		
		/* 生成摘要 here */
		setcachefile(filename, bname, g_filename[i]);
		fp = gzopen(filename, "rb");
		if (fp == NULL) {
			ERROR1("gzopen %s failed", filename);
			continue;
		}

		/* get author */
		strcpy(author, "-");
		gzgets(fp, line, sizeof(line));
		if (strncmp(line, "发信人: ", 8) == 0) {
			end = strchr(line + 8, ' ');
			if (end != NULL) {
				len = end - line - 8;
				strncpy(author, line + 8, len);
				author[len] = '\0';
			}
		}

		strcpy(title, "无标题文档");
		/* get title */
		gzgets(fp, line, sizeof(line));
		if (strncmp(line, "标  题: ", 8) == 0) {
			len = strlen(line) - 8 - 1;	/* remove tailing '\n' */
			strncpy(title, line + 8, len);
			title[len] = '\0';
		}
		
		char link[256], cache[256];
		snprintf(link, sizeof(link), "http://bbs.sysu.edu.cn/bbscon?board=%s&file=%s", bname, g_filename[i]);
		snprintf(cache, sizeof(cache), "/cgi-bin/cache?board=%s&file=%s", bname, g_filename[i]);

		printf("<a href='%s' target='_blank'><font size='3'>%s</font></a>&nbsp;&nbsp;&nbsp;-&nbsp;<font color=#696969>%s</font>",  link, emphasize(title, output), author);
		puts("<br><font size='-1'>");

		nline = 2;
		while (nline--) { /* 2 unused lines */
			if (gzgets(fp, line, sizeof(line)) == NULL)
				break;
		}
		
		int query_term_meet = 0;
		nline = 3;
		while (nline--) {
			if (gzgets(fp, line, sizeof(line)) == NULL)
				break;
			/* skip empty line or line with no query term */
			if (strlen(line) == 0 || garbage_line(line) || (query_term_meet == 0 && no_queryterm_line(line))) {
				nline++;
			} else {
				query_term_meet = 1;
				/* strip leading white space */
				emphasize(line, output);
				fputs(lstrip(output), stdout);
				if (nline == 0)
					fputs(" ...", stdout);
				puts("<br>");
			}
		}
		
		/* if no line printed, just ouput the first 2/3 lines of body */
		if (nline == 2) {
			gzrewind(fp);
			/* skip article header */
			int skip_lines = 4;
			while (skip_lines--) { 
				if (gzgets(fp, line, sizeof(line)) == NULL) {
					break;
				}
			}
			nline = 3;
			while (nline--) {
				if (gzgets(fp, line, sizeof(line)) == NULL)
					break;
				/* skip empty line or line with no query term */
				if (strlen(line) == 0 || garbage_line(line)) {
					nline++;
				} else {
					/* strip leading white space */
					emphasize(line, output);
					fputs(lstrip(output), stdout);
					if (nline == 0)
						fputs(" ...", stdout);
					puts("<br>");
				}
			}
		}
		gzclose(fp);
		
		printf("<font color=#008000>%s</font>", link);
		printf(" - <a href='%s' target='_blank' class=m>cached</a>", cache);
		puts("<br></font></td></tr></table><br>");
	}
}


int
get_telnet_path(char *fpath, char *telnetpath, size_t size)
{
	char dotdir[PATH_MAX], curdir[PATH_MAX];
	char filename[128], buf[128];
	struct annheader header;
	FILE *fpdir;
	int cnt;
	

	snprintf(curdir, sizeof(curdir), "data/0Announce/");
	fpath = fpath + strlen("data/0Announce/");

	int i = 0, j = strlen(curdir);
	while (fpath[i] != '\0' && fpath[i] != '/') {
		curdir[j++] = fpath[i++];
	}
	curdir[j] = '\0';
	fpath = fpath + i + 1;
	
	strcpy(telnetpath, "x");
	while (1) {
		snprintf(dotdir, sizeof(dotdir), "%s/.DIR", curdir);
		
		if ((fpdir = fopen(dotdir, "r")) == NULL) {
			ERROR1("no .DIR found in %s", dotdir);
			return -1;
		}

		i = 0, j = 0;
		while (fpath[i] != '\0' && fpath[i] != '/') {
			filename[j++] = fpath[i++];
		}
		filename[j] = '\0';
		fpath = fpath + i + 1;
		
		cnt = 1;
		while (fread(&header, 1, sizeof(header), fpdir) == sizeof(header)) {
			if (strcmp(header.filename, filename) == 0) {
				snprintf(buf,sizeof(buf), " - %d", cnt);
				strlcat(telnetpath, buf, size);
				break;
			}
			cnt++;
		}
		fclose(fpdir);
		if (!(header.flag & ANN_DIR))
			break;
		else {
			strlcat(curdir, "/", sizeof(curdir));
			strlcat(curdir, header.filename, sizeof(curdir));
		}
	}
	return 0;
}


void
show_ann_result()
{
	int i, len;
	int nline;
	char line[512], output[1024];
	char title[TITLELEN + 1], author[IDLEN + 1];
	char telnetpath[128];
	char link[512];	
	char *end;
	FILE *fp;
	
	
	for (i = 0; i < g_fname_cnt; i++) {
		/* 生成摘要 here */
		fp = fopen(g_filename[i], "r");
		if (fp == NULL) {
			ERROR1("fopen %s failed", g_filename[i]);
			continue;
		}

		/* get author */
		strcpy(author, "无名氏");
		ansi_fgets(line, sizeof(line), fp);
		
		if (strncmp(line, "发信人: ", 8) == 0) {
			end = strchr(line + 8, ' ');
			if (end != NULL) {
				len = end - line - 8;
				strncpy(author, line + 8, len);
				author[len] = '\0';
			}
		}

		strcpy(title, "无标题文档");
		/* get title */
		ansi_fgets(line, sizeof(line), fp);
		if (strncmp(line, "标  题: ", 8) == 0) {
			len = strlen(line) - 8 - 1;	/* remove tailing '\n' */
			strncpy(title, line + 8, len);
			title[len] = '\0';
		}

		puts("<table border='0' cellpadding='0' cellspacing='0' id='1'><tr><td class=f>");
		
		/* http://bbs.sysu.edu.cn/bbsanc?path=boards/Joke/D.1221483750.A/D.1221483765.A/D.1228045736.A/D.1229310219.A/M.1229310278.A */
		snprintf(link, sizeof(link), "http://bbs.sysu.edu.cn/bbsanc?path=boards%s", g_filename[i] + strlen("data/0Announce"));
		printf("<a href='%s' target='_blank'><font size='3'>%s</font></a>&nbsp;&nbsp;&nbsp;-&nbsp;<font color=#696969>%s</font>",  link, emphasize(title, output), author);
		
		puts("<br><font size='-1'>");

		nline = 2;
		while (nline--) { /* 2 unused lines */
			if (ansi_fgets(line, sizeof(line), fp) == NULL)
				break;
		}
		
		int query_term_meet = 0;
		nline = 3;
		while (nline--) {
			if (ansi_fgets(line, sizeof(line), fp) == NULL)
				break;
			/* skip empty line or line with no query term */
			if (strlen(line) == 0 || garbage_line(line) || (query_term_meet == 0 && no_queryterm_line(line))) {
				nline++;
			} else {
				query_term_meet = 1;
				/* strip leading white space */
				emphasize(line, output);
				fputs(lstrip(output), stdout);
				if (nline == 0)
					fputs(" ...", stdout);
				puts("<br>");
			}
		}
		
		/* if no line printed, just ouput the first 2/3 lines of body */
		if (nline == 2) {
			rewind(fp);
			/* skip article header */
			int skip_lines = 4;
			while (skip_lines--) { 
				if (ansi_fgets(line, sizeof(line), fp) == NULL) {
					break;
				}
			}
			nline = 3;
			while (nline--) {
				if (ansi_fgets(line, sizeof(line), fp) == NULL)
					break;
				/* skip empty line or line with no query term */
				if (strlen(line) == 0 || garbage_line(line)) {
					nline++;
				} else {
					/* strip leading white space */
					emphasize(line, output);
					fputs(lstrip(output), stdout);
					if (nline == 0)
						fputs(" ...", stdout);
					puts("<br>");
				}
			}
		}
		fclose(fp);
		
		get_telnet_path(g_filename[i], telnetpath, sizeof(telnetpath));
		printf("<font color=#008000>%s</font><br>", link);
		printf("<font color=#696969>%s</font>", telnetpath);
		puts("<br></font></td></tr></table><br>");
	}
}


void
show_right_column()
{
	puts("<table width='30%' border='0' cellpadding='0' cellspacing='0' align='right'><tr>");
	puts("<td align='left' style='padding-right:10px'>");
	puts("<div style='border-left:1px solid #e1e1e1;padding-left:10px;word-break:break-all;word-wrap:break-word;'>");
	puts("<script type='text/javascript'><!--");
	puts("google_ad_client = 'pub-9913823682183929';");
	puts("/* 160x600, 创建于 09-6-12 */");
	puts("google_ad_slot = '2304719016';");
	puts("google_ad_width = 160;");
	puts("google_ad_height = 600;");
	puts("//-->");
	puts("</script>");
	puts("<script type='text/javascript'");
	puts("src='http://pagead2.googlesyndication.com/pagead/show_ads.js'>");
	puts("</script>");
	puts("</div> </td></tr></table>");
}


void
show_result(char *bname, char *query_str, int type, int start, struct timeb *before)
{
	int i;
	struct timeb now;
	
	ftime(&now);
	int sec = now.time - before->time;
	int milli = now.millitm - before->millitm;
	if (milli < 0) {
		sec--;
		milli += 1000;
	}
	
	
	html_header(query_str);
	puts("<body>");
	puts("<table width='100%' height='54' align='center' cellpadding='0' cellspacing='0'>");
	puts("<tr valign=middle>");
	puts("<td width='100%' valign='top' style='padding-left:8px;width:137px;' nowrap>");
	puts("<a href='/'><img src='/frgg2.jpg' border='0' width='137' height='46' alt='到frgg首页'></a>");
	puts("</td>");
	puts("<td>&nbsp;&nbsp;&nbsp;</td>");
	puts("<td width='100%' valign='top'>");
		  
	puts("<table cellspacing='0' cellpadding='0'>");
	puts("<tr><td valign='top' nowrap>");
	puts("<form name=sb action='/cgi-bin/search' method='get'>");
	printf("<input type='text' name=q size='44' class='i' value='%s' maxlength='100'>", query_str);
	puts("&nbsp;&nbsp;&nbsp;&nbsp;");
	puts("<select name='b'>");

	char board[BFNAMELEN + 1];
	FILE *fp = fopen(BOARDS_LIST, "r");
	if (!fp) {
		puts("<option>Joke</option>");
		ERROR1("fopen failed %s", BOARDS_LIST);
	} else {
		
		while (fgets(board, sizeof(board), fp)) {
			if (board[0] == '\n')
				break;
			if (board[strlen(board) - 1] == '\n')
				board[strlen(board) - 1] = '\0';
			if (strcmp(bname, board) == 0)
				printf("<option selected = 'selected' >%s</option>\n", board);
			else
				printf("<option>%s</option>\n", board);
		}
		fclose(fp);
	}
	puts("</select>");
	puts("&nbsp;&nbsp;&nbsp;");
	puts("<input type=submit value='frgg一下'>");
	puts("</td></tr>");
	puts("<tr><td height='40'><font size=-1>");
        printf("<input type=radio name=t value='b'%s><label>版面文章 &nbsp;</label>", (type == BOARD) ? "checked" : " " );
	printf("<input type=radio name=t value='a'%s><label>精华区文章 &nbsp;</label>", (type == ANNOUNCE) ? "checked" : " " );
	puts("</font></td></tr>");
	puts("</form></table>");
	puts("</td></tr></table>");

	puts("<table width='100%' border='0' align='center' cellpadding='0' cellspacing='0' class='bi'>");
	puts("<tr><td nowrap>&nbsp;&nbsp;&nbsp;");
	printf("<td align='right' nowrap>frgg一下，找到相关文章约%d篇，用时%d.%03d秒&nbsp;", g_cnt, sec, milli);
	puts("&nbsp;&nbsp;&nbsp;</td></tr></table>");

	show_right_column();
	
	if (type == BOARD)
		show_board_result(bname);
	else
		show_ann_result();
	
	if (g_fname_cnt == 0) {
		printf("<p style=padding-left:15px;>在<b> %s </b>版面找不到和您的查询 <b> \"%s\"</b> 相符的内容或信息。", bname, query_str);
		puts("<p style=margin-top:1em;padding-left:15px;>建议:");
		printf("<ul><li>选择正确的版面, 你当前选择的版面是<b>%s</b>", bname);
		if (type == BOARD)
			puts("<li>尝试搜索<b>精华区文章</b>");
		puts("<li>使用不同的查询字词");
		puts("<li>改用常见的查询字词");
		puts("<li>减少关键词数量</ul></p></p>");
		puts("<p style=padding-left:15px;>阅读<a href=\"/help.html\">使用帮助</a></p>");
	}
	html_footer(query_str, bname, type, start);
	for (i = 0; i < MAX_PAGE_SIZE; i++)
		free(g_filename[i]);
}


void
search(char *bname, char *query_str, int type, int start)
{
	char *term;
	int i;

	struct timeb before;
	ftime(&before);

	g_text = query_str;
	g_len = strlen(query_str);
	init_segment();

	/* segmentation */
	while ((term = next_token())) {
		i = 0;
		if (isalpha(term[0])) {
			while (term[i]) {
				term[i] = tolower(term[i]);
				i++;
			}
		}
		g_termlist[g_nterm++] = term;
	}
	ranking(bname, type);
	gen_result(bname, type, start);
	show_result(bname, query_str, type, start, &before);
}
