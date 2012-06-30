#include "frgg.h"


char query_str[QUERY_STR_MAX];

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
	exit(1);
}


void
redirect(char *document)
{
	printf("Location: %s\n\n", document);
	exit(0);
}


int main(int argc, char *argv[])
{
	time_t now = time(NULL);
	char *raw_query_str = getenv("QUERY_STRING");
	char *remote_address = getenv("REMOTE_ADDR");

	if (raw_query_str == NULL)
		bad_request();
	chdir(FRGGHOME);
	decode(raw_query_str);
	if (query_str[0] == '\0')
		bad_request();
	/* q=xxxx&board=xxx&t=xxx&start=xxx */
	char *query_terms = strtok(query_str, "&");
	if (strncmp(query_terms, "q=", 2) == 0) {
		query_terms = query_terms + 2;	 /* skip "q=" */
	} else {
		bad_request();
	}

	if (query_terms[0] == '\0') {
		redirect("/");
	}
	
	char *board = strtok(NULL, "&");
	if (board != NULL && strncmp(board, "b=", 2) == 0) {
		board = board + 2;	 /* skip "b=" */
	} else {
		bad_request();
	}

	char *t = strtok(NULL, "&");
	int type = 0;
	if (t != NULL && strncmp(t, "t=", 2) == 0) {
		t = t + 2;
		if (strcmp(t, "a") && strcmp(t, "b")) {
			bad_request();
		} else if (strcmp(t, "a") == 0) {
			type = ANNOUNCE;
		} else {
			type = BOARD;
		}
	} else {
		bad_request();
	}

	char *start = strtok(NULL, "&");
	if (start != NULL  && strncmp(start, "start=", 6) == 0) {
		start = start + 6;
	}
	search(board, query_terms, type, (start) ? atoi(start) : 0);
	do_log(LOG_QUERY, "%s %24.24s b=%s t=%s p=%d %s", remote_address, ctime(&now), board, t, start ? atoi(start) : 0, query_terms);
	return 0;
}
