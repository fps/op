#include <vector>
#include <algorithm>
#include <functional>

typedef float num;

template<int channel>
struct in {
	std::vector<num>  out;

	// in(int n) : n(n) { }

	void operator()(num *vars) {

	}
};

#define unary_opp(name, eval) \
template<class U> \
struct name { \
	U u; \
	name(U u) : u(u) { } \
 \
	U operator()() { \
		eval \
	} \
 \
}; \

//unary_op(minus, { return -u; })

template<class U, class F, int channels>
struct unary {
	std::vector<std::vector<num> > outs;

	U u;
	F f;

	// unary(U u, F f) : u(u), f(f) { }

	void operator()() {
		for (unsigned int channel; channel < channels; ++channel) {
			u();
			std::transform(u.outs[channel].begin(), u.outs[channel].end(), outs[channel].begin(), f);
		}
	}
};

struct minus1 {
	num operator()(num &t) { return -t; } 
};

template<class U, int channels> 
struct minus : unary<U, minus1, channels> {
	void unary<U, minus1, channels>::operator();
};


template<class U, int channels>
unary<U, minus1, channels> minus2(U u) {
	return unary<U, minus1, channels>(u, minus1());
}


template<class U1, class U2, class U3>
struct sin {
	U1 u1; U2 u2; U3 u3;

	// sin(U1 u1, U2 u2, U3 u3) {

	// }
	void operator()(num *vars) {
		u1(vars);
		u2(vars);
		u3(vars);
	}
};

template<int variables, class U>
struct synth {
	U u;

	num vars[variables];
	// synth_(U u) : u(u) { }

	void operator()(...) {
		
		u(vars);
	}
};

// template<class U>
// synth_<U> synth(U u) { return synth_<U>(u); } 

struct const_num {
	num t;
};


struct _1 {
	std::vector<num> out;
	_1() { }
	void operator()(num *vars) {
		
	}	
};

struct _2 {
	void operator()(num *vars) {
	}
};

int main() {
	synth<
		2,
		sin<
			_1, 
			_2, 
			in<2> 
		> 
	>();
}