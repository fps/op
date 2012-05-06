#include "op.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
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

	// std::cout << "run: " << code.size() << std::endl;
	op::op::run(code);
}
