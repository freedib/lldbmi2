// test program for LLDBMI2

//#define SIMPLE
#define COMPLEX

#ifdef COMPLEX

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

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
	int m[1];
	startthread ();
	y.m=11; y.n=22; y.o=33L;
	int c=12;
	z.y = &y;
	z.a = 11;
	z.b = b;
	z.k[1][1] = 4;
	z.l[1]="string 2";
	m[0] = 123;
	++py->o;
	c = sub (67, b, &z);
	printf ("c=%d\n", c);
	fflush (stdout);
	waitthread ();
	return 0;
}

#endif

#ifdef SIMPLE

#include <stdio.h>

int main ()
{
	int c[1];
	c[0] = 0;
	c[0] = 1;
//	int a = 13;
//	int *b = &a;
//	++*b;
//	int c[1];
//	c[0] = *b;
	return 0;
}

#endif
