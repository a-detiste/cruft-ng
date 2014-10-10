CPP:=clang++

all: cruft

tests: test_mlocate cruftlib

cruft.o: cruft.cc mlocate.h dpkg.h
	$(CPP) cruft.cc -O2 -Wall -c -o cruft.o

mlocate.o: mlocate.cc mlocate.h
	$(CPP) mlocate.cc -O2 -Wall -c -o mlocate.o

dpkg_lib.o: dpkg_lib.cc dpkg.h
	$(CPP) dpkg_lib.cc -O2 -Wall -c -o dpkg_lib.o

dpkg_popen.o: dpkg_popen.cc dpkg.h
	$(CPP) dpkg_popen.cc -O2 -Wall -c -o dpkg_popen.o

shellexp.o: shellexp.c
	$(CPP) shellexp.c -O2 -Wall -c -o shellexp.o

cruft: cruft.o mlocate.o dpkg_popen.o shellexp.o
	$(CPP) cruft.o mlocate.o dpkg_popen.o shellexp.o -Wall -o cruft

cruftlib: cruft.o mlocate.o dpkg_lib.o shellexp.o
	$(CPP) cruft.o mlocate.o dpkg_lib.o shellexp.o -Wall -o cruftlib

test_mlocate: mlocate.o test_mlocate.cc
	$(CPP) mlocate.o test_mlocate.cc -Wall -o test_mlocate

#todo: install in $(DESTDIR)
install: all
	chgrp mlocate cruft
	chmod g+s cruft
	#chgrp mlocate test_mlocate
	#chmod g+s test_mlocate

clean:
	rm -f cruft cruftlib test_mlocate
	rm -f *.o
