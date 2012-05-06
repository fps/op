#ifndef OP_OP_H
#define OP_OP_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace op {

struct frame_base;

typedef std::vector<std::string> string_vector;
typedef std::vector<boost::function<void(frame_base&)> > code_vector;


struct frame_base {
	frame_base(frame_base *p, code_vector::const_iterator ip) 
	: 
		p(p), ip(ip) 
	{ }

	frame_base *p;
	code_vector::const_iterator ip;
};



template<class T>
struct frame : frame_base {
	frame(const T& t, frame_base *p, code_vector::const_iterator ip) 
	: 
		t(t), frame_base(p, ip) 
	{ }

	T t;
};

template<class T>
frame<T> make_frame(T t, frame_base &p, code_vector::const_iterator ip) {
	frame<T> f(t, &p, ip);
	return f;
}

struct op;

template<class T>
inline void func(frame<T> f, op *o);

template<class T>
inline void var(frame<T> f, op *o);

struct op {
	code_vector code;
	
	// the "instruction pointer"
	code_vector::const_iterator ip;

	frame<int> f;

	op() :
		f(0, 0, code.begin())
	{

	}

	int run(const code_vector &c) {
		std::cout << "run(const code_vector &c) " << std::endl;
		f = frame<int>(0, 0, c.begin());
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
				
				boost::function<void(frame_base)> f = boost::bind(&var<int>, boost::bind(&make_frame<int>, v, _1, o.f.ip),  &o);
				c.push_back(f); 
			}

			if (token == "double") {
				double v;
				str >> v;

				boost::function<void(frame_base)> f = boost::bind(&var<int>, boost::bind(&make_frame<int>, v, _1, o.f.ip),  &o);
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

		// Execute code at ip
		(*ip)(f);
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
