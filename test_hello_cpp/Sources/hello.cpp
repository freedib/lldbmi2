//============================================================================
// Name        : hellocpp.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

struct K {
	int kk;
};

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

int main() {
	bool go = true;
	if (go) {
		AB ab;
		struct K k;
		k.kk = 1;
		int z=0;
		ab.seta(1+z++);
		ab.setb(2+z++);
		z = ab.sumab();
		cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
	}
	return 0;
}
