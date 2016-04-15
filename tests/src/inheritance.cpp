
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


int
main (int argc, char **argv)
{
    H::D* d = new H::D();
    d->test();

	return 0;
}
