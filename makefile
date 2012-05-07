CXXFLAGS = -DNODEBUG

op: op.cc op.h makefile
	g++ $(CXXFLAGS) -I /opt/local/include/ -Wall -g  -o op op.cc


.PHONY: clean

clean:
	rm -rf op