#include "frgg.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}


void
parse_file(char *filename, char *result_file)
{
	FILE *fp = fopen(filename, "r");
	FILE *fout = fopen(result_file, "w");
	if (!fp || !fout) {
		ERROR("fopen failed\n");
		return ;
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
					fputc(*imglink, fout);
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
							putc('<', fout);
							s += 4;
						} else if (strncmp(s, "&gt;", 4) == 0) {
							putc('>', fout);
							s += 4;
						} else if (strncmp(s, "&quot;", 6) == 0) {
							putc('"', fout);
							s += 6;
						} else if (strncmp(s, "&amp;", 5) == 0) {
							putc('&', fout);
							s += 5;
						} else {
							putc(*s, fout);
							s++;
						}
					}
				}
			}
		}
	}
	fclose(fp);
	fclose(fout);
}


int
fetch_file(CURL *curl_handle, char *board, char *fname, char *tmpfile)
{

	FILE *headerfile;
	FILE *bodyfile;
	char url[512];
	char *pattern = "http://bbs.sysu.edu.cn/bbscon?board=%s&file=%s";
	

	snprintf(url, sizeof(url), pattern, board, fname);
	
	/* set URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* open the files */
	headerfile = fopen("/dev/null", "w");
	if (headerfile == NULL) {
		ERROR("fopen /dev/null failed");
		return -1;
	}
	
	bodyfile = fopen(tmpfile, "w");
	if (bodyfile == NULL) {
		ERROR1("fopen %s failed", tmpfile);
		return -1;
	}
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headerfile);

	/* we want the bodys to this file handle */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, bodyfile);

	/* get it! */
	curl_easy_perform(curl_handle);
	
	/* close the header file */
	fclose(headerfile);
	fclose(bodyfile);
	return 0;
}

	

int main(int argc, char *argv[])
{
	time_t now;
	int use_diff = 1;
	char board[BFNAMELEN + 1];
	char filelist[PATH_MAX], files_diff[PATH_MAX];
	FILE *filep;
	char file[PATH_MAX], filepath[PATH_MAX], tmpfile[PATH_MAX];
	CURL *curl_handle;

	if (argc > 1 && !strcmp(argv[1], "nodiff"))
		use_diff = 0;

	chdir(FRGGHOME);
	FILE *fp = fopen(BOARDS_LIST, "r");
	if (!fp) {
		ERROR1("fopen failed %s", BOARDS_LIST);
		return 1;
	}


	/* init CURL */
	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* no progress meter please */
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	
	while (fgets(board, sizeof(board), fp)) {
		if (board[0] == '\n')
			continue;
		if (board[strlen(board) - 1] == '\n')
			board[strlen(board) - 1] = '\0';
		
		setboardfile(filelist, board, board);
		if (use_diff) {
			snprintf(files_diff, sizeof(files_diff), "%s.diff", filelist);
			filep = fopen(files_diff, "r");
		} else {
			filep = fopen(filelist, "r");
		}
		
		if (filep ==  NULL) {
			ERROR1("fucking error %s", filelist);
				continue;
		}
		setboardfile(tmpfile, board, "frgg.tmp");
		size_t cnt_new = 0;
		size_t cnt_rm = 0;
		while (fgets(file, sizeof(file), filep)) {
			if (file[0] == '\n')
				continue;
			
			if (use_diff) {
				if (!strncmp(file, "---", 3))
					continue;
				if (!strncmp(file, "+++", 3))
					continue;
				if (!strncmp(file, "@@", 2))
					continue;
			}
			
			if (file[strlen(file) - 1] == '\n')
				file[strlen(file) - 1] = '\0';

			char *fname = file;
			if (use_diff)
				fname++;
			
			if (file[0] == '-') {	/* delete file */
				setboardfile(filepath, board, fname);
				unlink(filepath);
				cnt_rm++;
			} else {
				setboardfile(filepath, board, fname);
				if (access(filepath, R_OK) != 0) {
					fetch_file(curl_handle, board, fname, tmpfile);
					/* parse html */
					parse_file(tmpfile, filepath);
					cnt_new++;
				}
			}
		}
		unlink(tmpfile);
		time(&now);
		if (cnt_new != 0 || cnt_rm != 0)
			do_log(LOG_TRACE, "%24.24s %u new files, %u files deleted, from %s ", ctime(&now), cnt_new, cnt_rm, board);
		fclose(filep);
	}
	fclose(fp);
	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);	
        return 0;
}
    
