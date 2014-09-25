CPP:=clang++

all: cruft tests

tests: test_mlocate

cruft.o: cruft.cc
	$(CPP) cruft.cc -O2 -Wall -c -o cruft.o

mlocate.o: mlocate.cc
	$(CPP) mlocate.cc -O2 -Wall -c -o mlocate.o

shellexp.o: shellexp.c
	$(CPP) shellexp.c -O2 -Wall -c -o shellexp.o

cruft: cruft.o mlocate.o shellexp.o
	$(CPP) cruft.o mlocate.o shellexp.o -Wall -o cruft

test_mlocate: mlocate.o test_mlocate.cc
	$(CPP) mlocate.o test_mlocate.cc -Wall -o test_mlocate

#todo: install in $(DESTDIR)
install: all
	chgrp mlocate cruft
	chmod g+s cruft
	chgrp mlocate test_mlocate
	chmod g+s test_mlocate

clean:
	rm -f cruft test_mlocate
	rm -f *.o
