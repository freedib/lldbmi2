
#ifndef BUFFER_H
#define BUFFER_H

/*
 * StringB buffer class
 * This class is a mix between std::string ans std::vector
 * with the addon of catsprintf
 */

#define BIG_LIMIT 100000		// protection

class StringB {
private:
	int buffer_capacity;
	int buffer_size;
	char *buffer_array;
public:
	StringB ();
	StringB (int max_size);
	virtual ~StringB ();
	char *grow (int at_least);
	int capacity ();
	int size ();
	char *c_str();
	char *clear (int bytes=BIG_LIMIT, int start=0);
	char *append (const char *string);
	int   catsprintf  (const char *format, ...);
	int   catvsprintf (const char *format, va_list args);
};

#endif // BUFFER_H
