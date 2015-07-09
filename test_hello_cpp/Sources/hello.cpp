//============================================================================
// Name        : hellocpp.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

class AB {
public:
	int a;
	int b;
public:
	AB ();
	void seta  (int v);
	void setb  (int v);
	int  sumab ();
};
AB::AB () {
	a = 0;
	b = 0;
}
void AB::seta (int v) {
	a = v;
}
void AB::setb (int v) {
	b = v;
}
int AB::sumab () {
	return a+b;
}

struct CD {
	int c;
	const char *d;
};

int main() {
#if 0
	bool go = true;
	if (go) {
		AB ab;
		struct CD cd;
		cd.c = "Hello world";
		int z=0;
		ab.seta(1+z++);
		ab.setb(2+z++);
		z = ab.sumab();
		cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
	}
#else
	int c;
	const char *d;
	struct CD cd;
	d="hello";
	cd.d="hello";
	c=33;
	cd.c=33;
	d="a";
	cd.d="a";
	d="b";
	cd.d="b";
#endif
	return 0;
}
