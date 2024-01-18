// Jackson Burns
// 9-19-2023
// Lab 3 "Ring Buffer"
// Description: This code is meant to mimic the behavior of a ring buffer which is a circular data structure
// that is used with the assistance of reading in data in chunks. This code allows the user to push new data into
// the buffer as long as the buffer has capacity for it, the user can pop data and either store it or ignore it, 
// the user can also peek at the next byte to be read or popped, the user can also ignore a number of bytes. The
// user can also read from the buffer and write to the buffer in larger chunks

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "rbuf.h"
#include <string.h>

// Opaque structure definition, reserved is included for padding
// if it was left out the padding would still happen automatically
typedef struct RingBuffer
{
    int at;
    int size;
    int capacity;
    int __reserved;
    char *buffer;
} RingBuffer;

// Memory Management

// Ring Buffer "constructor" to create a new Ring Buffer with a given capacity
RingBuffer *rb_new(int capacity)
{
    RingBuffer *rb;
    if (capacity <= 0) {
        // Capacity is invalid.
        return NULL;
    }
    if (!(rb = (RingBuffer *)malloc(sizeof(*rb)))) {
        // Out of memory.
        return NULL;
    }
    // If we get here, all is well with the memory.
    rb->at = 0;
    rb->size = 0;
    rb->capacity = capacity;
    rb->buffer = (char *)calloc(rb->capacity, sizeof(char));
    if (!rb->buffer) {
        // We were able to create the structure in memory,
        // but not the buffer. We need both, so free the
        // structure and return NULL (error).
        free(rb);
        return NULL;
    }
    return rb;
}

// Free the memory allocated for the buffer
void rb_free(RingBuffer *rb)
{
    free(rb->buffer);
    free(rb);
}

// Ring Buffer Values

// Each of these are so that we can test the values of the Ring Buffer with our main.c file
// since its an opaque structure we need these functions to access the values. 
int rb_at(const RingBuffer *rb)
{
    return rb->at;
}

int rb_size(const RingBuffer *rb)
{
    return rb->size;
}

int rb_capacity(const RingBuffer *rb)
{
    return rb->capacity;
}

// This one was not provided for us I only added it so that I could see what was in the buffer
// during testing
char *rb_buffer(const RingBuffer *rb)
{
	return rb->buffer;
}

// Individual Item Functions

// This function sets the index to at+size%capacity to behave circularly and
// if a value doesn't exist at the index then it puts the given data into that
// spot and returns true. If there was no room then it returns false.
bool rb_push(RingBuffer *rb, char data)
{	
	int index = (rb->at + rb->size) % rb->capacity;

	if (!rb->buffer[index])
	{
		rb->buffer[index] = data;
		rb->size = rb->size + 1;
		return true;
	}

	return false;
}

// First checks if there is anything stored in the current byte at the at index
// if there is something stored then it checks if the given data pointer is NULL
// if it is not NULL then we have the memory location of data become the value at
// the current index. Regardless of the data being stored we will decrease the size
// by one and increase the at by one and return true. If there wasn't even a value
// to be popped then we return false
bool rb_pop(RingBuffer *rb, char *data)
{
	if (rb->buffer[rb->at])
	{
		if (data != NULL)
		{
			*data = rb->buffer[rb->at];
		}
		rb->size = rb->size - 1;
		rb->at = rb->at + 1;
		return true;
	}
	return false;
}

// This makes sure that the size is greater than 0 and makes sure there is a value
// at what the current at index is and if so then it returns the value but if either
// condition is false then we return 0
char rb_peek(const RingBuffer *rb)
{
	if ((rb->size > 0) && (rb->buffer[rb->at]))
	{
		return rb->buffer[rb->at];
	}
	return 0;
}

// This checks if the given num is smaller than the current size, if it is not then
// it increases at by num and decreases size by num. If the size was smaller than num
// then it uses size instead
void rb_ignore(RingBuffer *rb, int num)
{
	if (num <= rb->size)
	{
		rb->at = rb->at + num;
		rb->size = rb->size - num;
	} else 
	{
		rb->at = rb->at + rb->size;
		rb->size = rb->size - rb->size;
	}	
}

// Buffer Functions

// This funciton is meant to copy data up to the max_bytes argument from the buffer into the 
// given buffer "buf". It first creates a new pointer called "src" and increases it by the at
// value so that it is now pointing to where the ring buffer is at. Then it runs memcpy. Then
// we calculate how many values were actually copied by getting the size of the buffer and dividing
// it by the size of one of the values in the buffer. Then we use that value to create a for loop
// that increments "num_el" for every value that exists in the buffer. It then sets at and size to 0
//
// I could not for the life of me get this function to behave as intended. It keeps returning one 
// extra byte when it shouldn't. I left the ideas that I tried commented out.
int rb_read(RingBuffer *rb, char *buf, int max_bytes)
{
	char *src = rb->buffer;
	src += rb->at;

	//if (max_bytes > rb->size)
	//{
	//	max_bytes = rb->size;
	//}
	
	memcpy(buf, src, max_bytes);

	int num_cpd = sizeof(buf) / sizeof(buf[0]);
	int num_el = 0;
	
	for (int i = 0; i < num_cpd; i++)
	{
		if (buf[i])
		{
			num_el++;
		}
	}
	//rb->at = (rb->at + num_el) % rb->capacity;
	//rb->size = rb->size - num_el;
	rb->at = 0;
	rb->size = 0;

	return num_el;
}

// This function writes from the given "buf" to the ring buffer. The number of bytes written
// is either the given number or if the given number was more than the amount of capacity left over
// then it is the difference between capacity and size. It then calls memcpy and uses the same method
// from above to determine how many bytes were actually written. It then sets size to the number
// of elements now inside the buffer
int rb_write(RingBuffer *rb, const char *buf, int bytes)
{
	char *dest = rb->buffer;
	
	if (bytes > rb->capacity - rb->size)
	{
		bytes = rb->capacity - rb->size;
	}

	memcpy(dest, buf, bytes);

	int num_cpd = sizeof(rb->buffer) / sizeof(rb->buffer[0]);
	int num_el = 0;

	for (int i = 0; i < num_cpd; i++)
	{
		if (rb->buffer[i])
		{
			num_el++;
		}
	}

	rb->size = num_el;

	return num_el;
}

// Clears the Ring Buffer
void rb_clear(RingBuffer *rb)
{
    rb->at = 0;
    rb->size = 0;
}
