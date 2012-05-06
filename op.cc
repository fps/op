#include "op.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
	if (argc < 2) { std::cout << "usage op [file.op]" << std::endl; exit(1); }

	std::ifstream f(argv[1]);
	std::vector<std::string> code;

	while(f.good()) {
		std::string line;
		std::getline(f, line);

		if (
			false == line.empty() &&
			'#' != line[0]
		) {
			code.push_back(line);
			// std::cout << "+++ " <<  line << std::endl;
		}
	}

	op::op op = op::op::compile(code);
	op.run();
}
