PKG_CONFIG ?= pkg-config
LIBDPKG_LIBS = $(shell $(PKG_CONFIG) --static --libs libdpkg)
LIBDPKG_CFLAGS = $(shell $(PKG_CONFIG) --static --cflags libdpkg)
JSONCPP_LIBS = $(shell $(PKG_CONFIG) --libs jsoncpp)

CXXFLAGS ?= -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wl,-z,relro -D_FORTIFY_SOURCE=2
CXXFLAGS += -Wall
CXXFLAGS += $(LIBDPKG_CFLAGS)
#CXXFLAGS += -std=c++17 #  clang++
SHARED_OBJS = cruft.o dpkg_exclude.o explain.o filters.o plocate.o shellexp.o usr_merge.o python.o owner.o

all: cruft ruleset cpigs
tests: test_plocate test_explain test_filters test_excludes test_dpkg test_dpkg_old test_python cruftold

cpigs.o: cpigs.cc owner.h
owner.o: owner.cc owner.h
explain.o: explain.cc owner.h
filters.o: filters.cc owner.h

cruft.o: cruft.cc explain.h filters.h dpkg.h python.h
dpkg_lib.o: dpkg_lib.cc dpkg.h
dpkg_popen.o: dpkg_popen.cc dpkg.h
shellexp.o: shellexp.c

cruftold: $(SHARED_OBJS) dpkg_popen.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_popen.o -o cruftold
cruft: $(SHARED_OBJS) dpkg_lib.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) dpkg_lib.o $(LIBDPKG_LIBS) -o cruft

cpigsold: cpigs.o explain.o filters.o plocate.o shellexp.o usr_merge.o python.o dpkg_popen.o owner.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) cpigs.o explain.o filters.o plocate.o shellexp.o usr_merge.o python.o dpkg_popen.o owner.o -lstdc++fs $(JSONCPP_LIBS) -o cpigsold
cpigs: cpigs.o explain.o filters.o plocate.o shellexp.o usr_merge.o python.o dpkg_lib.o owner.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) cpigs.o explain.o filters.o plocate.o shellexp.o usr_merge.o python.o dpkg_lib.o owner.o $(LIBDPKG_LIBS) $(JSONCPP_LIBS) -o cpigs

test_%: %.o test_%.cc dpkg_lib.o usr_merge.o $(LIBDPKG_LIBS)
test_dpkg_old: dpkg_popen.o test_dpkg.cc usr_merge.o
test_dpkg: dpkg_lib.o test_dpkg.cc usr_merge.o $(LIBDPKG_LIBS)
test_plocate: plocate.o python.o test_plocate.cc
test_python: python.o test_python.cc
test_excludes: dpkg_exclude.o test_excludes.cc
test_diversions: test_diversions.cc dpkg_popen.o usr_merge.o
test_explain: test_explain.cc explain.o dpkg_lib.o usr_merge.o owner.o $(LIBDPKG_LIBS)
test_filters: test_filters.cc filters.o dpkg_lib.o usr_merge.o owner.o $(LIBDPKG_LIBS)

clean:
	rm -f cpigs cruft cruftold test_plocate test_explain test_filters test_excludes test_dpkg test_dpkg_old test_diversions test_python
	rm -f *.o

ruleset: rules/*
	echo Checking for trailing whitespaces
	grep -E -R -H " +$$" rules/ || true
	! grep -E -R -q " +$$" rules/
	echo Checking for trailing slashes
	grep -E -R -H "/+$$" rules/ || true
	! grep -E -R -q "/+$$" rules/
	./ruleset.sh

flow.png: flow.ditaa
	ditaa flow.ditaa flow.png
