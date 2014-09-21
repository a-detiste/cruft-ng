all: cruft

gcc:
	g++ cruft.cc -Wall -ldpkg -o cruft

cruft: cruft.cc
	clang++ cruft.cc -O2 -Wall -ldpkg -o cruft

clean:
	rm -f cruft
