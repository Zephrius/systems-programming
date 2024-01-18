// Jackson Burns
// 9-12-2023
// Lab 2 "Vector Lab"
// Description: this program is meant to mimic the function of the vector library in C++ using C code

#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

// Opaque vector structure
typedef struct Vector {
    int size;
    int capacity;
    int64_t *values;
} Vector;

// Allocation/Creation of a vector

// Allocates enough memory for our vector struct but with a capacity of 0
Vector *vector_new(void)
{
	Vector *v = (Vector *)malloc(sizeof(Vector));
	v->size = 0;
	v->capacity = 0;
	v->values = NULL;

	return v;
}

// Allocates enough memory for our vector struct with a capacity and initialized all values to 0
Vector *vector_new_with_capacity(int capacity)
{
	Vector *v = (Vector *)malloc(sizeof(Vector));
	v->size = 0;
	v->capacity = capacity;
	v->values = (int64_t *)calloc(v->capacity, sizeof(int64_t));

	// Error check in case it fails to allocate any memory for values
	if (v->values == NULL)
	{
		printf("Failure to allocate memory for values\n");
	}

	return v;
}

// Frees our struct
void vector_free(Vector *vec)
{
	free(vec->values);
	free(vec);
}

// Resizes the vector, if the given size is bigger than the current capacity then it reallocates the 
// current vector's values array by making a new one of the desired size and copying over the values 
// and then freeing the old one before assigning the new one the same pointer so that it points
// to old memory location
void vector_resize(Vector *vec, int new_size)
{
	if (new_size > vec->capacity)
	{
		vec->capacity = new_size;

		int64_t *temp = (int64_t *)calloc(vec->capacity, sizeof(int64_t));

		for (int64_t i = 0; i < vec->size; i++)
		{
			temp[i] = vec->values[i];
		}

		free(vec->values);
		vec->values = temp;
		vec->size = new_size;

	} else {
		vec->size = new_size;
	}
}

// Works similarly to resize except it is directly changing the capacity without changing the size
void vector_reserve(Vector *vec, int new_capacity)
{
	if (new_capacity > vec->size || new_capacity > vec->capacity)
	{
		vec->capacity = new_capacity;

		int64_t *temp = (int64_t *)calloc(vec->capacity, sizeof(int64_t));

		for (int64_t i = 0; i < vec->size; i++)
		{
			temp[i] = vec->values[i];
		}

		free(vec->values);
		vec->values = temp;

	} else {
		vec->capacity = vec->size;
	}
}

// Element Functions

// Resizes the vector to just the same size but plus one and then appends the given value to the end 
// of the vector
void vector_push(Vector *vec, int64_t value)
{
	vector_resize(vec, vec->size + 1);
	vec->values[vec->size-1] = value;
}

// Resizes the vector and shifts all values to the right of the given index over one place to the right 
// including the value at the index and then it assigns the given value to the given index
void vector_insert(Vector *vec, int index, int64_t value)
{
	if (index >= vec->size - 1)
	{
		vector_resize(vec, vec->size + 1);

		for (int i = vec->size - 2; i >= index; i--)
		{
			vec->values[i+1] = vec->values[i];
		}
		vec->values[index] = value;
	} else
	{
		vector_push(vec, value);
	}
	
}

// Removes the value at the given index by shifting over all values to the right of the index over one place to 
// the left so that it overrides the value at the given index and then it decrements the size by 1. This does not
// require a resize as it will still be within the capacity
bool vector_remove(Vector *vec, int index)
{
	if (index <= vec->size)
	{
		for (int i = index; i < vec->size - 1; i++)
		{
			vec->values[i] = vec->values[i+1];
		}
		vec->size = vec->size - 1;
		return true;
	}
	return false;
}

// Returns true if there is a value at the given index and then sets the given value pointer
// to the value at said index
bool vector_get(Vector *vec, int index, int64_t *value)
{
	if (index <= vec->size)
	{
		if (value)
		{
			*value = vec->values[index];
		}
		return true;
	} 

	return false;
}

// Returns true if there is a value at the given index and then replaces that value
// with the given value
bool vector_set(Vector *vec, int index, int64_t value)
{
	if (index <= vec->size)
	{
		vec->values[index] = value;
		return true;
	}
	return false;
}

// Clears the vector by setting size to 0
void vector_clear(Vector *vec)
{
	vec->size = 0;
}

// Finding/Sorting

// Uses a linear search to find the index of the first instance of the given value
int vector_find(Vector *vec, int64_t value)
{
	for (int i = 0; i < vec->size; i++)
	{
		if (vec->values[i] == value)
		{
			return i;
		}
	}
}

// Comparison function
static bool comp_ascending(int64_t left, int64_t right)
{
    return left <= right;
}

// Sort wrapping function
void vector_sort(Vector *vec)
{
    vector_sort_by(vec, comp_ascending);
}

// Sorting function so that we may sort our vector to test bsearch
void vector_sort_by(Vector *vec, bool (*comp)(int64_t left, int64_t right))
{
    int i;
    int j;
    int min;
    int64_t tmp;

    for (i = 0; i < vec->size - 1; i++) 
	{
        min = i;
        for (j = i + 1; j < vec->size; j++) 
		{
            if (!comp(vec->values[min], vec->values[j])) 
			{
                min = j;
            }
        }
        if (min != i) 
		{
            tmp = vec->values[min];
            vec->values[min] = vec->values[i];
            vec->values[i]   = tmp;
        }
    }
}

// Binary search that works by comparing the given value with the value at the middle index,
// if the given value is equal to the value at the middle index then its job is finished and it 
// will return the middle index. If the value is greater than the middle value then we ignore the
// left half and if it is less than then we ignore the right half. This continues until it finds 
// the desired value. If the desired value is not found then this function will return -1
int vector_bsearch(Vector *vec, int64_t value)
{
	int left = 0; 
	int right = vec->size - 1;

	while (left <= right)
	{
		int mid = left + (right - 1) / 2;

		if (vec->values[mid] == value)
		{
			return mid;
		}

		if (vec->values[mid] < value)
		{
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return -1;
}

// Accessors for Fields in the Vector

// Since vector is an opaque structure we need accessors for size and capacity
int vector_capacity(Vector *vec)
{
	return vec->capacity;
}

int vector_size(Vector *vec)
{
	return vec->size;
}
