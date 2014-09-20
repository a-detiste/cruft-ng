all: cruft

gcc:
	g++ cruft.cc -Wall -ldpkg -o cruft

cruft: cruft.cc
	clang++ cruft.cc -Wall -ldpkg -o cruft

clean:
	rm -f cruft
