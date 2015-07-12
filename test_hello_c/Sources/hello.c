// test program for LLDBMI2
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

//#define TEST_ALL
#define TEST_ARRAYS
//#define TEST_POINTERS

#ifdef TEST_ALL

typedef struct {
	short m;
	long n;
	long long o;
} Y;
typedef struct {
	int a;
	char *b;
	Y *y;
	int k[2][3];
	char *l[4];
} Z;

Y y;
pthread_t tid;

void *
hellothread (void *arg)
{
	for (int loop=0; loop<20; loop++) {
		sleep (10);
		printf ("loop %d\n", loop);
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

int sub(int a, char * b, Z *z) {
	int d = 33;
	printf ("hello, world: a=%d, b=%s\n", z->a, b);
	fflush (stdout);
	return a+d;
}

int main (int argc, char **argv, char **envp)
{
	printf ("PID=%d\n", getpid());
	Z z;
	char *b="22";
	Y* py = &y;
	float m[1];
	startthread ();
	y.m=11; y.n=22; y.o=33L;
	int c=12;
	z.y = &y;
	z.a = 11;
	z.b = b;
	z.k[1][1] = 4;
	z.l[1]="string 2";
	m[0] = 123.456;
	++py->o;
	c = sub (67, b, &z);
	printf ("c=%d\n", c);
	fflush (stdout);
	waitthread ();
	return 0;
}

#endif

#ifdef TEST_ARRAYS

#include <stdio.h>
typedef struct {
	double d;
	const char *c;
} S;
int main ()
{
	char c[101];
	int i[200];
	S s[200];
	c[0] = 'H';
	i[100] = 1001;
	s[0].c = "hello";
	return 0;
}

#endif

#ifdef TEST_POINTERS

#include <stdio.h>

typedef struct {
	int a;
	char *b;
} X;
X x;

int main ()
{
	X *px = &x;
	x.a = 13;
	px->a = 26;
	x.b = "hello";
	px->b = "hellohello";
	return 0;
}

#endif
