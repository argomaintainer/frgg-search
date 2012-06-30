#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(1);
	}
	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "fopen %s failed\n", argv[1]);
		exit(1);
	}
	char buf[2048];
	int in_article = 0, in_tag = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		if (strstr(buf, "</BLOCKQUOTE>")) {
			break;
		}
		if (strstr(buf, "<BLOCKQUOTE>")) {
			in_article = 1;
			continue;
		}
		if (in_article) {
			char *has_img;
			//lines like "<a target=_blank href='http://bbs.sysu.edu.cn/mlgb.gif'><img src='http://bbs.sysu.edu.cn/mlgb.gif'  ></a>"
			if ((has_img = strstr(buf, "'><img src='"))) {
				char *imglink = has_img + strlen("'><img src='");
				while (*imglink != '\'') {
					putc(*imglink, stdout);
					imglink++;
				}
				continue;
			}
			char *s = buf;
			while (*s) {
				if (in_tag == 1) {
					if (*s == '>')
						in_tag = 0;
					s++;
				} else {
					if (*s == '<') {
						in_tag = 1;
						s++;
					} else {
						if (strncmp(s, "&lt;", 4) == 0) {
							putc('<', stdout);
							s += 4;
						} else if (strncmp(s, "&gt;", 4) == 0) {
							putc('>', stdout);
							s += 4;
						} else if (strncmp(s, "&quot;", 6) == 0) {
							putc('"', stdout);
							s += 6;
						} else if (strncmp(s, "&amp;", 5) == 0) {
							putc('&', stdout);
							s += 5;
						} else {
							putc(*s, stdout);
							s++;
						}
					}
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

