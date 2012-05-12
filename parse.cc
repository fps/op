#include <iostream>
#include <fstream>
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;


namespace parser {
  using namespace qi;

  template<class Iterator>
  struct parser {
    rule<Iterator> rule;

    //    parser(Iterator it) {

    //    }
  };
  
} // namespace

int main(int argc, char *argv[]) {
  std::ifstream input("/dev/stdin"); 
  parser::parser<std::istream_iterator<std::ifstream> > p;

  std::cout << "(";

  std::string line;
  unsigned int last_indent = 0;
  std::string last_token = "";
  while(input.good()) {
    std::getline(input, line);
   
    // Count the number of tabs at the beginning of the line
    unsigned int num_of_tabs = 0;
    while(line.length() > num_of_tabs && line.substr(num_of_tabs, 1) == "\t") {
      ++num_of_tabs;
    }

    std::string token = line.substr(num_of_tabs);

    int tab_diff = num_of_tabs - last_indent;

    last_indent = num_of_tabs;

    if (tab_diff > 0) {
      for (unsigned int index = 0; index < tab_diff; ++index) {
	std::cout << "(" << last_token;
      }
    }

    if (tab_diff < 0) {
      for (unsigned int index = 0; index < -tab_diff; ++index) {
	std::cout << ")";
      }
    }
  
    // std::cout << "tabs: " << num_of_tabs << std::endl;
    last_token =  token;
  }

  std::cout << ")";
}

