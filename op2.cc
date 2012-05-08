#include <iostream>

#define NOP_DBG(x) { std::cout << x << std::endl; }

struct nop {
	nop() {
		NOP_DBG("nop")
	}

	void eval() { }
};


template <class E1, class E2>
struct sequence {
	E1 e1;
	E2 e2;
	sequence(E1 e1, E2 e2) : e1(e1), e2(e2) { } 

	typedef sequence<E1, E2> seq_type;	

	template<class E3>
	sequence<seq_type, E3> operator()(E3 e3) {
		return sequence<seq_type, E3>(*this, e3);
	}

	void eval() {
		e1.eval();
		e2.eval();
	}
};

template <class E1>
sequence<E1, nop> seq(E1 e1 = E1()) {
	return sequence<E1, nop>(e1, nop());
}

template <class E1, class E2>
sequence<E1, E2> seq(E1 e1 = E1(), E2 e2 = E2()) {
	return sequence<E1, E2>(e1, e2);
}


template<class T>
struct variable {
	T &t;

	variable(T &t) : t(t) { }

	T &eval() { return t; }
};

template<class T>
variable<T> var(T &t) {
	return variable<T>(t);
}


template<class T>
struct printer {
	T v;

	printer(T v) : v(v) { }

	void eval() {
		NOP_DBG("hj")
		// std::cout << v.eval() << std::endl;
	}
};

template<>
void printer<variable<int> >::eval() {
		std::cout << v.eval() << std::endl;
}

template<class T>
printer<T> print(T t) {
	return printer<T>(t);
}

template<class E1>
void eval(E1 e1) {
	e1.eval();
}

int main() {
	int store[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	eval(
		seq
			(nop())
			(seq
				(nop())
				(nop())
				(print(var(store[0])))
				(nop()))
	);
}