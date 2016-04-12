
#ifndef BUFFER_H
#define BUFFER_H

class Buffer {
private:
	int buffer_capacity;
	int buffer_size;
	char *buffer_array;
public:
	Buffer ();
	Buffer (int max_size);
	virtual ~Buffer ();
	char *grow (int at_least);
	int capacity ();
	int size ();
	char *data();
	char *clear ();
	char *append (const char *string);
	char *shift (int bytes);
	int   sprintf (const char *format, ...);
};

#endif // BUFFER_H
