#include "frgg.h"


static char query_str[QUERY_STR_MAX];

int
hex2int(char ch)
{
	if (ch >= 'A' && ch <= 'F')
		return 10 + ch - 'A';
	if (ch >= 'a' && ch <= 'f')
		return 10 + ch - 'a';
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	/* error occur */
	return -1;
}


void
decode(char *raw)
{
	int i = 0, j = 0;
	while(raw[i]) {
		if(raw[i] == '%'){
			query_str[j++] = hex2int(raw[i + 1]) * 16 + hex2int(raw[i + 2]);
			i += 3;
		} else if(raw[i]=='+'){
			query_str[j++] = ' ';
			i++;
                } else	{
			query_str[j++] = raw[i++];
		}
		if (j == QUERY_STR_MAX - 1)
			break;
 	}
	query_str[j] = '\0';
}


int
bad_request()
{
	puts("Content-type: text/html; charset=gbk\n\n");
	puts("<html><head>");
	puts("<meta http-equiv='content-type' content='text/html;charset=gbk'>");
	puts("</head><body>");
	puts("<h3>bad request</h3>");
	puts("</body></html>");
	return 0;
}


void
header(char *link, char *date)
{
	printf("Content-type: text/html; charset=gbk\n\n");
	puts("<meta http-equiv='Content-Type' content='text/html; charset=gbk'>");
	puts("<div style='margin:-1px -1px 0;padding:0;border:1px solid #999;background:#fff'>");
	puts("<div style='margin:12px;padding:8px;border:1px solid #999;background:#ddd;font:13px arial,sans-serif;color:#000;font-weight:normal;text-align:left'>");
	printf("这是 frgg 对 <a href='%s' style='text-decoration:underline;color:#00c'>%s </a>的缓存。 这是该文章在 %s 的快照。", link, link, date);
	printf("<a href='%s' style='text-decoration:underline;color:#00c'>当前页</a>在此期间可能已经更改。<br><br> ", link);
	puts("&nbsp;&nbsp;(frgg和该文章的作者无关，不对其内容负责，谢绝跨省追捕。)");
	puts("<br></div></div>");
		
	puts("<html><head>");
	puts("<meta content='text/html; charset=gbk' http-equiv='content-type'>");
        puts("<title>frgg快照</title>");
	puts("</head><body><pre><BLOCKQUOTE>");
}


int
show_cache(char *board, char *file)
{
	char filepath[PATH_MAX];
	char line[1024];
	char link[1024];
	char date[256];
	struct stat st;
	int i, len;
	gzFile fp;

	
	snprintf(link, sizeof(link), "http://bbs.sysu.edu.cn/bbscon?board=%s&file=%s", board, file);
	setcachefile(filepath, board, file);

	if (stat(filepath, &st) == -1) {
		bad_request();
		return -1;
	}
	fp = gzopen(filepath, "rb");
	if (fp == NULL) {
		bad_request();
		return -1;
	}
	header(link, ctime(&st.st_mtime));
	while (gzgets(fp, line, sizeof(line))) {
		len = strlen(line);
		for (i = 0; i < len; i++) {
			if (line[i] == '<')
				fputs("&lt;", stdout);
			else if (line[i] == '>')
				fputs("&gt;", stdout);
			else if (line[i] == '"')
				fputs("&quot;", stdout);
			else if (line[i] == '&')
				fputs("&amp;", stdout);
			else
				fputc(line[i], stdout);
		}
	}
	gzclose(fp);
	puts("</BLOCKQUOTE></pre></body></html>");
	return 0;
}


int main(int argc, char *argv[])
{
	char *raw_query_str = getenv("QUERY_STRING");
	
	chdir(FRGGHOME);
	decode(raw_query_str);
	/* board=xxx&file=xxx */
	
	char *board = strtok(query_str, "&");
	if (board != NULL && strncmp(board, "board=", 6) == 0) {
		board = board + 6;	 /* skip "board=" */
	}

	char *file = strtok(NULL, "&");
	if (file != NULL  && strncmp(file, "file=", 5) == 0) {
		file += 5;
	}
	show_cache(board, file);
	return 0;
}
