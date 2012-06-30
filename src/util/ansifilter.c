#include "frgg.h"
 
extern off_t g_len;		/* global content length */

/*
 * remove the ansi control char
 * @filename - file to be processed
 * @return - filtered content
 * @sideffect - set g_len to length of the filename
 * @warning - don't forget to free the return buf
 **/
char*
ansi_filter(char *filename)
{
	int fd, inansi = NA;
	char *ptr, *ret;
	struct stat st;
	off_t i;
	
	if ((fd = open(filename, O_RDONLY)) == -1)
		return NULL;
	
	if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode) || st.st_size <= 0) {
		close(fd);
		return NULL;
	}

	ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	if (ptr == MAP_FAILED) {
		return NULL;
	}
	
		
	ret = malloc(st.st_size);
	if (ret == NULL)
		goto done;
	
	g_len = 0;
	for (i = 0; i < st.st_size; ++i) {
		if (inansi) {
			if (ptr[i] == 'm') {
				inansi = NA;
			}
		} else if (ptr[i] == KEY_ESC) {
			inansi = YEA;
		} else {
			ret[g_len++] = ptr[i];
		}
	}
	
done:
	munmap(ptr, st.st_size);
	return ret;
}


#ifdef DEBUG
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
