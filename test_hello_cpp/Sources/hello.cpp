
//#define TEST_CRASH
//#define TEST_ARGS
//#define TEST_MEMBERS
#define TEST_STRING

#include <iostream>
using namespace std;

#include <string.h>

class AB {
public:
	int a;
	int b;
	int c;
public:
	AB ();
	void seta  (int v);
	void setb  (int v);
	int  sumab ();
};
AB::AB () {
	a = 0;
	b = 0;
	c = 0;
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

typedef struct CD {
	int c;
	const char *d;
} CDCD;

#ifdef TEST_MEMBERS
int main()
{
	int m[] = {33,66};
	struct CD cd = {44, "444"};
	AB ab;
	ab.seta(11);
	ab.setb(22);
	int	z = ab.sumab();
	cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
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
void testfunction (int *i, char (&sr)[7], char *s, char *ps, const char*v, double *d, bool &b, struct CD (*cdp) [3], struct CD (&cdr)[3], AB &ab)
{								// note: struct CD (*cd) [2] is invalid syntax for a pointer to a structure of 2 elements
	int bb=b;	(void)bb;
	return;
}
int main()
{
	AB ab;
	char s[7];
	bool b=true;
	int i[3] = {5,3,1};
	struct CD cd[3] = {{10,"10"}, {20,"20"}, {30,"30"}};
	double d[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
	strcpy (s, "A");
	testfunction (i, s, s, &s[0], "a", d, b, &cd, cd, ab);
	strcpy (s, "B");
	b=false;
	ab.setb(4);
	cd[1].c=30;
	d[0] = 6.6;
	testfunction (i, s, s, &s[0], "b", d, b, NULL, cd, ab);
	d[0] = 9.9;
	testfunction (i, s, s, &s[0], "b", d, b, NULL, cd, ab);
}
#endif

#include <vector>

class ComplexClass
{
    public:
		ComplexClass();
        virtual ~ComplexClass() {}

        typedef enum {
            v1, v2, v3, v4
        } valueslist;

        std::vector<uint8_t> data;
        valueslist vl;
        uint16_t number;
};
ComplexClass::ComplexClass () : vl(v1), number(0) {
}

#ifdef TEST_STRING
int main()
{
//	int j=1;
	std::string s;
	s = "Hello";
	ComplexClass cc;
}
#endif

