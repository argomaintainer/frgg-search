db_include_path = /usr/local/BerkeleyDB.4.7/include/
db_library_path = /usr/local/BerkeleyDB.4.7/lib/

CFLAGS = -Wall -g -D_GNU_SOURCE
include_opt = -I../include -I$(db_include_path)
library_opt = -L$(db_library_path) -ldb -lm -lz

all: buildbrdindex buildannindex search fetch_filelist get_files cache

update: buildbrdindex buildannindex search fetch_filelist get_files cache
	cp search ../../www/cgi-bin/;
	cp buildbrdindex ../../bin/;
	cp buildannindex ../../bin/;
	cp fetch_filelist ../../bin/;
	cp get_files ../../bin/;
	cp cache ../../www/cgi-bin/;

fetch_filelist: fetch_filelist.c log.o fileop.o
	gcc $(CFLAGS) -o fetch_filelist log.o fileop.o fetch_filelist.c $(include_opt) -lcurl

get_files: get_files.c log.o 
	gcc $(CFLAGS) -o get_files log.o get_files.c $(include_opt) -lcurl

buildbrdindex: brdindex.o index.o segment.o dict.o ansifilter.o log.o db.o formula.o fileop.o string.o
	gcc $(CFLAGS) -o buildbrdindex brdindex.o index.o segment.o dict.o ansifilter.o log.o string.o \
	db.o formula.o fileop.o $(library_opt)

buildannindex: annindex.o index.o segment.o dict.o ansifilter.o log.o db.o formula.o fileop.o string.o
	gcc $(CFLAGS) -o buildannindex annindex.o index.o segment.o dict.o ansifilter.o log.o string.o \
	db.o formula.o fileop.o $(library_opt)

search: cgi.o formula.o log.o db.o segment.o dict.o search.o ansifilter.o string.o
	gcc $(CFLAGS) -o search  ansifilter.o cgi.o search.o log.o dict.o db.o segment.o formula.o string.o \
	$(include_opt) $(library_opt)

cache: showcache.c
	gcc $(CACHE) -o cache showcache.c $(include_opt) $(library_opt)

brdindex.o: brdindex.c
	gcc $(CFLAGS) -c brdindex.c $(include_opt)
annindex.o: annindex.c
	gcc $(CFLAGS) -c annindex.c $(include_opt)
index.o: index.c
	gcc $(CFLAGS) -c index.c $(include_opt)
dict.o: dict.c
	gcc $(CFLAGS) -c dict.c $(include_opt)
ansifilter.o: ansifilter.c
	gcc $(CFLAGS) -c ansifilter.c $(include_opt)
segment.o: segment.c
	gcc $(CFLAGS) -c segment.c $(include_opt)
log.o: log.c
	gcc $(CFLAGS) -c log.c $(include_opt)
db.o: db.c
	gcc $(CFLAGS) -c db.c $(include_opt)
formula.o: formula.c
	gcc $(CFLAGS) -c formula.c $(include_opt)
fileop.o:
	gcc $(CFLAGS) -c fileop.c $(include_opt)
cgi.o: cgi.c
	gcc $(CFLAGS) -c cgi.c $(include_opt)
search.o: search.c 
	gcc $(CFLAGS) -c search.c $(include_opt)
string.o: string.c
	gcc $(CFLAGS) -c string.c $(include_opt)

.PHONY: clean
clean:
	rm -f *.o core buildbrdindex buildannindex search segment fetch_filelist get_files cache
