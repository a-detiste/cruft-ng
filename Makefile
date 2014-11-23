CPP:=clang++

all: cruft

tests: test_mlocate test_explain test_filters cruftlib

cruft.o: cruft.cc mlocate.h dpkg.h
	$(CPP) cruft.cc -O2 -Wall -c -o cruft.o

explain.o: explain.cc explain.h
	$(CPP) explain.cc -O2 -Wall -c -o explain.o

filters.o: filters.cc filters.h
	$(CPP) filters.cc -O2 -Wall -c -o filters.o

mlocate.o: mlocate.cc mlocate.h
	$(CPP) mlocate.cc -O2 -Wall -c -o mlocate.o

dpkg_lib.o: dpkg_lib.cc dpkg.h
	$(CPP) dpkg_lib.cc -O2 -Wall -c -o dpkg_lib.o

dpkg_popen.o: dpkg_popen.cc dpkg.h
	$(CPP) dpkg_popen.cc -O2 -Wall -c -o dpkg_popen.o

shellexp.o: shellexp.c
	$(CPP) shellexp.c -O2 -Wall -c -o shellexp.o

cruft: cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o
	$(CPP) cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o -Wall -o cruft-ng

cruftlib: cruft.o explain.o filters.o mlocate.o dpkg_lib.o shellexp.o
	$(CPP) cruft.o explain.o filters.o mlocate.o dpkg_lib.o   shellexp.o -Wall -o cruftlib

test_mlocate: mlocate.o test_mlocate.cc
	$(CPP) mlocate.o test_mlocate.cc -Wall -o test_mlocate

test_explain: explain.o test_explain.cc dpkg_popen.o
	$(CPP) explain.o test_explain.cc dpkg_popen.o -Wall -o test_explain

test_filters: filters.o test_filters.cc dpkg_popen.o
	$(CPP) filters.o test_filters.cc dpkg_popen.o -Wall -o test_filters

install: all
	install -D -m 2755 -g mlocate cruft-ng   $(DESTDIR)/usr/bin/cruft-ng
	install -D -m 0644            cruft-ng.8 $(DESTDIR)/usr/share/man/man8/cruft-ng.8
	
clean:
	rm -f cruft-ng cruftlib test_mlocate test_filters test_explain
	rm -f *.o
