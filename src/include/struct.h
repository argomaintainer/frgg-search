#ifndef _STRUCT_H_
#define _STRUCT_H_

/**
 * A chunk stores 3 (or less) successive words.
 */
struct chunk
{
	short n;	/* number of words */
        char *words[3];
};


struct postinglist
{
	unsigned int freq;	/* doc freq */
	unsigned int *docs;	/* DocIDs list */
	unsigned short *tf;	/* term freq list */
	size_t  size;		/* list capacity */
};


struct dict_t
{
	char *term;
	struct postinglist *p;
	struct dict_t *next;
};


/**
 *  BBS board file header definition
 */
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
	unsigned int flag;		/* ANN_*** 属性组合 */
	int mtime;            	 	/* modification time */
	char reserved[20];   		/* 保留字段 */
};


#endif
