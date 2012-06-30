/**
 *  遍历精华区目录
 *  Print filenames and titles of files under the announce directory
 *  of the board `argv[1]' to stdout. 
 *  kofreestyler@gmail.com 2009.3.3
 **/

#include <stdio.h>	
#include <limits.h>	/* for PATH_MAX */
#include <unistd.h>     /* for chdir(2) */


/* some constants defined in bbs src */
#define BBSHOME			"/home/bbs"

#define ANN_DIR                 0x02              /* 普通目录 */
#define ANN_GUESTBOOK           0x08              /* 留言本 */

#define STRLEN                  80      /* length of string buffer */
#define IDLEN                   12      /* length of user id. */
#define BUFLEN                  1024    /* length of general buffer */
#define TITLELEN                56      /* length of article title */
#define FNAMELEN                16      /* length of filename */
#define NAMELEN                 20      /* length of realname */

#define YEA		1
#define NA		0


/**
 * 精华区每个目录下有个索引文件.DIR
 * .DIR         ::= annheaders ;
 * annheaders   ::= annheaders annheader |  ;
 * annheader    ::= struct annheader ;
 **/
struct annheader {			/* announce header */
	char filename[FNAMELEN];
	char owner[IDLEN + 2];		/* 作者 */
	char editor[IDLEN + 2];		/* 整理 */
	char title[TITLELEN];       	/* 标题 */
	unsigned int flag;		/* 上面定义的 ANN_*** 属性组合 */
	int mtime;            	 	/* modification time */
	char reserved[20];   		/* 保留字段 */
};


void
ann_traverse(char *directory)
{
	char dotdir[PATH_MAX + 1], sdir[PATH_MAX + 1];
	struct annheader header;
	FILE *fpdir;

	snprintf(dotdir, sizeof(dotdir), "%s/.DIR", directory);
	if ((fpdir = fopen(dotdir, "r")) == NULL)      /* no .DIR found */
		return;

	while (fread(&header, 1, sizeof(header), fpdir) == sizeof(header)) {
		/*　目录或留言本, 递归 */
		if ((header.flag & ANN_DIR) || (header.flag & ANN_GUESTBOOK)) {
			snprintf(sdir, sizeof(sdir), "%s/%s", directory, header.filename);
			ann_traverse(sdir);
		} else {
			printf("%s/%s %s\n ", directory, header.filename, header.title);
		}
	}
	
	fclose(fpdir);
}


int
main(int argc, char *argv[])
{
	char board_path[PATH_MAX + 1];		/* 版面真实路径 */
	
	if (argc != 2) {
		printf("usage: %s board_name\n", argv[0]);
		return 0;
	}
	
	snprintf(board_path, sizeof(board_path), BBSHOME"/0Announce/boards/%s", argv[1]);
	ann_traverse(board_path);
	return 0;
}
