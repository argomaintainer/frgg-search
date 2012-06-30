#include <stdio.h>
#include <string.h>


#define MAX_FREQ 7922684

int main(int argc, char *argv[])
{
	char buf[128];
	
	/* 65535  ~ 7922684 */
	while (gets(buf)) {
		unsigned short freq = 1 + atoi(buf + 3) / (double)MAX_FREQ * 65534;
		printf("%.2s %d\n", buf, freq);
	}
	
	return 0;
}

