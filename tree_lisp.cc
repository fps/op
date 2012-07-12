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

//template <>
//struct t_<nil_, nil_> {
//	t_(nil_ n1, nil_ n2) { } 
//	void operator()(nil_) {  }
//};


template <class E, class T>
t_<E, T> t(E e, T t) { return t_<E, T>(e,t); }

template<class T>
struct v_ {
	T t;
	v_(T t) : t(t) { } 
	void operator()() { } 
};

template <class T>
v_<T> v(T t) { return v_<T>(t); }

//template <class E, class T>
//v_<E, T> v(E e, T t) { return v_<E, T>(e,t); }

template <class T>
struct plus_ {
	T t;
	plus_(T t) : t(t) { }

	void operator()() { } 
};

template <class T>
plus_<T> plus(T t) { return plus_<T>(t); }


int main() {
	t(
		v(1), 
		t(
			t(
				v(4),
				nil
			), 
			t(
				v(4),
				nil
			)
		)
	)();
}