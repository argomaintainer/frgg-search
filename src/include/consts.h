/*******************************************
 *              BBS PARAMETERS             *
 *******************************************/

#define STRLEN                  80    	/* length of string buffer */
#define IDLEN                   12	/* length of user id. */
#define BUFLEN                  1024	/* length of general buffer */
#define TITLELEN                56	/* length of article title */
#define FNAMELEN                16	/* length of filename */
#define NAMELEN                 20	/* length of realname */

#define BTITLELEN               40	/* length of board title */
#define BFNAMELEN               20	/* length of board name */


/*******************************************
 *          BBS RELATED CONSTANTS          *
 *******************************************/

/* filenames */
#define PASSFILE        BBSHOME"/.PASSWDS"
#define BOARDS          BBSHOME"/.BOARDS"

#define DOT_DIR     	".DIR"
#define THREAD_DIR  	".THREAD"
#define DIGEST_DIR  	".DIGEST"
#define MARKED_DIR  	".MARKEDDIR"
#define AUTHOR_DIR  	".AUTHORDIR"
#define KEY_DIR     	".KEYDIR"
#define DELETED_DIR	".DELETED"
#define JUNK_DIR	".JUNK"
#define DENY_DIR	".DENYLIST"

/* fileheader->flag */
#define FILE_READ       0x000001
#define FILE_OWND       0x000002
#define FILE_VISIT      0x000004
#define FILE_MARKED     0x000008	/* article is marked */
#define FILE_DIGEST     0x000010        /* article is added to digest */
#define FILE_FORWARDED  0x000020        /* article restored from recycle bin */
#define MAIL_REPLY      0x000020        /* mail replyed */
#define FILE_NOREPLY    0x000040        /* reply to the article is not allowed */
#define FILE_DELETED    0X000080
#define FILE_SELECTED   0x000100	/* article selected */
#define FILE_ATTACHED   0x000200        /* article comes with attachments */
#define FILE_OUTPOST    0x010000
#define FILE_RECOMMENDED 0x000400       /* article has been recommended */


/* boardheader->flag */
#define VOTE_FLAG       0x000001
#define NOZAP_FLAG      0x000002
#define OUT_FLAG        0x000004
#define ANONY_FLAG      0x000008
#define NOREPLY_FLAG    0x000010
#define READONLY_FLAG   0x000020
#define JUNK_FLAG       0x000040
#define NOPLIMIT_FLAG   0x000080

#define BRD_READONLY    0x000100		  /* 只读 (不能发文, 只能删文和做标记 */
#define BRD_RESTRICT    0x000200		  /* 限制版 */
#define BRD_NOPOSTVOTE  0x000400		  /* 投票结果不公开 */
#define BRD_ATTACH	0x000800		  /* 允许上传附件 */
#define BRD_GROUP	0x001000		  /* 版面列表 */
#define BRD_HALFOPEN	0x002000		  /* changed by freestyler: 激活用户可访问 */
#define BRD_INTERN  	0x004000		  /* Added by betterman :仅限校内访问 */

/* announce.c */
#define ANN_FILE                0x01              /* 普通文件 */
#define ANN_DIR                 0x02              /* 普通目录 */
#define ANN_PERSONAL            0x04              /* 个人文集目录 */
#define ANN_GUESTBOOK           0x08              /* 留言本 */
#define ANN_LINK                0x10              /* Local Link */
#define ANN_RLINK               0x20              /* Remote Link (unused) */
#define ANN_SELECTED            0x100             /* 被选择 */
#define ANN_ATTACHED		0x200		  /* 带有附件 */
#define ANN_RESTRICT            0x010000          /* 限制性文件/目录 */
#define ANN_READONLY            0x020000          /* 只读 (不能修改属性/内容) */


/*******************************************
 *	  KEYBOARD RELATED CONSTANTS       *
 *******************************************/

#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206


/*******************************************
 *        FUNCTION RELATED CONSTANTS       *
 *******************************************/

/* general constants */
#define YEA		1
#define NA		0

#define TRUE		1
#define FALSE		0

#define CRLF		"\r\n"



/*******************************************
 *        FRGG RELATED CONSTANTS  	   *
 *******************************************/

/* segment, index */
#define char_dict_size 10000
#define word_dict_size 2000000

#define MAX_NEXT_WORDS 7	/* 从某处开始可以组成的最大词数 */

#define BOARD	 0
#define ANNOUNCE 1


#define INIT_POSTINGLIST_SIZE	1

#define MAX_FILE_SIZE 2097152   /* 2 * 1024 * 1024 */

#define MAX_WORD_LENGTH 30	/* 最长词的长度 中文15个字 */

#define MAX_POSTLING_MEMORY_SIZE  1048576  /* 1M,  1024 * 1024 */

/* search.c */
#define MAX_RETURN_DOCS 1000

#define MAX_PAGE_SIZE 10	/* number of results in one page */


/* cgi.c */
#define QUERY_STR_MAX 512
