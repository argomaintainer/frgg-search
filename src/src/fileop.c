#include "frgg.h"


/* following code is extracted from source code of freebsd's mkdir(1) */
int
f_mkdir(char *path, int omode)
{
	struct stat sb;
	mode_t numask, oumask;
	int first, last, retval;
	char *p;

	p = path;
	oumask = 0;
	retval = 0;
	if (p[0] == '/')		/* Skip leading '/'. */
		++p;
	for (first = 1, last = 0; !last ; ++p) {
		if (p[0] == '\0')
			last = 1;
		else if (p[0] != '/')
			continue;
		*p = '\0';
		if (p[1] == '\0')
			last = 1;
		if (first) {
			/*
			 * POSIX 1003.2:
			 * For each dir operand that does not name an existing
			 * directory, effects equivalent to those cased by the
			 * following command shall occcur:
			 *
			 * mkdir -p -m $(umask -S),u+wx $(dirname dir) &&
			 *    mkdir [-m mode] dir
			 *
			 * We change the user's umask and then restore it,
			 * instead of doing chmod's.
			 */
			oumask = umask(0);
			numask = oumask & ~(S_IWUSR | S_IXUSR);
			umask(numask);
			first = 0;
		}
		if (last) umask(oumask);
		if (mkdir(path, last ? omode : S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
			if (errno == EEXIST || errno == EISDIR) {
				if (stat(path, &sb) < 0) {
					retval = -1;
					break;
				} else if (!S_ISDIR(sb.st_mode)) {
					if (last) {
						errno = EEXIST;
						retval = 0; /* ignore if target directory exists */
					} else {
						errno = ENOTDIR;
						retval = -1;
					}
					break;
				}
			} else {
				retval = -1;
				break;
			}
		}
		if (!last)
		    *p = '/';
	}
	if (!first && !last)
		umask(oumask);
	return retval;
}
