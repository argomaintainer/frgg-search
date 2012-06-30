#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


#define BBSHOME "/home/bbs"

#define STRLEN                  80    	/* length of string buffer */
#define IDLEN                   12	/* length of user id. */
#define BUFLEN                  1024	/* length of general buffer */
#define TITLELEN                56	/* length of article title */
#define FNAMELEN                16	/* length of filename */
#define NAMELEN                 20	/* length of realname */


struct fileheader {
	char filename[FNAMELEN];		// filename format: {M|G}.time.{Alphabet}
	char owner[IDLEN + 2];			
	char realowner[IDLEN + 2];		// to perserve real owner id even in anonymous board
	char title[TITLELEN];
	unsigned int flag;
	unsigned int size;
	unsigned int id;  			// identity of article (per thread)
	time_t filetime;		
	char reserved[12];
};


int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s boandname", argv[0]);
		exit(1);
	}
	
	char *board = argv[1];
	char dir[PATH_MAX], ofile[PATH_MAX];
	struct fileheader fh;
	chdir(BBSHOME);
	snprintf(dir, sizeof(dir), "boards/%s/.DIR", board);
	snprintf(ofile, sizeof(ofile), "htdocs/frgg/%s", board);
	
	FILE *fp = fopen(dir, "r");
	FILE *fout = fopen(ofile, "w");
	
	while (fread(&fh, sizeof(fh), 1, fp)) {
		fprintf(fout, "%s\n", fh.filename);
	}
	fclose(fp);
	fclose(fout);
	return 0;
}

		
	       
	
	     
