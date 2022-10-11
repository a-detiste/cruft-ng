PKG_CONFIG ?= pkg-config
LIBDPKG_LIBS = $(shell $(PKG_CONFIG) --static --libs libdpkg)
LIBDPKG_CFLAGS = $(shell $(PKG_CONFIG) --static --cflags libdpkg)
CXXFLAGS ?= -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wl,-z,relro -D_FORTIFY_SOURCE=2
CXXFLAGS += -Wall
CXXFLAGS += $(LIBDPKG_CFLAGS)
SHARED_OBJS = cruft.o dpkg_exclude.o explain.o filters.o mlocate.o plocate.o shellexp.o usr_merge.o python.o

all: cruft ruleset
tests: test_plocate test_explain test_filters test_excludes test_dpkg test_dpkg_lib test_python cruftold

cruft.o: cruft.cc explain.h filters.h mlocate.h dpkg.h python.h
dpkg_lib.o: dpkg_lib.cc dpkg.h
dpkg_popen.o: dpkg_popen.cc dpkg.h
shellexp.o: shellexp.c

cruftold: $(SHARED_OBJS) dpkg_popen.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_popen.o -o cruftold

cruft: $(SHARED_OBJS) dpkg_lib.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_lib.o $(LIBDPKG_LIBS) -o cruft

test_%: %.o test_%.cc dpkg_popen.o usr_merge.o
test_dpkg: test_dpkg.cc dpkg_popen.o usr_merge.o
test_dpkg_lib: test_dpkg.cc dpkg_lib.o LDLIBS=$(LIBDPKG_LIBS)
test_mlocate: mlocate.o test_mlocate.cc
test_plocate: plocate.o python.o test_plocate.cc
test_python: python.o test_python.cc
test_excludes: dpkg_exclude.o test_excludes.cc
test_diversions: test_diversions.cc dpkg_popen.o usr_merge.o

install: all
	#install -D -m 2755 -g mlocate cruftg   $(DESTDIR)/usr/bin/cruft
	install -D -m 0755            cruft   $(DESTDIR)/usr/bin/cruft
	install -D -m 0644            cruft.8 $(DESTDIR)/usr/share/man/man8/cruft.8
	install -D -m 0644            cruft.8 $(DESTDIR)/usr/share/man/man8/cruft-ng.8
	install -D -m 0644            README.md  $(DESTDIR)/usr/share/doc/cruft/README.md

clean:
	rm -f cruft cruftold test_mlocate test_plocate test_explain test_filters test_excludes test_dpkg test_dpkg_lib test_diversions test_python
	rm -f *.o

ruleset: rules/*
	echo Checking for trailing whitespaces
	grep -E -R -H " +$$" rules/ || true
	! grep -E -R -q " +$$" rules/
	echo Checking for trailing slashes
	grep -E -R -H "/+$$" rules/ || true
	! grep -E -R -q "/+$$" rules/
	./ruleset.sh
