#include <iostream>
#include <algorithm>
#include <string>


using namespace std;

#define MAX_LINE 1000000

string v[MAX_LINE];


int main(int argc, char *argv[])
{

	cerr << "warning: max number of lines = 1000000" << endl;
	cerr << "max length of line = 128" << endl;
	char buf[128];
	int n = 0;
	
	cerr << "start reading" << endl;
	
	while (gets(buf)) {
		v[n++] = buf;
	}
	
	cerr << "start sorting" << endl;
	sort(v, v+n);

	int end = unique(v, v+n) - v;

	for (int i = 0; i < end; ++i) {
		cout << v[i] << endl;
	}
	return 0;
}


