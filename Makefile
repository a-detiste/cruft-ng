CXXFLAGS ?= -O2
CXXFLAGS += -Wall

all: cruft-ng
tests: test_mlocate test_explain test_filters cruftlib

cruft.o: cruft.cc explain.h filters.h mlocate.h dpkg.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c cruft.cc

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c $<

dpkg_lib.o: dpkg_lib.cc dpkg.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c dpkg_lib.cc

dpkg_popen.o: dpkg_popen.cc dpkg.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c dpkg_popen.cc

shellexp.o: shellexp.c
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c shellexp.c

cruft-ng: cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) cruft.o explain.o filters.o mlocate.o dpkg_popen.o shellexp.o -o cruft-ng

cruftlib: cruft.o explain.o filters.o mlocate.o dpkg_lib.o shellexp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) cruft.o explain.o filters.o mlocate.o dpkg_lib.o   shellexp.o -o cruftlib

# TODO: dpkg_popen.o is not needed to build test_mlocate
test_%: %.o test_%.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $< $@.cc dpkg_popen.o -o $@

install: all
	#install -D -m 2755 -g mlocate cruft-ng   $(DESTDIR)/usr/bin/cruft-ng
	install -D -m 0755            cruft-ng   $(DESTDIR)/usr/bin/cruft-ng
	install -D -m 0644            cruft-ng.8 $(DESTDIR)/usr/share/man/man8/cruft-ng.8
	install -D -m 0644            README.md  $(DESTDIR)/usr/share/doc/cruft-ng/README.md

clean:
	rm -f cruft-ng cruftlib test_mlocate test_filters test_explain
	rm -f *.o
