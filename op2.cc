#include "op2.h"

int main() {
	int store[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	eval(
		seq
			(nop())
			(seq
				(nop())
				(nop())
				(print(var(store[0])))
				(nop()))
	);
}