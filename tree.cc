#include <iostream>

#define dbg(x) std::cout << (x) << std::endl;

struct nil_ {
	void operator()() { }
};

nil_ nil;

template <class E, class T>
struct t_ {
	E e;
	T t;
	t_(E e, T t) : e(e), t(t) { }
	void operator()() { dbg("e()") e(); }
};

template <class E>
t_<E, nil_> t(E e) { return t_<E, nil_>(e,nil); }

template <class E, class T>
t_<E, T> t(E e, T t) { return t_<E, T>(e,t); }

template <class E1, class E2, class T>
t_<E1, t_<E2, T> > t(E1 e1, E2 e2, T tr) { return t(e1,t(e2, tr)); }

int main() {
	t(0.8);
	t(1, 5);
	t(0, t(t(3,7,nil), 7));
	t(
		1, 
		t(
			t(
				4
			), 
			t(
				4
			)
		)
	);
}