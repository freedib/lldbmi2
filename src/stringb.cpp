
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

// shift remove bytes first characters from buffer. capacity remains unchanged
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

// copy string at the start of the buffer
// if bytes specified, copy at most bytes bytes and terminate string
char *
StringB::copy (const char *string, int maxBytes) {
	return copyat (0, string, maxBytes);
}


// append string to the end of the buffer
// if bytes specified, append at most bytes bytes and terminate string
char *
StringB::append (const char *string, int extraBytes) {
	return copyat (buffer_size, string, BIG_LIMIT, extraBytes);
}

// append string to the end of the buffer
// if bytes specified, append at most bytes bytes and terminate string
char *
StringB::append (const char c) {
	if (buffer_size+1 >= buffer_capacity)
		if (grow (1) == NULL)
			return NULL;
	buffer_array[buffer_size++] = c;
	buffer_array[buffer_size] = '\0';
	return buffer_array;
}

// copy at offset of the buffer. usually 0 (copy) or buffer size (append)
// if bytes specified, copy at most bytes bytes and terminate string
char *
StringB::copyat (int offset, const char *string, int maxBytes, int extraBytes) {
	int string_length = strlen(string) + extraBytes;
	if (maxBytes<string_length)
		string_length = maxBytes;
	if (offset+string_length >= buffer_capacity)
		if (grow (string_length+1) == NULL)
			return NULL;
	if (maxBytes<=string_length)
		::strlcpy (buffer_array+offset, string, string_length+1);
	else
		::strcpy (buffer_array+offset, string);
	buffer_size = offset+string_length;
	return buffer_array;
}

// sprintf at start of the buffer
int
StringB::sprintf (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    return vosprintf (0, format, args);
}

// sprintf at the end of the buffer
int
StringB::catsprintf (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    return vosprintf (buffer_size, format, args);
}

// vsprintf at offset of the buffer. usually 0 (copy) or buffer size (append)
int
StringB::vosprintf (int offset, const char *format, va_list args)
{
	va_list args_start;
	va_copy(args_start, args);
    int string_length = vsnprintf (buffer_array+offset, buffer_capacity-offset, format, args);
    if (string_length > buffer_capacity-offset-1) {
    	if (grow (string_length)==NULL)			// no room
    		string_length = buffer_capacity-offset-1;
    	else {
    		string_length = vsnprintf (buffer_array+offset, buffer_capacity-offset, format, args_start);
    	}
    }
	buffer_size = offset+string_length;
    return string_length;
}
