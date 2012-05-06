#ifndef OP_OP_H
#define OP_OP_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

#include <sys/time.h>

namespace op {

struct frame_base;

typedef std::vector<std::string> string_vector;
typedef std::vector<boost::function<void(frame_base&)> > code_vector;


struct frame_base {
	frame_base(frame_base *p, code_vector::const_iterator ip) 
	: 
		p(p), ip(ip) 
	{
		//std::cout << "frame_base()" << std::endl;
	}

	frame_base *p;
	code_vector::const_iterator ip;

	virtual ~frame_base() { }

	virtual void print() { 
		std::cout << "frame_base::print()" << std::endl; 
	}

	virtual void assign(frame_base *o) { 
		std::cout << "frame_base::assign()" << std::endl;
	}

	virtual void plus(frame_base *o, frame_base *res) {
		std::cout << "frame_base::plus()" << std::endl;
	}
};



template<class T>
struct frame : frame_base {
	frame(const T& t, frame_base *p, code_vector::const_iterator ip) 
	: 
		t(t), frame_base(p, ip) 
	{
		//std::cout << "frame()" << std::endl;
	}

	T t;

	virtual void print() {
		std::cout << "frame<T>::print() " << t << std::endl;
	}

	virtual void assign(frame_base *o) {
		// std::cout << "frame<T>::assign()" << std::endl;
		frame<T> *co = dynamic_cast<frame<T> *>(o);

		if (0 == co) throw std::runtime_error("type mismatch");
		t = co->t;
	}

	virtual void plus(frame_base *o, frame_base *res) {
		frame<T> *co = dynamic_cast<frame<T> *>(o);
		frame<T> *cres = dynamic_cast<frame<T> *>(res);

		if (0 == co || 0 == cres) throw std::runtime_error("type mismatch");
		cres->t = t + co->t;
	}
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

inline void print(frame_base &f, int sp, op *o);
inline void assign(frame_base &f, int sp1, int sp2, op *o);
inline void plus(frame_base &f, int sp1, int sp2, int sp3, op *o);

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
		struct timeval tv1;
		gettimeofday(&tv1, 0);

		// std::cout << "run(const code_vector &c) " << std::endl;
		f = frame<int>(0, 0, c.begin());
		code = c;
		ip = code.begin();
		run(f);

		struct timeval tv2;
		gettimeofday(&tv2, 0);
		std::cout 
			<< "time(ms): " 
			<< ((double)tv2.tv_sec * 1000.0 + (double)tv2.tv_usec/1000.0) 
				- ((double)tv1.tv_sec * 1000.0 + (double)tv1.tv_usec/1000.0) 
			<< std::endl;

		return f.t;
	}

	static int run(const string_vector &s) {
		op o;
		// parse and build intermediate representation
		code_vector c;

		for (string_vector::const_iterator it = s.begin(); it != s.end(); ++it) {
			// std::cout << "--- " << *it << std::endl;
			std::stringstream str(*it);
			std::string token;
			str >> token;

			if (token == "int") {
				int v;
				str >> v;
				
				boost::function<void(frame_base&)> f = 
					boost::bind(&var<int>, boost::bind(&make_frame<int>, v, _1, o.f.ip),  &o);

				c.push_back(f); 
			}

			if (token == "double") {
				double v;
				str >> v;

				boost::function<void(frame_base&)> f = 
					boost::bind(&var<double>, boost::bind(&make_frame<double>, v, _1, o.f.ip),  &o);

				c.push_back(f); 
			}

			if (token == "print") {
				std::string v;
				str >> v;

				int sp;
				std::stringstream refstr(v.substr(1));
				refstr >> sp;

				boost::function<void(frame_base&)> f = 
					boost::bind(&print, _1, sp, &o);

				c.push_back(f); 
			}

			if (token == "=") {
				std::string ssp1, ssp2;
				str >> ssp1 >> ssp2;

				int sp1, sp2;
				std::stringstream refstr1(ssp1.substr(1));
				refstr1 >> sp1;

				std::stringstream refstr2(ssp2.substr(1));
				refstr2 >> sp2;

				boost::function<void(frame_base&)> f = 
					boost::bind(&assign, _1, sp1, sp2, &o);

				c.push_back(f); 
			}

			if (token == "+") {
				std::string ssp1, ssp2, ssp3;
				str >> ssp1 >> ssp2 >> ssp3;

				int sp1, sp2, sp3;
				std::stringstream refstr1(ssp1.substr(1));
				refstr1 >> sp1;

				std::stringstream refstr2(ssp2.substr(1));
				refstr2 >> sp2;

				std::stringstream refstr3(ssp3.substr(1));
				refstr3 >> sp3;

				boost::function<void(frame_base&)> f = 
					boost::bind(&plus, _1, sp1, sp2, sp3, &o);

				c.push_back(f); 
			}
		}
		// std::cout << "size: " << c.size() << std::endl;

		return o.run(c);
	}

	void run(frame_base &f) {
		// std::cout << "void run(frame_base &f) " << f.p << " " << (code.end() - ip) << std::endl;
		//if (code.end() == ip) { std::cout << "done" << std::endl; return; }

		while(code.end() != ip) {
			// Execute code at ip
			(*ip)(f);
		}
	}
};

// get the sp-th frame from the top of the stack
frame_base *get(frame_base *top, int sp) {
	while(0 != sp) {
		top = top->p;
		--sp;
	}
	return top;
}

inline void plus(frame_base &f, int sp1, int sp2, int sp3, op *o) {
	++(o->ip);

	get(&f, sp2)->plus(get(&f, sp3), get(&f, sp1));
}


inline void assign(frame_base &f, int sp1, int sp2, op *o) {
	++(o->ip);
	
	get(&f, sp1)->assign(get(&f, sp2));
}


inline void print(frame_base &f, int sp, op *o) {
	++(o->ip);

	get(&f, sp)->print();
}

template<class T>
inline void var(frame<T> f, op *o) {
	++(o->ip);

	o->run(f);
}



} // namespace

#endif
