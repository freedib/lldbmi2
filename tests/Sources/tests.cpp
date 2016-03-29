
// test programs for LLDBMI2


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/////////////////////////////

typedef struct {
	short m;
	long n;
	long long o;
	char s[10];
} Y;
typedef struct {
	int a;
	const char *b;
	Y *y;
	int k[2][3];
	const char *l[4];
} Z;

Y y;
pthread_t tid;

void *
hellothread (void *arg)
{
	for (int loop=0; loop<20; loop++) {
		sleep (10);
		printf ("loop %d\n", loop);					// breakpoint 1 THREAD and ATTACH
	}
	return NULL;
}
int
startthread ()
{
	int ret = pthread_create (&tid, NULL, &hellothread, NULL);
	if (ret)
		tid = 0;
	return ret;
}
void
waitthread ()
{
	if (tid)
		pthread_join (tid, NULL);
}

int sub (int a, const char *b, Z *z) {
	int d = 33;
	printf ("hello, world: a=%d, b=%s\n", z->a, b);
	fflush (stdout);
	return a+d;
}

int test_BASE ()
{
	printf ("PID=%d\n", getpid());
	Z z;
	const char *b="22";
	Y* py = &y;
	float m[1];
	startthread ();
	y.m=11; y.n=22; y.o=33L;
	int c=12;
	z.y = &y;
	z.a = 11;
	z.b = b;
	z.k[1][1] = 4;
	z.l[1]="string 2";								// breakpoint 1 VARS
	m[0] = 123.456;
	++py->o;										// breakpoint 1 UPDATE
	c = sub (67, b, &z);							// breakpoint 2 THREAD
	c = sub (68, b, &z);
	printf ("c=%d\n", c);
	fflush (stdout);
	waitthread ();
	return 0;
}

/////////////////////////////

struct S {
	double d;
	const char *c;
};

int test_LARGE_ARRAY ()
{
	char c[101];
	int i[200];
	struct S s[200];
	c[0] = 'H';								// breakpoint 1 LARGE_ARRAY
	i[100] = 1001;
	s[0].c = "hello";
	return 0;
}

/////////////////////////////

typedef struct {
	int a;
	const char *b;
} X;
X x;

int test_POINTERS ()
{
	X *px = &x;
	x.a = 13;
	px->a = 26;
	x.b = "hello";
	px->b = "hellohello";
	return 0;
}

///////////////////////////////

#include <iostream>
using namespace std;

class BA {
protected:
	const char *n;
public:
	BA ();
	void setn  (const char *s);
};
BA::BA () {
	n = "";
}
void BA::setn (const char *s) {
	n = s;
}

class AB : public BA {
private:
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
	return a+b;								// breakpoint 1 MEMBERS
}

typedef struct CD {
	int c;
	const char *d;
} CDCD;


int test_MEMBERS()
{
	int m[] = {33,66};
	struct CD cd = {44, "444"};
	AB ab;
	ab.seta(m[0]);
	ab.setb(cd.c);
	int	z = ab.sumab();
	cout << "!!!Hello World " << z << endl; // prints !!!Hello World!!!
	return 0;
}

/////////////////////////////

int test_CRASH()
{
	int *err=NULL;
	*err=99;
	return 0;					// breakpoint 1 CRASH
}

/////////////////////////////

void testfunction (int *i, char (&sr)[7], char *s, char *ps, const char*v, double *d, bool &b, struct CD (*cdp) [3], struct CD (&cdr)[3], AB &ab, BA *pba)
{								// note: struct CD (*cd) [2] is invalid syntax for a pointer to a structure of 2 elements
	int bb=b;	(void)bb;
	return;															// breakpoint 1 ARGS
}

int test_ARGS()
{
	printf ("test_ARGS\n");
	AB ab;
	((BA *)&ab)->setn("Hello world!");
	char s[7];
	bool b=true;
	int i[3] = {5,3,1};
	struct CD cd[3] = {{10,"10"}, {20,"20"}, {30,"30"}};
	double d[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
	strcpy (s, "A");
	testfunction (i, s, s, &s[0], "a", d, b, &cd, cd, ab, &ab);		// breakpoint 2 ARGS
	strcpy (s, "B");
	b=false;														// breakpoint 3 ARGS
	ab.setb(4);
	cd[1].c=30;
	d[0] = 6.6;
	testfunction (i, s, s, &s[0], "b", d, b, NULL, cd, ab, NULL);
	d[0] = 9.9;
	testfunction (i, s, s, &s[0], "b", d, b, NULL, cd, ab, &ab);
	return 0;
}

/////////////////////////////

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

int test_STRING()
{
//	int j=1;
	std::string s;
	s = "Hello";
	ComplexClass cc;							// breakpoint 1 STRING
	return 0;
}

/////////////////////////////

int test_INPUT()
{
	char c;
	while ((c=getchar()) != '!')
		putchar (c);
	return 0;
}

/////////////////////////////

int test_CATCH_THROW()
{
	try
	{
		throw std::runtime_error("test error");
	}
	catch(const std::runtime_error& error)
	{
		printf ("catch %s\n", error.what());
	}
	return 0;
}


// execute a specific test sequence

int
main (int argc, char **argv)
{
	int test_sequence = 0;
	if (argc>1)
		sscanf (*++argv, "%d", &test_sequence);
	printf ("tests %d\n", test_sequence);

	switch (test_sequence) {		// must match test.cpp in lldbmi2
	case 1:		return test_BASE ();		// test_THREAD
	case 2:		return test_BASE ();		// test_VARS
	case 3:		return test_BASE ();		// test_UPDATE
	case 4:		return test_LARGE_ARRAY ();
	case 5:		return test_POINTERS ();
	case 6:		return test_BASE ();		// ATTACH
	case 7:		return test_MEMBERS ();
	case 8:		return test_STRING ();
	case 9:		return test_ARGS ();
	case 10:	return test_BASE ();		// OTHER
	case 11:	return test_CRASH ();
	case 12:	return test_INPUT ();
	case 13:	return test_CATCH_THROW ();
	}
	return 0;
}
