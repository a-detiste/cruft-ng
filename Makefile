all: cruft-ng
tests: test_mlocate test_explain test_filters cruftlib

cruft.o: cruft.cc explain.h filters.h mlocate.h dpkg.h
	$(CXX) cruft.cc -O2 -Wall -c -o cruft.o

%.o: %.cc %.h
	$(CXX) $< -O2 -Wall -c -o $@

dpkg_lib.o: dpkg_lib.cc dpkg.h
	$(CXX) dpkg_lib.cc -O2 -Wall -c -o dpkg_lib.o

dpkg_popen.o: dpkg_popen.cc dpkg.h
	$(CXX) dpkg_popen.cc -O2 -Wall -c -o dpkg_popen.o

shellexp.o: shellexp.c
	$(CXX) shellexp.c -O2 -Wall -c -o shellexp.o

cruft-ng: cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o
	$(CXX) cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o -Wall -o cruft-ng

cruftlib: cruft.o explain.o filters.o mlocate.o dpkg_lib.o shellexp.o
	$(CXX) cruft.o explain.o filters.o mlocate.o dpkg_lib.o   shellexp.o -Wall -o cruftlib

test_%: %.o test_%.cc
	# TODO: dpkg_popen.o is not needed to build test_mlocate
	$(CXX) $< $@.cc dpkg_popen.o -Wall -o $@

install: all
	install -D -m 2755 -g mlocate cruft-ng   $(DESTDIR)/usr/bin/cruft-ng
	install -D -m 0644            cruft-ng.8 $(DESTDIR)/usr/share/man/man8/cruft-ng.8
	install -D -m 0644            README.md  $(DESTDIR)/usr/share/doc/cruft-ng/README.md

clean:
	rm -f cruft-ng cruftlib test_mlocate test_filters test_explain
	rm -f *.o
