CXXFLAGS ?= -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wl,-z,relro -D_FORTIFY_SOURCE=2
CXXFLAGS += -Wall
SHARED_OBJS = cruft.o dpkg_exclude.o explain.o filters.o mlocate.o plocate.o shellexp.o usr_merge.o python.o

all: check cruft
tests: test_plocate test_explain test_filters test_excludes test_dpkg test_python cruftlib

cruft.o: cruft.cc explain.h filters.h mlocate.h dpkg.h python.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c cruft.cc

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c $<

dpkg_lib.o: dpkg_lib.cc dpkg.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c dpkg_lib.cc

dpkg_popen.o: dpkg_popen.cc dpkg.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c dpkg_popen.cc

shellexp.o: shellexp.c
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) -c shellexp.c

cruft: $(SHARED_OBJS) dpkg_popen.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_popen.o -o cruft

cruftlib: $(SHARED_OBJS) dpkg_lib.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_lib.o   -o cruftlib

test_%: %.o test_%.cc dpkg_popen.o usr_merge.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $< $@.cc dpkg_popen.o usr_merge.o -o $@
test_dpkg: test_dpkg.cc dpkg_popen.o usr_merge.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) dpkg_popen.o usr_merge.o test_dpkg.cc -o $@
test_mlocate: mlocate.o test_mlocate.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) mlocate.o test_mlocate.cc -o $@
test_plocate: plocate.o test_plocate.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) plocate.o test_plocate.cc -o $@
test_python: python.o test_python.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) python.o test_python.cc -o $@
test_excludes: dpkg_exclude.o test_excludes.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) dpkg_exclude.o test_excludes.cc -o $@
test_diversions: test_diversions.cc dpkg_popen.o usr_merge.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) test_diversions.cc dpkg_popen.o usr_merge.o -o $@

install: all
	#install -D -m 2755 -g mlocate cruftg   $(DESTDIR)/usr/bin/cruft
	install -D -m 0755            cruft   $(DESTDIR)/usr/bin/cruft
	install -D -m 0644            cruft.8 $(DESTDIR)/usr/share/man/man8/cruft.8
	install -D -m 0644            cruft.8 $(DESTDIR)/usr/share/man/man8/cruft-ng.8
	install -D -m 0644            README.md  $(DESTDIR)/usr/share/doc/cruft/README.md

clean:
	rm -f cruft cruftlib test_mlocate test_plocate test_explain test_filters test_excludes test_dpkg test_diversions test_python
	rm -f *.o

check:
	echo Checking for trailing whitespaces
	grep -E -R -H " +$$" rules/ || true
	! grep -E -R -q " +$$" rules/
	echo Checking for trailing slashes
	grep -E -R -H "/+$$" rules/ || true
	! grep -E -R -q "/+$$" rules/
