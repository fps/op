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

#ifndef NODEBUG
#define OP_DBG(x) { std::cout << x << std::endl; }
#else
#define OP_DBG(x) { }
#endif

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
		//std::cout << "frame_base()" )
	}

	frame_base *p;

	virtual ~frame_base() { }

	virtual void print() { 
		OP_DBG("frame_base::print()") 
		throw std::runtime_error("!!!!!");
	}

	virtual void assign(frame_base *o) { 
		OP_DBG("frame_base::assign()" )
		throw std::runtime_error("!!!!!");
	}

	virtual void plus(frame_base *o, frame_base *res) {
		OP_DBG("frame_base::plus()" )
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
		//OP_DBG("frame()" )
	}

	T t;

	virtual void print() {
		std::cout << "frame<T>::print() " << t << std::endl;
	}

	virtual void assign(frame_base *o) {
		// OP_DBG("frame<T>::assign()" )
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
  OP_DBG("a function : " << t.first << " " << t.second )
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
inline void iff(frame_base &f, int sp1, int sp2, int sp3, op *o);
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

		// OP_DBG("run(const code_vector &c) " )
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
			// OP_DBG("--- " << *it )
			std::stringstream str(*it);
			std::string token;
			str >> token;

			if (token == "<<<") {
				in_function = true;			
				ips_function_start = ips_in_compile;				
				OP_DBG(token )
			}

			if (token == ">>>") {
				OP_DBG(token << " " << ips_in_compile )
				if (false == in_function) throw std::runtime_error("not in function scope");
				in_function = false;			

				function fn = std::make_pair(ips_function_start + 2, ips_in_compile + 2);

				OP_DBG(fn.first << " " <<  fn.second )

				boost::function<void(frame_base&,op*)> f2 = 
				  boost::bind(&alter_ip, _1, 1 + (ips_in_compile - ips_function_start), _2); 

				c.insert(c.begin() + ips_function_start, f2);

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
				++(++ips_in_compile);
			}

			if (token == "if") {
				std::string v1, v2, v3;
				str >> v1 >> v2 >> v3;

				int sp1, sp2, sp3;

				std::stringstream refstr1(v1.substr(1));
				refstr1 >> sp1;

				std::stringstream refstr2(v2.substr(1));
				refstr2 >> sp2;

				std::stringstream refstr3(v3.substr(1));
				refstr3 >> sp3;

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&iff, _1, sp1, sp2, sp3,_2);

				c.push_back(f);
				++ips_in_compile;		
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
				++ips_in_compile;		
			}

			if (token == "int") {
				int v;
				str >> v;
				
				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&var<int>, boost::bind(&make_frame<int>, v, _1),  _2);

				c.push_back(f); 
				++ips_in_compile;
			}

			if (token == "bool") {
				bool v;
				str >> v;

				OP_DBG("boolean is: " << v)
				
				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&var<bool>, boost::bind(&make_frame<bool>, v, _1),  _2);

				c.push_back(f); 
				++ips_in_compile;
			}

			if (token == "double") {
				double v;
				str >> v;

				boost::function<void(frame_base&, op*)> f = 
					boost::bind(&var<double>, boost::bind(&make_frame<double>, v, _1),  _2);

				c.push_back(f); 
				++ips_in_compile;
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
				++ips_in_compile;
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
				++ips_in_compile;
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
				++ips_in_compile;
			}


		}
		// OP_DBG("size: " << c.size() )
		o.run();
		return o;
	}

	void run(frame_base &fb, code_vector::const_iterator end) {
	  OP_DBG("<<< void run(frame_base &f) " << this << " " << ip - code->begin() << " " << ip - code->begin() << " " << fb.p << " " << (end - ip) << " " << code->end() - ip )
		//if (code.end() == ip) { OP_DBG("done" ) return; }

		while(end != ip) {
		  OP_DBG(" **** " << " " << ip - code->begin() )
			// Execute code at ip
			(*ip)(fb, this);
		}
		OP_DBG(">>> run done well" )
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
	OP_DBG("alter_ip: " << sp1 )
}


inline void iff(frame_base &f, int sp1, int sp2, int sp3, op *o) {
	OP_DBG("iff")
	frame<bool> *fbool = dynamic_cast<frame<bool> *>(get(&f, sp1));

	if (0 == fbool) throw std::runtime_error("not a boolean");

	if (true == fbool->t) {
		OP_DBG("first " << sp2)
		call(f, sp2, o);
	} else {
		OP_DBG("second" << sp3)
		call(f, sp3, o);
	}
}

inline void call(frame_base &f, int sp1, op *o) {
	OP_DBG("call" )
	// (*get(&f, sp1))();
	//op o2 = *o;
	//o2.code = o->code;
	frame<function> *fr = 
		dynamic_cast<frame<function> *>(get(&f, sp1));

	if (0 == fr) 
		throw std::runtime_error("not a function");


	code_vector::const_iterator  ip = o->ip;
	o->ip = o->code->begin() + fr->t.first;

	o->run(*fr, o->code->begin() + fr->t.second);
	o->ip = ++ip;
}


inline void plus(frame_base &f, int sp1, int sp2, int sp3, op *o) {
	++(o->ip);

	OP_DBG(" + " )
	get(&f, sp2)->plus(get(&f, sp3), get(&f, sp1));
}


inline void assign(frame_base &f, int sp1, int sp2, op *o) {
	++(o->ip);
	OP_DBG(" = " )
	get(&f, sp1)->assign(get(&f, sp2));
}


inline void print(frame_base &f, int sp, op *o) {
	++(o->ip);
	OP_DBG(" print ")
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
