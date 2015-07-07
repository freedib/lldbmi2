//============================================================================
// Name        : hellocpp.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

class Test {
private:
	int a;
	int b;
public:
	void seta (int v);
	void setb (int v);
	int  sumab ();
};

void Test::seta (int v) {
	a = v;
}

void Test::setb (int v) {
	b = v;
}

int Test::sumab () {
	return a+b;
}


int main() {
	Test test;
	test.seta(1);
	test.setb(2);
	cout << "!!!Hello World " << test.sumab() << endl; // prints !!!Hello World!!!
	return 0;
}
