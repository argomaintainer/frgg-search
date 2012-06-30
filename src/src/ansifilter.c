#include "frgg.h"

extern char *g_text;
extern off_t g_len;		/* global content length */


char *
ansi_fgets(char *buf, size_t size, FILE *fp)
{
	if (fgets(buf, size, fp) == NULL)
		return NULL;
	
	size_t len = strlen(buf);
	int inansi = NA;
	int i = 0, j = 0;
	for (i = 0; i < len; i++) {
		if (inansi && buf[i] == 'm') {	/* only filter ESC[...m sequence */
			inansi = NA;
		} else if (inansi) {
			continue;
		} else if (buf[i] == KEY_ESC) {
			inansi = YEA;
		} else {
			buf[j++] = buf[i];
		}
	}
	buf[j] = '\0';
	return buf;
}


/**
 * read the file into g_text
 * set g_len to length of the file
 **/
void
ansi_filter(char *filename)
{
	int fd, inansi = NA;
	char *ptr;
	struct stat st;
	off_t i;

	g_len = 0;
	if ((fd = open(filename, O_RDONLY)) == -1) {
		ERROR1("open file %s failed", filename);
		return;
	}
	
	if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode) || st.st_size <= 0) {
		close(fd);
		return;
	}

	if (st.st_size > MAX_FILE_SIZE) {
		ERROR1("file %s size exceed MAX_FILE_SIZE constant", filename);
		close(fd);
		return;
	}

	if ((ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		close(fd);
		ERROR1("mmap %s failed", filename);
		return;
	}
	close(fd);
	
	for (i = 0; i < st.st_size; ++i) {
		if (inansi && ptr[i] == 'm') {	/* only filter ESC[...m sequence */
			inansi = NA;
		} else if (inansi) {
			continue;
		} else if (ptr[i] == KEY_ESC) {
			inansi = YEA;
		} else {
			g_text[g_len++] = ptr[i];
		}
	}
	munmap(ptr, st.st_size);
}


#if 0
int main(int argc, char *argv[])
{
	char * t = ansi_filter(argv[1]);
	FILE *fp = fopen("output.txt", "w");
	fwrite(t, g_len, 1, fp);
	free(t);
	fclose(fp);
	return 0;
}
#endif
