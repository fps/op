#ifndef OP_OP_H
#define OP_OP_H

#include <boost/shared_ptr.hpp>
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

struct op;

typedef std::vector<std::string> string_vector;
typedef std::vector<boost::function2<void, frame_base&, op*> > code_vector;
typedef std::pair<unsigned int, unsigned int> function;


struct frame_base {
	frame_base(frame_base *p) 
	: 
		p(p)
	{
		//std::cout << "frame_base()" << std::endl;
	}

	frame_base *p;

	virtual ~frame_base() { }

	virtual void print() { 
		std::cout << "frame_base::print()" << std::endl; 
		throw std::runtime_error("!!!!!");
	}

	virtual void assign(frame_base *o) { 
		std::cout << "frame_base::assign()" << std::endl;
		throw std::runtime_error("!!!!!");
	}

	virtual void plus(frame_base *o, frame_base *res) {
		std::cout << "frame_base::plus()" << std::endl;
		throw std::runtime_error("!!!!!");
	}

	virtual void operator()() {
		throw std::runtime_error("!!!!!");
	}
};



template<class T>
struct frame : frame_base {
	frame(const T& t, frame_base *p) 
	: 
		frame_base(p) , t(t)
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

	virtual void operator()() {
		throw std::runtime_error("not a function");
	}
};

template<>
void frame<function>::print() {
  std::cout << "a function : " << t.first << " " << t.second << std::endl;
  throw std::runtime_error("SAY WHAT!!!!!");
}

template<>
void frame<function>::plus(frame_base *o, frame_base *res) {

}

template<>
void frame<boost::function<void(void)> >::operator()() {
	t();
}

template<class T>
frame<T> make_frame(T t, frame_base &p) {
	frame<T> f(t, &p);
	return f;
}

struct op;

template<class T>
inline void func(frame_base &f, op *o);

template<class T>
inline void var(frame<T> f, op *o);

inline void print(frame_base &f, int sp, op *o);
inline void assign(frame_base &f, int sp1, int sp2, op *o);
inline void plus(frame_base &f, int sp1, int sp2, int sp3, op *o);
inline void call(frame_base &f, int sp1, op *o);
inline void alter_ip(frame_base &f, int sp1, op *o);

struct op {
	boost::shared_ptr<code_vector> code;

	// the "instruction pointer"
	code_vector::const_iterator ip;

	frame<int> f;

	op() :
		f(0, 0)
	{

	}

	int run() {
		struct timeval tv1;
		gettimeofday(&tv1, 0);

		// std::cout << "run(const code_vector &c) " << std::endl;
		f = frame<int>(0, 0);
		ip = code->begin();
		run(f, code->end());

		struct timeval tv2;
		gettimeofday(&tv2, 0);
		std::cout 
			<< "time(ms): " 
			<< ((double)tv2.tv_sec * 1000.0 + (double)tv2.tv_usec/1000.0) 
				- ((double)tv1.tv_sec * 1000.0 + (double)tv1.tv_usec/1000.0) 
			<< std::endl;

		return f.t;
	}

	static op compile(const string_vector &s) {
		op o;
		// parse and build intermediate representation
		o.code = boost::shared_ptr<code_vector>(new code_vector());

		code_vector &c = *(o.code);

		bool in_function = false;
		int ips_function_start = 0;

		unsigned int ips_in_compile = 0;

		for (string_vector::const_iterator it = s.begin(); it != s.end(); ++it) {
			// std::cout << "--- " << *it << std::endl;
			std::stringstream str(*it);
			std::string token;
			str >> token;

			if (token == "<<<") {
				in_function = true;			
				ips_function_start = ips_in_compile;				
				std::cout << token << std::endl;
			}

			if (token == ">>>") {
				std::cout << token << " " << ips_in_compile << std::endl;
				if (false == in_function) throw std::runtime_error("not in function scope");
				in_function = false;			

				function fn = std::make_pair(ips_function_start, ips_in_compile);

				std::cout << fn.first << " " <<  fn.second << std::endl;

				boost::function<void(frame_base&,op*)> f2 = 
					boost::bind(&alter_ip, _1, ips_in_compile - ips_function_start, _2); 

				c.insert(c.begin() + ips_in_compile, f2);

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(
						&var<function>, 
						boost::bind(
							&make_frame<function>, 
							fn, 
							_1
						),  
					_2);

				c.insert(c.begin() + ips_function_start, f);
			}

			if (token == "call") {
				std::string v;
				str >> v;

				int sp1;
				std::stringstream refstr(v.substr(1));
				refstr >> sp1;

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&call, _1, sp1, _2);

				c.push_back(f);				
			}

			if (token == "int") {
				int v;
				str >> v;
				
				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&var<int>, boost::bind(&make_frame<int>, v, _1),  _2);

				c.push_back(f); 
			}

			if (token == "double") {
				double v;
				str >> v;

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&var<double>, boost::bind(&make_frame<double>, v, _1),  _2);

				c.push_back(f); 
			}

			if (token == "print") {
				std::string v;
				str >> v;

				int sp;
				std::stringstream refstr(v.substr(1));
				refstr >> sp;

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&print, _1, sp, _2);

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

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&assign, _1, sp1, sp2, _2);

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

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&plus, _1, sp1, sp2, sp3, _2);

				c.push_back(f); 
			}

			++ips_in_compile;
		}
		// std::cout << "size: " << c.size() << std::endl;
		o.run();
		return o;
	}

	void run(frame_base &fb, code_vector::const_iterator end) {
	  std::cout << "void run(frame_base &f) " << this << " " << ip - code->begin() << " " << fb.p << " " << (end - ip) << " " << code->end() - ip << std::endl;
		//if (code.end() == ip) { std::cout << "done" << std::endl; return; }

		while(end != ip) {
			std::cout << " **** " << std::endl;
			// Execute code at ip
			(*ip)(fb, this);
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

inline void alter_ip(frame_base &f, int sp1, op *o) {
	o->ip += sp1;
	std::cout << "alter_ip: " << sp1 << std::endl;
}

inline void call(frame_base &f, int sp1, op *o) {
	std::cout << "call" << std::endl;
	// (*get(&f, sp1))();
	//op o2 = *o;
	//o2.code = o->code;
	frame<function> *fr = 
		dynamic_cast<frame<function> *>(get(&f, sp1));

	if (0 == fr) 
		throw std::runtime_error("not a function");

	code_vector::const_iterator  ip = o->ip;
	o->ip = o->code->begin() + fr->t.first;

	o->run(o->f, o->code->begin() + fr->t.second);
	o->ip = ++ip;
}


inline void plus(frame_base &f, int sp1, int sp2, int sp3, op *o) {
	++(o->ip);

	std::cout << " + " << std::endl;
	get(&f, sp2)->plus(get(&f, sp3), get(&f, sp1));
}


inline void assign(frame_base &f, int sp1, int sp2, op *o) {
	++(o->ip);
	std::cout << " = " << std::endl;
	get(&f, sp1)->assign(get(&f, sp2));
}


inline void print(frame_base &f, int sp, op *o) {
	++(o->ip);
	std::cout << " print " << std::endl;
	get(&f, sp)->print();
}

template<class T>
inline void var(frame<T> f, op *o) {
	//f.ip = o->ip;

	++(o->ip);

	o->run(f, o->code->end());
}



} // namespace

#endif
