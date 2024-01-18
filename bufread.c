// Jackson Burns
// 10-1-2023
// Lab 4 "File Buffer Lab"
// Description: This code is meant to mimic the behavior of a file buffer reader with a ring buffer. Also using
// read and open system calls.
// Citations: Gabriel Johnson

#include "rbuf.h"
#include "bufread.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct RingBuffer 
{
    int at;
    int size;
    int capacity;
    int __reserved;
    char *buffer;
} RingBuffer;

typedef struct BufReader {
    RingBuffer *rb;
    int fill;
    int fd;
} BufReader;

// This function is meant open up the given file and also serve as a constructor for the BufReader object
BufReader *br_open(const char *path, int capacity, int fill_trigger)
{
	// Open given path, store file descriptror
	int fd = open(path, O_RDONLY, 0666);

	// If fd is negative then the open call failed
	if (fd < 0)
	{
		perror(path);
		return NULL;
	}

	// If we got here then the file is open
	BufReader *br;

	// Allocate memory for the bufreader
	if (!(br = (BufReader *)malloc(sizeof(*br))))
	{
		// out of memory
		return NULL;
	}

	// Allocate memory for the ring buffer in the bufreader
	if (!(br->rb = (RingBuffer *)malloc(sizeof(RingBuffer))))
	{		
		// out of memory
		return NULL;
	}

	// Assign br values
	br->fill = fill_trigger;
	br->fd = fd;

	// Initialize all to zero except capacity which will be the given capacity
	br->rb->at = 0;
	br->rb->size = 0;
	br->rb->capacity = capacity;
	br->rb->buffer = (char *)calloc(capacity, sizeof(char));

	// If we can allocate the ring buffer but just not the actual buffer within
	if (!br->rb->buffer)
	{
		free(br->rb);
		return NULL;
	}

	// Fill buffer to capacity
	read(fd, br->rb->buffer, capacity);
	br->rb->size = capacity;

	// Return bufreader
	return br;

}

// This is to close the file and free and allocated memory like a destructor
void br_close(BufReader *br)
{
	int fd = br->fd;

	rb_free(br->rb);
	free(br);

	close(fd);
}

// This function is used to fill the ring buffer if the size is ever less than or equal to the fill trigger
void br_fill(BufReader *br)
{
	// Variables so its easier to type them
	int fd = br->fd;
	int at = br->rb->at;
	int size = br->rb->size;
	int cap = br->rb->capacity;

	// Where the end of the buffer is located
	int buff_end = (at + size) % cap;

	// Number of bytes to read in
	int bytes = cap - size;

	// If memory is contiguous then only one read call is needed but if it isn't
	// then we need two, one for the end of the buffer and one for the beginning
	if (buff_end + bytes < cap)
	{
		read(fd, br->rb->buffer + buff_end, bytes);
	}
	else
	{
		read(fd, br->rb->buffer + buff_end, cap - buff_end);
		read(fd, br->rb->buffer, bytes - cap + buff_end);
	}

	// Since it is filled the size is now the capacity
	br->rb->size = cap;
}

// This function is so that we can manipulate the internal file pointer while also handling the
// data in the buffer
void br_seek(BufReader *br, int offset, int whence)
{
	// If we're seeking from the beginning
	if (whence == SEEK_SET)
	{
		// We only need to seek, clear the buffer since it will be earlier than
		// where ever we originally were and then fill
		lseek(br->fd, offset, SEEK_SET);
		rb_clear(br->rb);
		br_fill(br);
	} 
	// If we're seeking from the end
	else if (whence == SEEK_END) 
	{
		// Get the current position and the new position, then get the difference
		int curr = lseek(br->fd, 0, SEEK_CUR);
		int new_pos = lseek(br->fd, offset, SEEK_END);
		int ignore = new_pos - curr;

		// If the difference is less than the capacity of our buffer then we need to ignore the amount of bytes
		// and then if this makes our size less than the fill trigger we need to fill
		if (ignore <= br->rb->capacity)
		{
			rb_ignore(br->rb, ignore);
			if (br->rb->size <= br->fill) br_fill(br);
		} 
		// Otherwise, just go ahead and clear and refill since we'll be passed the buffer anyway
		else
		{
			rb_clear(br->rb);
			br_fill(br);
		}
	} 
	// If we're seeking from the current location then we'll need to see if offset is positive or negative
	else if (whence == SEEK_CUR)
	{
		int curr = 0, new_pos = 0, ignore = 0;

		// If offset is positive
		if (offset >= 0)
		{
			// Get how far we are from where we were
			curr = lseek(br->fd, 0, SEEK_CUR);
			new_pos = lseek(br->fd, offset, SEEK_CUR);
			ignore = new_pos - curr;

			// If its less than our capacity then we need to ignore and fill if the new size hits the fill trigger
			if (ignore <= br->rb->capacity)
			{
				rb_ignore(br->rb, ignore);
				if(br->rb->size <= br->fill) br_fill(br);
			}
			else
			{
				rb_clear(br->rb);
				br_fill(br);
			}
		} 
		// If the offset is negative then we need to flush and fill our buffer
		else
		{
			lseek(br->fd, offset, SEEK_CUR);
			rb_clear(br->rb);
			br_fill;
		}
	} 
	// If we got here then the user inputted an invalid mode
	else 
	{
		printf("Invalid mode please try either SEEK_SET, SEEK_END, SEEK_CUR\n");
	}
	
}

// Gives us where we are in the file
int br_tell(BufReader *br)
{
	int pos = 0;

	pos = lseek(br->fd, 0, SEEK_CUR);

	// If pos = -1 then it failed which means we need to return 0
	if (pos == -1) return 0;

	// Returning pos - size since pos is equal to where the end of the buffer is rather than where we are in the file
	return pos - br->rb->size;
}

// This function is used to get the next char in the buffer and cast it as an integer
int br_getchar(BufReader *br)
{
	char c;
	int val = -1;

	// Check if the buffer even has anything in it
	if (br->rb->size > 0)
	{
		rb_pop(br->rb, &c);
		val = (int)c;
		
		// Check for fill trigger
		if (br->rb->size <= br->fill)
		{
			br_fill(br);
		}

		return val;
	}

	// If we get here then size is < 0 which means there's nothing in the buffer to read
	return val;
}

// This function is to get a whole line from the file
char *br_getline(char s[], int size, BufReader *br)
{
	int i = 0;

	// Loop until either a newline, eof, or until we reach our given size limit
	while (1)
	{
		int val = br_getchar(br);
		if (val == 10 || val == 26) break;
		s[i] = val;
		i++;
		if (i >= size) break;
	}

	// Replace the end with a null character
	s[size-1] = '\0';

	return s;
}

// This function is meant to read the given number of bytes into the given destination
int br_read(BufReader *br, char *dest, int max_bytes)
{
	// If max_bytes is negative then its invalid and if the size is <= 0 then there's nothing to read
	// so return 0
	if (max_bytes < 0 || br->rb->size <= 0) return 0;

	int bytes = 0;

	// First read from the buffer using rb_read
	bytes = rb_read(br->rb, dest, max_bytes);

	// If the read bytes is less than max_bytes that means we need to read directly from the file now
	if (bytes < max_bytes)
	{
		bytes += read(br->fd, dest + bytes, max_bytes - bytes);
	}

	// Fill trigger check
	if (br->rb->size <= br->fill) br_fill(br);

	return bytes;
}
