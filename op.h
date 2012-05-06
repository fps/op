#ifndef OP_OP_H
#define OP_OP_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace op {

typedef double number;

struct frame_base {
	frame_base(frame_base *p) : p(p) { }
	frame_base *p;
};

template<class T>
struct frame : frame_base {
	frame(const T& t = T(), frame_base *p = 0) 
	: 
		t(t), frame_base(p) 
	{ }

	T t;
};

template<class T>
frame<T> make_frame(T t, frame_base *p) {
	frame<T> f(t,p);
	return f;
}

struct op;

template<class T>
inline void func(frame<T> f, op *o);

template<class T>
inline void var(frame<T> f, op *o);

struct op {
	typedef std::vector<std::string> string_vector;
	typedef std::vector<boost::function<void(void)> > code_vector;
	code_vector code;
	
	// the "instruction pointer"
	code_vector::const_iterator ip;

	frame<int> f;

	op() :
		f(0, 0)
	{

	}

	int run(const code_vector &c) {
		std::cout << "run(const code_vector &c) " << std::endl;
		f = frame<int>(0,0);
		code = c;
		ip = code.begin();
		run(f);
		return f.t;
	}

	static int run(const string_vector &s) {
		op o;
		// parse and build intermediate representation
		code_vector c;

		for (string_vector::const_iterator it = s.begin(); it != s.end(); ++it) {
			std::cout << "--- " << *it << std::endl;
			std::stringstream str(*it);
			std::string token;
			str >> token;

			if (token == "int") {
				int v;
				str >> v;
				boost::function<void(void)> f = boost::bind(&var<int>, make_frame(v,0),  &o);
				c.push_back(f); 
			}

			if (token == "double") {
				double v;
				str >> v;
				boost::function<void(void)> f = boost::bind(&var<double>, make_frame(v,0),  &o);
				c.push_back(f); 
			}
		}
		std::cout << "size: " << c.size() << std::endl;

		o.run(c);
		return o.f.t;
	}

	void run(frame_base &f) {
		std::cout << "void run(frame_base &f) " << f.p << " " << (code.end() - ip) << std::endl;
		if (code.end() == ip) { std::cout << "done" << std::endl; return; }

		(*ip)();
	}
};

template<class T>
inline void var(frame<T> f, op *o) {
	std::cout << "var(frame<T> f, op *o) " << f.t << std::endl;
	// bump instruction pointer
	++(o->ip);

	// call back to the executor
	o->run(f);
}



} // namespace

#endif
