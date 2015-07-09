
// test program for LLDBMI2

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
Z z;

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

int main (int argc, char **argv, char **envp) {
//	printf ("arg=%d, *envp=%s\n", argc, *envp);
//	for (int a=0; a<argc; a++)
//		printf ("argv[%d]=%s\n", a, argv[a]);
//	printf ("\n");
	printf ("PID=%d\n", getpid());
	y.m=11; y.n=22; y.o=33L;
	Z *pz = &z;
	startthread ();
	int c=12;
	z.y = &y;
	char *b="22";
	pz->b = b;
	pz->a = 11;
	pz->k[1][1] = 4;
	z.l[1]="string 2";
	c = sub (67, b, &z);
	printf ("c=%d\n", c);
	fflush (stdout);
	waitthread ();
	return 0;
}
