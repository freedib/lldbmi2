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


//#define TEST_MEMBERS
//#define TEST_CRASH
#define TEST_ARGS

#ifdef TEST_MEMBERS
int main()
{
	bool go = true;
	if (go) {
		AB ab;
		int z=0;
		ab.seta(1+z++);
		ab.setb(2+z++);
		z = ab.sumab();
		cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
	}
	return 0;
}
#endif

#ifdef TEST_CRASH
int main()
{
	int *err=NULL;
	*err=99;
	return 0;
}
#endif

#ifdef TEST_ARGS
void testfunction (char *s, const char*v)
{
	return;
}
int main()
{
	char s[10];
	strcpy (s, "A");
	testfunction (s, "a");
	strcpy (s, "B");
	testfunction (s, "b");
}
#endif


