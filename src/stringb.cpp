
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stringb.h"


// allocate a new StringB
StringB::StringB () {
	buffer_capacity = 0;
	buffer_array = NULL;
	buffer_size = 0;
}

// allocate a new StringB
StringB::StringB (int max_size): StringB() {
	grow (max_size);
}

// delete StringB
StringB::~StringB () {
	if (buffer_array != NULL)
		free (buffer_array);
}

// create or increase StringB capacity
char *
StringB::grow (int at_least) {
	buffer_capacity += ((at_least>buffer_capacity)? at_least: buffer_capacity);
	if (buffer_capacity>BIG_LIMIT)
		buffer_capacity = BIG_LIMIT;
	buffer_array = (char *) realloc (buffer_array, buffer_capacity);
	return buffer_array;
}

// return StringB capacity
int
StringB::capacity () {
	return buffer_capacity;
}

// return StringB size
int
StringB::size () {
	return buffer_size;
}

// return buffer array
char *
StringB::c_str () {
	return buffer_array;
}

// shift remove n first characters from buffer. capacity remains unchanged
char *
StringB::clear (int bytes, int start) {
	if (buffer_array == NULL)
		return NULL;
	if (start+bytes>=buffer_size) {
		*(buffer_array+start) = '\0';
		buffer_size = start;
	}
	else {
		::strcpy (buffer_array+start, buffer_array+start+bytes);
		buffer_size -= bytes;
	}
	return buffer_array;
}


// append string to the end of the buffer
char *
StringB::append (const char *string) {
	int string_length = strlen (string);
	if (buffer_size+string_length >= buffer_capacity)
		if (grow (string_length+1) == NULL)
			return NULL;
	::strcat (buffer_array+buffer_size, string);
	buffer_size += string_length;
	return buffer_array;
}

// sprintf at the end of the buffer
int
StringB::catsprintf (const char *format, ...)
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

// sprintf at the end of the buffer
int
StringB::catvsprintf (const char *format, va_list args)
{
	va_list args_back;
	va_copy(args_back, args);
    int length = vsnprintf (buffer_array+buffer_size, buffer_capacity-buffer_size, format, args);
    if (length > buffer_capacity-buffer_size-1) {
    	if (grow (length)==NULL)			// no room
    		length = buffer_capacity-buffer_size-1;
    	else {
    		length = vsnprintf (buffer_array+buffer_size, buffer_capacity-buffer_size, format, args_back);
    	}
    }
	buffer_size += length;
    return length;
}
