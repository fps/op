op: op.cc op.h
	g++ -O3 -funroll-loops -march=native -o op op.cc