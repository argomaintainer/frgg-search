
all: sort_and_uniq parse_article dump_dir tranverse_announce
	cp parse_article ../../bin;
	cp dump_dir ../../bin;

ansi_filter: ansifilter.c
	gcc -I../include -DDEBUG -o ansi_filter ansifilter.c

sort_and_uniq: sort_and_uniq.cpp
	g++ -DDEBUG -o sort_and_uniq sort_and_uniq.cpp

parse_article: parse_article.c
	gcc -g -o parse_article parse_article.c

dump_dir: dump_dir.c
	gcc -g -o dump_dir dump_dir.c

tranverse_announce: tranverse.c
	gcc -g -o tranverse_announce tranverse.c

.PHONY: clean
clean:
	rm -f *.o ansi_filter sort_and_uniq parse_article dump_dir tranverse_announce
