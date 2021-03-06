#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "cutils/data_structures/dyn_array.h"
#include "cutils/math/constants.h"

// By one measure, the golden ratio is the optimal growth factor. It allows reuse of a memory block after only two
// reallocations, the theoretical minimum. Of course, whether memory blocks are reused in this way depends on the amount
// of contiguous memory available, and how memory allocation is implemented.
#define GROWTH_FACTOR CTLS_PHI
#define DEFAULT_INITIAL_CAPACITY 8

static bool reallocData(struct ctls_DynArray* dynArr, size_t newCapacity, size_t elemSize)
{
    void* newData = realloc(dynArr->data, newCapacity * elemSize);
    if (newData)
        dynArr->data = newData, dynArr->capacity = newCapacity;
    return newData;
}

struct ctls_DynArray* ctls_dyn_init(struct ctls_DynArray* dynArr, size_t initialCapacity, size_t elemSize)
{
    bool dynArrOriginallyNull = !dynArr;
    if (dynArrOriginallyNull)
        dynArr = malloc(sizeof(struct ctls_DynArray));
    if (dynArr)
    {
        void* newData = malloc(initialCapacity * elemSize);
        if (newData)
            *dynArr = (struct ctls_DynArray){newData, 0, initialCapacity};
        else
        {
            if (dynArrOriginallyNull)
                free(dynArr);
            dynArr = NULL;
        }
    }
    return dynArr;
}

struct ctls_DynArray* ctls_dyn_defaultInit(struct ctls_DynArray* dynArr, size_t elemSize)
{
    return ctls_dyn_init(dynArr, DEFAULT_INITIAL_CAPACITY, elemSize);
}

void ctls_dyn_reset(struct ctls_DynArray* dynArr, size_t elemSize)
{
    free(dynArr->data);
    memset(dynArr, 0, sizeof(struct ctls_DynArray));
}

bool ctls_dyn_shrinkToFit(struct ctls_DynArray* dynArr, size_t elemSize)
{
    return !dynArr->size || reallocData(dynArr, dynArr->size, elemSize);
}

struct ctls_DynArray* ctls_dyn_copy(struct ctls_DynArray* restrict dest, const struct ctls_DynArray* restrict src,
    size_t elemSize)
{
    bool destOriginallyNull = !dest;
    if (!dest || !dest->data)
    {
        dest = ctls_dyn_init(dest, src->capacity, elemSize);
        if (!dest)
            return NULL;
    }
    else if (!reallocData(dest, src->capacity, elemSize))
        return NULL;
    memcpy(dest->data, src->data, src->size * elemSize);
    dest->capacity = src->capacity, dest->size = src->size;
    return dest;
}

bool ctls_dyn_append(struct ctls_DynArray* restrict dynArr, const void* restrict elem, size_t elemSize)
{
    if (dynArr->size == dynArr->capacity)
    {
        size_t newCapacity = round(GROWTH_FACTOR * dynArr->capacity);
        if (newCapacity < dynArr->capacity || !reallocData(dynArr, newCapacity, elemSize))
            return false;
    }
    memcpy((char*)dynArr->data + elemSize * dynArr->size, elem, elemSize);
    ++dynArr->size;
    return true;
}

bool ctls_dyn_insert(struct ctls_DynArray* dynArr, const void* src, size_t pos, size_t srcLen, size_t elemSize)
{
    size_t n = dynArr->capacity;
    for (; n < dynArr->size + srcLen; n = round(GROWTH_FACTOR * n))
    {
        if (n < dynArr->capacity)
            return false;
    }
    if (!reallocData(dynArr, n, elemSize))
        return false;
    char* data = dynArr->data;
    size_t scaledPos = pos * elemSize, scaledSrcLen = srcLen * elemSize;
    memmove(data + scaledPos + scaledSrcLen, data + scaledPos, dynArr->size * elemSize - scaledPos);
    memmove(data + scaledPos, src, scaledSrcLen);
    dynArr->size += srcLen;
    return true;
}

bool ctls_dyn_extend(struct ctls_DynArray* dynArr, const void* src, size_t srcLen, size_t elemSize)
{
    return ctls_dyn_insert(dynArr, src, dynArr->size, srcLen, elemSize);
}

void ctls_dyn_remove(struct ctls_DynArray* dynArr, size_t from, size_t to, size_t elemSize)
{
    size_t scaledFrom = from * elemSize, scaledTo = to * elemSize;
    memmove((char*)dynArr->data + scaledFrom, (char*)dynArr->data + scaledTo, dynArr->size * elemSize - scaledTo);
    dynArr->size -= (to - from);
}
