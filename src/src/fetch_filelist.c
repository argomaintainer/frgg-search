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


int
fetch_file(CURL *curl_handle, char *board, char *ofile)
{

	FILE *headerfile;
	FILE *bodyfile;
	char url[512];
	
	char *pattern = "http://bbs.sysu.edu.cn/frgg/%s";
	
	snprintf(url, sizeof(url), pattern, board);
	
	/* set URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* open the files */
	headerfile = fopen("/dev/null", "w");
	if (headerfile == NULL) {
		ERROR("fopen /dev/null failed");
		return -1;
	}
	
	bodyfile = fopen(ofile, "w");
	if (bodyfile == NULL) {
		ERROR1("fopen %s failed", ofile);
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
	chdir(FRGGHOME);
	FILE *fp = fopen(BOARDS_LIST, "r");
	if (!fp) {
		ERROR1("fopen failed %s", BOARDS_LIST);
		return 1;
	}
	char board[BFNAMELEN + 1];
	char filelist_file[PATH_MAX];
	char old_file[PATH_MAX];
	char bdir[PATH_MAX];
	CURL *curl_handle;

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
		setboardfile(filelist_file, board, board);
		snprintf(old_file, sizeof(old_file), "%s.old", filelist_file);
		
		sprintf(bdir, "data/boards/%s/", board);
		/* mkdir first if dir not exists */
		f_mkdir(bdir, 0755);
		rename(filelist_file, old_file);
		fetch_file(curl_handle, board, filelist_file);
	}
	fclose(fp);
	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);		
        return 0;
}
    
