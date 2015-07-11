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


#define TEST_CRASH

int main() {
#ifdef TEST_CRASH
	int *err=NULL;
	*err=99;
#else
	bool go = true;
	if (go) {
		AB ab;
		int z=0;
		ab.seta(1+z++);
		ab.setb(2+z++);
		z = ab.sumab();
		cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
	}
#endif
	return 0;
}
