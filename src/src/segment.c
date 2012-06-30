#include "frgg.h"

extern char *g_text;	/* filterd text */
size_t g_pos;		/* current position */
size_t g_len;		/* text length */

#define MAX_CHUNKS 128	/* 一次可组成最大数目的块数 */

struct chunk chunks[MAX_CHUNKS];
int g_nchunks;		/* number of chunks */


int
chunk_len(struct chunk *chk)
{
	int len = 0;
	int i;
	for (i = 0; i < chk->n; ++i)
		len += (strlen(chk->words[i]) / 2);
	return len;
}


double
chunk_avglen(struct chunk *chk)
{
	return ((double) chunk_len(chk)) / chk->n;
}


double
chunk_variance(struct chunk *chk)
{
	double avg = chunk_avglen(chk);
	double sqr_sum = 0;
	double tmp;
	int i;
	for (i = 0; i < chk->n; ++i) {
		tmp = strlen(chk->words[i]) - avg;
		sqr_sum += tmp * tmp;
	}
	return sqrt(sqr_sum);
}


/**
 *  sum of degree of morphemic freedom of one-character words
 */
double
chunk_mor_free_degree(struct chunk *chk)
{
	double sum = 0;
	unsigned int freq;
	int i;
	for (i = 0; i < chk->n; ++i) {
		if (strlen(chk->words[i]) == 2) { /* single character */
			freq = dict_get_char_freq(chk->words[i]);
			if (freq != 0) 
				sum += log(freq);
		}
	}
	return sum;
}


/**
 * Maximum matching
 */
double
mm_cmp(struct chunk *c1, struct chunk *c2)
{
	return chunk_len(c1) - chunk_len(c2);
}


/**
 * Largest average word length
 */
double
lawl_cmp(struct chunk *c1, struct chunk *c2)
{
	#if 0
	double a = chunk_avglen(c1);
	double b = chunk_avglen(c2);
	fprintf(stderr, "%d %d\n", c1->n, c2->n);
	int i;
	for (i = 0; i < c1->n; ++i) {
		fprintf(stderr, "%s  ", c1->words[i]);
	}

	fprintf(stderr, "\n");
	for (i = 0; i < c2->n; ++i) {
		fprintf(stderr, "%s  ", c2->words[i]);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "%lf %lf %lf\n", a, b, a-b);
	#endif
		
	return chunk_avglen(c1) - chunk_avglen(c2);
}


/**
 * Smallest variance of word lengths
 */
double
svwl_cmp(struct chunk *c1, struct chunk *c2)
{
	#if 0
	double a = chunk_variance(c1);
	double b = chunk_variance(c2);
	fprintf(stderr, "%d %d", c1->n, c2->n);
	int i;
	for (i = 0; i < c1->n; ++i) {
		fprintf(stderr, "%s  ", c1->words[i]);
	}

	fprintf(stderr, "\n");
	for (i = 0; i < c2->n; ++i) {
		fprintf(stderr, "%s  ", c2->words[i]);
	}
	
	fprintf(stderr, "\n");
	fprintf(stderr, "%lf %lf\n", a, b);
	#endif
	return chunk_variance(c2) - chunk_variance(c1);
}


/**
 * Largest sum of degree of morphemic freedom of one-character words
 */
double
lsdmfocw_cmp(struct chunk *c1, struct chunk *c2)
{
	return chunk_mor_free_degree(c1) - chunk_mor_free_degree(c2);
}


void
choose_best_chunk(double (*cmp)(struct chunk *c1, struct chunk *c2))
{
	struct chunk *best = &chunks[0];
	struct chunk tmp;
	int n = 1;
	
	int i;
	for (i = 1; i < g_nchunks; ++i) {
		double result = cmp(&chunks[i], best);
		
		if (result > 0) 	/* chunks[i] is the best */
			n = 0;		/* clear the previous `best chunks' */
		if (result >= 0) {
			tmp = chunks[n];
			chunks[n] = chunks[i];
			chunks[i] = tmp;
			n++;
		}
	}
	g_nchunks = n;
}


void
init_segment()
{
	g_pos = 0;
	load_dict(0);	/* load char dict */
	load_dict(1);	/* load word dict */
}


void
cleanup_segment()
{
	destroy_dict();
}


/**
 * 设置全局变量 g_nchunks, chunks
 */
void
create_chunks()
{
	char *words1[MAX_NEXT_WORDS];
	char *words2[MAX_NEXT_WORDS];
	char *words3[MAX_NEXT_WORDS];


	int i, j, k;
	off_t old_pos = g_pos;
	int nw1 = get_next_words(words1);
	g_nchunks = 0;
	for (i = 0; i < nw1; ++i) {
		g_pos += strlen(words1[i]);
		int nw2 = get_next_words(words2);
		if (nw2 == 0) {
			chunks[g_nchunks].n = 1;
			chunks[g_nchunks].words[0] = words1[i];
			g_nchunks++;
			if (g_nchunks == MAX_CHUNKS) {
				ERROR("MAX_CHUNKS reach i\n");
				ERROR1("%s\n", g_text + old_pos);
			}
			
		}
		
		for (j = 0; j < nw2; ++j) {
			g_pos += strlen(words2[j]);
			int nw3 = get_next_words(words3);
			if (nw3 == 0) {
				chunks[g_nchunks].n = 2;
				chunks[g_nchunks].words[0] = words1[i];
				chunks[g_nchunks].words[1] = words2[j];
				g_nchunks++;
			
				if (g_nchunks == MAX_CHUNKS) {
					ERROR("MAX_CHUNKS reach j\n");
					ERROR1("%s\n", g_text + old_pos);
				}
			}
			
			for (k = 0; k < nw3; ++k) {
				chunks[g_nchunks].n = 3;
				chunks[g_nchunks].words[0] = words1[i];
				chunks[g_nchunks].words[1] = words2[j];
				chunks[g_nchunks].words[2] = words3[k];
				g_nchunks++;
				if (g_nchunks == MAX_CHUNKS) {
					ERROR("MAX_CHUNKS reach k\n");
					ERROR1("%s\n", g_text + old_pos);
				}
				
			}
			g_pos -= strlen(words2[j]);
		}
		g_pos -= strlen(words1[i]);
	}
}


int
is_punctuation(char *str)
{
	static char *cp = "。，、；：？！…—·ˉ¨‘’“”～‖∶＂＇｀｜〃〔〕〈〉《》「」『』．〖〗【】（）［］｛｝";
	
	char *s = cp;
	while (*s) {
		if (strncmp(str, s, 2) == 0)
			return 1;
		s += 2;
	}
	return 0;
}


/**
 * store the results to words
 * return the number of words found in dict
 */
int
get_next_words(char *words[MAX_NEXT_WORDS])
{
	int n = 0;	/* number of words found */
	off_t pos = g_pos;

	off_t end = g_pos + MAX_WORD_LENGTH + 1;
	if (end > g_len)
		end = g_len;
	
	while (pos < end && (g_text[pos] & 0x80)) {
		if (is_punctuation(g_text + pos)) 
			break;
		
		pos += 2;
		if ((words[n] = dict_get_word(g_text + g_pos, pos - g_pos))) {
			n++;
			if (n == MAX_NEXT_WORDS) {
				DEBUG("MAX_NEXT_WORDS reach\n");
				DEBUG1("%s\n", g_text + g_pos);
				int i;
				for (i = 0; i<n; i++)
					DEBUG1("%s ", words[i]);
				break;
			}
		}
	}	
	return n;
}


/**
 * @return 返回NULL表示到达末尾.
 * @note 可能返回英文单词.
 */
char *
get_ch_word()
{
	create_chunks();

	if (g_nchunks > 1)
		choose_best_chunk(mm_cmp);
	
	if (g_nchunks > 1)
		choose_best_chunk(lawl_cmp);
	
	if (g_nchunks > 1)
		choose_best_chunk(svwl_cmp);
	
	if (g_nchunks > 1)
		choose_best_chunk(lsdmfocw_cmp);
	
	if (g_nchunks == 0) {
		g_pos += 2;
		/* no chinese words found, call next_token() recursively */
		return next_token();
	} else {
		g_pos += strlen(chunks[0].words[0]);
		return chunks[0].words[0];
	}
}


/**
 * @return 返回NULL表示到达末尾.
 */
char *
get_ascii_word()
{
	size_t start = g_pos;

	/* 数字开头, 只获取数字 */
	/* 字母开头, 获取字母或数字 */
	if (isdigit(g_text[start])) 
		while (g_pos < g_len && isdigit(g_text[g_pos]))
			g_pos++;
	else
		while (g_pos < g_len && isalnum(g_text[g_pos]))
			g_pos++;

	return strndup(g_text + start, g_pos - start);
}


char *
next_token()
{
	char *result = NULL;
	while (g_pos < g_len && result == NULL) {
		if (g_text[g_pos] & 0x80) { /*	ch character */
			result = get_ch_word();
		} else if (isalnum(g_text[g_pos])) {
			result = get_ascii_word();
		} else {
			g_pos++;
		}
	}
	return result;
}


/*
int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s filename [separator]\n", argv[0]);
		return 0;
	}

	chdir(FRGGHOME);
	
	char *word, *sep = " ";
	char *filename = argv[1];

	
	g_text = ansi_filter(filename);
	if (g_text != NULL) {
		init_segment();
		while (word = next_token()) 
			if (word != "")
				printf("%s%s", word, sep);

		printf("\n");
		free(g_text);
		dict_destroy();
	}
	
	return 0;
}
*/
