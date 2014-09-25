all: cruft tests

tests: test_mlocate

gcc:
	g++ cruft.cc -Wall -ldpkg -o cruft

cruft.o: cruft.cc
	clang++ cruft.cc -O2 -Wall -c -o cruft.o

mlocate.o: mlocate.cc
	clang++ mlocate.cc -O2 -Wall -c -o mlocate.o

shellexp.o: shellexp.c
	clang++ shellexp.c -O2 -Wall -c -o shellexp.o

cruft: cruft.o mlocate.o shellexp.o
	clang++ cruft.o mlocate.o shellexp.o -Wall -o cruft

test_mlocate: mlocate.o test_mlocate.cc
	clang++ mlocate.o test_mlocate.cc -Wall -o test_mlocate

clean:
	rm -f cruft test_mlocate
	rm -f *.o
