#ifndef OP_OP_H
#define OP_OP_H

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace op {

typedef double number;

struct frame_base {
	frame_base(frame_base *p) : p(p) { }
	frame_base *p;
};

template<class T>
struct frame : frame_base {
	frame(const T& t = T(), frame_base *p = 0) : frame_base(p) { }

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
	op() : ip(0) {

		frame<int> f;
		f.t = 0;

		run(f);
	}

	int ip;

	void run(frame_base &f) {
		while(true) {
			++ip;
			std::cout << "ip: " << ip << std::endl;
			switch(ip) {
				case 0:
					var(make_frame(0, &f), this);
				break;

				case 1:
					var(make_frame(0.0d, &f), this);
				break;

				case 2: {
					// function definition
					boost::function<void(void)> fn = 
						boost::bind(&op::run, this, f);

					var(make_frame(fn, &f), this);
				}
				break;

				case 100:
					--ip; 
					return;
				break;

				default: break;
			}
		}

	}
};

template<class T>
inline void var(frame<T> f, op *o) {
	o->run(f);
}



} // namespace

#endif
