
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

int test_LARGE_CHAR_ARRAY ()
{
	const char *ddd="HI\"abc\"JK";
	char ccc[102];
	strcpy (ccc, ddd);
	strcpy (&ccc[100], "K");						// breakpoint 1 LARGE_CHAR_ARRAY

	return 0;
}

/////////////////////////////

typedef struct {	// can view structs only if typedef in dwarf
	double d;
	const char *c;
} SSS;

int test_LARGE_ARRAY ()
{
	int i[200];
	SSS s[200];

	i[100] = 1001;
	s[0].c = "hello";

	return 0;									// breakpoint 1 LARGE_ARRAY
}

/////////////////////////////

typedef struct {
	int a;
	const char *b;
} X;
X x;

int test_POINTERS ()
{
	X *px = &x;									// breakpoint 1 POINTERS
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
	return 0;								// breakpoint 1 CRASH
}

/////////////////////////////

void testfunction (int *i, char (&sr)[7], char *s, char *ps, const char*v, double *d, bool &b, struct CD (*cdp) [3], struct CD (&cdr)[3], AB &ab, BA *pba)
{
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
	return 0;									// breakpoint 1 INPUT
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
	return 0;									// breakpoint 1 CATCH_THROW
}

///////////////////////////////

class BigClass
{
    public:
		BigClass() {
			number = 9;
			vl = v2;
			v010=v011=v012=v013=v014=v015=v016=v017=v018=v019=v1;
			v030=v031=v032=v033=v034=v035=v036=v037=v038=v039=0;
			v040=v041=v042=v043=v044=v045=v046=v047=v048=v049=0;
			v050=v051=v052=v053=v054=v055=v056=v057=v058=v059=0;
			v060=v061=v062=v063=v064=v065=v066=v067=v068=v069=0;
			v070=v071=v072=v073=v074=v075=v076=v077=v078=v079=0;
			v080=v081=v082=v083=v084=v085=v086=v087=v088=v089=0;
			v090=v091=v092=v093=v094=v095=v096=v097=v098=v099=0;
			v100=v101=v102=v103=v104=v105=v106=v107=v108=v109=0;
			v110=v111=v112=v113=v114=v115=v116=v117=v118=v119=0;
		}
        virtual ~BigClass() {}

        typedef enum {
            v1, v2, v3, v4
        } valueslist;

        std::vector<uint8_t> data;
        valueslist vl;
        uint16_t number;

        std::vector<uint8_t> v000, v001, v002, v003, v004, v005, v006, v007, v008, v009;
        valueslist   v010, v011, v012, v013, v014, v015, v016, v017, v018, v019;
        std::string  v020, v021, v022, v023, v024, v025, v026, v027, v028, v029;
        double v030, v031, v032, v033, v034, v035, v036, v037, v038, v039;
        int    v040, v041, v042, v043, v044, v045, v046, v047, v048, v049;
        int    v050, v051, v052, v053, v054, v055, v056, v057, v058, v059;
        int    v060, v061, v062, v063, v064, v065, v066, v067, v068, v069;
        int    v070, v071, v072, v073, v074, v075, v076, v077, v078, v079;
        int    v080, v081, v082, v083, v084, v085, v086, v087, v088, v089;
        int    v090, v091, v092, v093, v094, v095, v096, v097, v098, v099;
        int    v100, v101, v102, v103, v104, v105, v106, v107, v108, v109;
        int    v110, v111, v112, v113, v114, v115, v116, v117, v118, v119;
 };

int test_BIG_CLASS()
{
	BigClass bg;
	return 0;									// breakpoint 1 BIG_CLASS
}

///////////////////////////////

namespace H
{
class A {
    public:
        int a;
        A () : a(1){};
};
class B : public A {
    public:
        int b;
        B () : b(2){};
};
class C : public B {
    public:
        int c;
        C () : c(3){};
};
class D : public C {
    public:
        int d;
        D () : d(4){};
        int test ()
        {
            H::D* d = this;						// breakpoint 1 LONG_INHERITANCE
            return d->a;
        };
};
}

int test_LONG_INHERITANCE ()
{
    H::D* d = new H::D();
    d->test();
    return 0;
}

///////////////////////////////

// execute a specific test sequence
// MUST HAVE A CORRESPONDING TEST SEQUENCE IN lldbmi2/test.cpp

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
	case 4:		return test_LARGE_CHAR_ARRAY ();
	case 5:		return test_LARGE_ARRAY ();
	case 6:		return test_POINTERS ();
	case 7:		return test_BASE ();		// ATTACH
	case 8:		return test_MEMBERS ();
	case 9:		return test_STRING ();
	case 10:	return test_ARGS ();
	case 11:	return test_CRASH ();
	case 12:	return test_INPUT ();
	case 13:	return test_CATCH_THROW ();
	case 14:	return test_BASE ();		// OTHER
    case 15:    return test_BIG_CLASS ();
    case 16:    return test_LONG_INHERITANCE ();
	}
	return 0;
}
