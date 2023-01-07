PKG_CONFIG ?= pkg-config
LIBDPKG_LIBS = $(shell $(PKG_CONFIG) --static --libs libdpkg)
LIBDPKG_CFLAGS = $(shell $(PKG_CONFIG) --static --cflags libdpkg)

CXXFLAGS ?= -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wl,-z,relro -D_FORTIFY_SOURCE=2
CXXFLAGS += -Wall -Wextra
override CXXFLAGS += $(LIBDPKG_CFLAGS)
#CXXFLAGS += -std=c++17 #  clang++
SHARED_OBJS = explain.o filters.o shellexp.o usr_merge.o python.o owner.o
CRUFT_OBJS = cruft.o dpkg_exclude.o bugs.o

sid: cruft ruleset cpigs
buster: cruftold cpigsold

tests: test_locate test_explain test_filters test_excludes test_dpkg test_dpkg_old test_python cruftold

cpigs.o: cpigs.cc owner.h
owner.o: owner.cc owner.h
explain.o: explain.cc owner.h
filters.o: filters.cc owner.h
plocate.o: plocate.cc locate.h
mlocate.o: mlocate.cc locate.h

cruft.o: cruft.cc explain.h filters.h dpkg.h python.h
dpkg_lib.o: dpkg_lib.cc dpkg.h
dpkg_popen.o: dpkg_popen.cc dpkg.h

cruftold: $(SHARED_OBJS) $(CRUFT_OBJS) mlocate.o dpkg_popen.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) $(CRUFT_OBJS) mlocate.o dpkg_popen.o -lstdc++fs -pthread -o cruftold
cruft: $(SHARED_OBJS) $(CRUFT_OBJS) plocate.o dpkg_lib.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) $(CRUFT_OBJS) plocate.o dpkg_lib.o $(LIBDPKG_LIBS) -pthread -o cruft

cpigsold: $(SHARED_OBJS) cpigs.o mlocate.o dpkg_popen.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) cpigs.o mlocate.o dpkg_popen.o -lstdc++fs -o cpigsold
cpigs: $(SHARED_OBJS) cpigs.o plocate.o dpkg_lib.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SHARED_OBJS) cpigs.o plocate.o dpkg_lib.o $(LIBDPKG_LIBS) -o cpigs

test_%: %.o test_%.cc dpkg_lib.o usr_merge.o $(LIBDPKG_LIBS)
test_dpkg_old: dpkg_popen.o test_dpkg.cc usr_merge.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) test_dpkg.cc usr_merge.o dpkg_popen.o -o test_dpkg_old
test_dpkg: dpkg_lib.o test_dpkg.cc usr_merge.o $(LIBDPKG_LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) test_dpkg.cc usr_merge.o dpkg_lib.o $(LIBDPKG_LIBS) -Wl,--no-demangle -o test_dpkg

ifdef $(BUSTER):
test_locate: test_mlocate
else
test_locate: test_plocate
endif
test_mlocate: test_locate.cc mlocate.o python.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) test_locate.cc mlocate.o python.o -lstdc++fs -o test_mlocate
test_plocate: test_locate.cc plocate.o python.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) test_locate.cc plocate.o python.o -o test_plocate

test_python: python.o test_python.cc
test_excludes: dpkg_exclude.o test_excludes.cc
test_diversions: test_diversions.cc dpkg_popen.o usr_merge.o
test_explain: test_explain.cc explain.o dpkg_lib.o usr_merge.o owner.o $(LIBDPKG_LIBS)
test_filters: test_filters.cc filters.o dpkg_lib.o usr_merge.o owner.o $(LIBDPKG_LIBS)

clean:
	rm -f cpigs cruft cruftold ruleset test_?locate test_explain test_filters test_excludes test_dpkg test_dpkg_old test_diversions test_python test_bugs
	rm -f *.o

ruleset: rules/*
	echo Checking for trailing whitespaces
	grep -E -R -H " +$$" rules/ || true
	! grep -E -R -q " +$$" rules/
	./ruleset.sh

flow.png: flow.ditaa
	ditaa flow.ditaa flow.png

doc: flow.png
