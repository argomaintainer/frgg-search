#include "frgg.h"


/* dict entry */
struct dict_ent
{
        char  *word;		/* '\0' 结尾 */
	unsigned int freq;
	struct dict_ent *next;
};


// int max_density = 5;

/*
  Table of prime numbers 2^n+a, 2<=n<=30.

static int primes[] = {
        524288 + 21,
        1048576 + 7,
        2097152 + 17,
        4194304 + 15,
        8388608 + 9,
        16777216 + 43,
        33554432 + 35,
        67108864 + 15,
        134217728 + 29,
        268435456 + 3,
        536870912 + 11,
        1073741824 + 85,
};

*/


//size_t n_chars = 0;	/* current number of chars in dict */
//size_t n_words = 0;


struct dict_ent *char_dict[char_dict_size];
struct dict_ent *word_dict[word_dict_size];


/*
static int
get_new_size(int size)
{
	int i;
	for (i = 0; i < sizeof(primes)/sizeof(primes[0]); ++i) 
		if (primes[i] > size) 
			return primes[i];
	
        // the code should never reach here 
        return size;
}
*/

void
load_dict(int type)
{
	FILE *fp;
	char buf[64];
	if (type == 0) {	 /* single char */
		fp = fopen(CHAR_DICT_FILE, "r");
		/* $ head -n2 dict_file
		   的 7922684
		   一 3050722
		*/
		if (fp != NULL) {
			while (fgets(buf, sizeof(buf), fp)) {
				if (buf[0] == '\n')
					break;
				buf[2] = '\0';		/* buf[0,1] = '的' */
				dict_add_char(strdup(buf), (unsigned short)atoi(buf + 3));
				dict_add_word(strdup(buf));
			}
			fclose(fp);
		}
	} else {
		fp = fopen(WORD_DICT_FILE, "r");
		if (fp != NULL) {
			while (fgets(buf, sizeof(buf), fp)) {
				if (buf[0] == '\n')
					break;
				buf[strlen(buf) - 1] = '\0';
				dict_add_word(strdup(buf));
			}
			fclose(fp);
		}
	}
}


void
dict_add_word(char *word)
{
	unsigned int hash_val = hash(word);
	unsigned int h = hash_val % word_dict_size;
	struct dict_ent *entry = word_dict[h];
	while (entry != NULL) {
		entry = entry->next;
	}

	/* #if 0
	if (n_words / word_dict_size > max_density) {
		rehash();
		h = hash_val % word_dict_size;
	}
	#endif */
	 
        entry = (struct dict_ent *)malloc(sizeof(struct dict_ent));
	entry->word = word;
	entry->freq = 0;
	entry->next = word_dict[h];
	word_dict[h] = entry;
//	n_words++;
}


char *
dict_get_word(char *str, size_t len)
{
	unsigned int h = nhash(str, len) % word_dict_size;
	struct dict_ent *entry = word_dict[h];
	while (entry != NULL) {
		if (strlen(entry->word) == len &&
		    strncmp(str, entry->word, len) == 0)
			return entry->word;
		
		entry = entry->next;
	}
	return NULL;
}


/**
 * @param ch - 中文单字, '\0' 结尾
 * @param freq - 单字频率
 */
void
dict_add_char(char *ch, unsigned short freq)
{
	unsigned int hash_val = nhash(ch, 2);
	unsigned int h = hash_val % char_dict_size;
	struct dict_ent *entry = char_dict[h];
	while (entry != NULL) {
		entry = entry->next;
	}
	
	/*
	if (n_chars / char_dict_size > max_density) {
		rehash();
		h = hash_val % char_dict_size;
	}
	*/
            
	entry = (struct dict_ent *)malloc(sizeof(struct dict_ent));
	entry->word = ch;
	entry->freq = freq;
	entry->next = char_dict[h];
	char_dict[h] = entry;
//	n_chars++;
}


/**
 * @param ch - 单字, 2bytes
 */
unsigned int
dict_get_char_freq(char *ch)
{
	unsigned int h = nhash(ch, 2) % char_dict_size;
	struct dict_ent *entry = char_dict[h];

	while (entry != NULL) {
		if (strncmp(ch, entry->word, 2) == 0)
			return entry->freq;
		entry = entry->next;
	}
	return 0;
}


/**
 * free the memroy used by dict
 */
void
destroy_dict()
{
	struct dict_ent *entry, *next;
	int i;
	for (i = 0; i < char_dict_size; ++i) {
		entry = char_dict[i];
		while (entry != NULL) {
			next = entry->next;
			free(entry->word);
			free(entry);
			entry = next;
		}
	}
	
	for (i = 0; i < word_dict_size; ++i) {
		entry = word_dict[i];
		while (entry != NULL) {
			next = entry->next;
			free(entry->word);
			free(entry);
			entry = next;
		}
	}
}


/*
void
rehash()
{
	
}
*/
