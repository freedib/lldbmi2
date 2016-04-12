
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "Buffer.h"

// allocate a new Buffer
Buffer::Buffer () {
	buffer_capacity = 0;
	buffer_array = NULL;
	buffer_size = 0;
}

// allocate a new Buffer
Buffer::Buffer (int max_size): Buffer() {
	grow (max_size);
}

// delete Buffer
Buffer::~Buffer () {
	if (buffer_array != NULL)
		free (buffer_array);
}

// create or increase Buffer capacity
char *
Buffer::grow (int at_least) {
	buffer_capacity += ((at_least>buffer_capacity)? at_least: buffer_capacity);
	buffer_array = (char *) realloc (buffer_array, buffer_capacity);
	return buffer_array;
}

// return Buffer capacity
int
Buffer::capacity () {
	return buffer_capacity;
}

// return Buffer size
int
Buffer::size () {
	return buffer_size;
}

// return buffer array
char *
Buffer::data () {
	return buffer_array;
}

// clear buffer array content
char *
Buffer::clear () {
	buffer_size = 0;
	if (buffer_array != NULL)
		*buffer_array = '\0';
	return buffer_array;
}

// return Buffer size
char *
Buffer::append (const char *string) {
	int string_length = strlen (string);
	if (buffer_size+string_length >= buffer_capacity)
		if (grow (string_length+1) == NULL)
			return NULL;
	strcat (buffer_array+buffer_size, string);
	buffer_size += string_length;
	return buffer_array;
}

// shift buffer content from n characters. size remains unchanged
char *
Buffer::shift (int bytes) {
	if (buffer_array == NULL)
		return NULL;
	if (bytes>=buffer_size) {
		buffer_size = 0;
		*buffer_array = '\0';
	}
	else
		strcpy (buffer_array, buffer_array+bytes);
	return buffer_array;
}

// sprintf in the buffer
int
Buffer::sprintf (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    int length = vsnprintf (buffer_array+buffer_size, buffer_capacity-buffer_size, format, args);
    va_end(args);
    if (length > buffer_capacity-buffer_size-1) {
    	if (grow (length)==NULL)			// no room
    		length = buffer_capacity-buffer_size-1;
    	else {
    	    va_start (args, format);
    		length = vsnprintf (buffer_array+buffer_size, buffer_capacity-buffer_size, format, args);
    	    va_end(args);
    	}
    }
	buffer_size += length;
    return length;
}
