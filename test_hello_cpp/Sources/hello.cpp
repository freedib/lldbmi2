#define BASE_TEST
//#define TEST_ALLOCATOR

#ifdef BASE_TEST

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

//#define TEST_MEMBERS
#define TEST_STRING
//#define TEST_CRASH
//#define TEST_ARGS

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
	ComplexClass cc;
}
#endif
#endif // BASE_TEST

#ifdef TEST_ALLOCATOR

#include <memory>
#include <iostream>
#include <string>

#include <string>
#include <iostream>
#include <cctype>

struct ci_char_traits : public std::char_traits<char> {
    static bool eq(char c1, char c2) {
         return std::toupper(c1) == std::toupper(c2);
     }
    static bool lt(char c1, char c2) {
         return std::toupper(c1) <  std::toupper(c2);
    }
    static int compare(const char* s1, const char* s2, size_t n) {
        while ( n-- != 0 ) {
            if ( std::toupper(*s1) < std::toupper(*s2) ) return -1;
            if ( std::toupper(*s1) > std::toupper(*s2) ) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const char* find(const char* s, int n, char a) {
        auto const ua (std::toupper(a));
        while ( n-- != 0 )
        {
            if (std::toupper(*s) == ua)
                return s;
            s++;
        }
        return nullptr;
    }
};

typedef std::basic_string<char, ci_char_traits> ci_string;

std::ostream& operator<<(std::ostream& os, const ci_string& str) {
    return os.write(str.data(), str.size());
}

void sub (std::allocator<int> &a1)
{

}

int main()
{
	ci_string s1 = "Hello";
	ci_string s2 = "heLLo";
	if (s1 == s2)
		std::cout << s1 << " and " << s2 << " are equal\n";

    std::allocator<int> a1; // default allocator for ints
    int* a = a1.allocate(10); // space for 10 ints

    sub (a1);

    a[9] = 7;

    std::cout << a[9] << '\n';

    a1.deallocate(a, 10);

    // default allocator for strings
    std::allocator<std::string> a2;

    // same, but obtained by rebinding from the type of a1
//    decltype(a1)::rebind<std::string>::other a2_1;

    // same, but obtained by rebinding from the type of a1 via allocator_traits
    std::allocator_traits<decltype(a1)>::rebind_alloc<std::string> a2_2;

    std::string* s = a2.allocate(2); // space for 2 strings

    a2.construct(s, "foo");
    a2.construct(s + 1, "bar");

    std::cout << s[0] << ' ' << s[1] << '\n';

    a2.destroy(s);
    a2.destroy(s + 1);
    a2.deallocate(s, 2);
}

#endif
