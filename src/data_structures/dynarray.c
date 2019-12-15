#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fenv.h>

#include "data_structures/dynarray.h"
#include "math/constants.h"

#define GROWTH_FACTOR PHI
#define DEFAULT_INITIAL_CAPACITY 8

static bool reallocData(struct DynArray* dynArr, size_t newCapacity, size_t elemSize)
{
    void* newData = realloc(dynArr->data, newCapacity * elemSize);
    if (newData)
        dynArr->data = newData, dynArr->capacity = newCapacity;
    return newData;
}

static bool expand(struct DynArray* dynArr, size_t elemSize)
{
    // TODO: Handle possible failure of `feholdexcept` and `fesetenv`.
    fenv_t fenv;
    bool fenvSaved = !feholdexcept(&fenv);
    bool succeeded = reallocData(dynArr, GROWTH_FACTOR * dynArr->capacity + 0.5, elemSize);
    if (fenvSaved)
        fesetenv(&fenv);
    return succeeded;
}

struct DynArray* dyn_init(struct DynArray* dynArr, size_t elemSize)
{
    if (!dynArr)
        dynArr = malloc(sizeof(struct DynArray));
    dynArr->data = malloc(DEFAULT_INITIAL_CAPACITY * elemSize);
    dynArr->size = 0, dynArr->capacity = DEFAULT_INITIAL_CAPACITY;
    return dynArr;
}

void dyn_reset(struct DynArray* dynArr, size_t elemSize)
{
    free(dynArr->data);
    dynArr->size = 0, dynArr->capacity = 0;
}

bool dyn_shrinkToFit(struct DynArray* dynArr, size_t elemSize)
{
    return reallocData(dynArr, dynArr->size, elemSize);
}

bool dyn_append(struct DynArray* restrict dynArr, const void* restrict elem, size_t elemSize)
{
    if (dynArr->size < dynArr->capacity || expand(dynArr, elemSize))
    {
        memcpy((char*)dynArr->data + elemSize * dynArr->size, elem, elemSize);
        ++dynArr->size;
        return true;
    }
    else
        return false;
}

bool dyn_insert(struct DynArray* dynArr, const void* src, size_t pos, size_t srcLen, size_t elemSize)
{
    while (dynArr->capacity < dynArr->len + srcLen)
    {
        if (!expand(dynArr, elemSize))
            return false;
    }
    char* data = dynArr->data;
    size_t scaledPos = pos * elemSize, scaledSrcLen = srcLen * elemSize;
    memmove(data + scaledPos + scaledSrcLen, data + scaledPos, dynArr->size * elemSize - scaledPos);
    memmove(data + scaledPos, src, scaledSrcLen);
    dynArr->size += srcLen;
    return true;
}

bool dyn_extend(struct DynArray* dynArr, const void* src, size_t srcLen, size_t elemSize)
{
    return dyn_insert(dynArr, src, dynArr->size, srcLen, elemSize);
}

void dyn_remove(struct DynArray* dynArr, size_t from, size_t to, size_t elemSize)
{
    size_t scaledFrom = from * elemSize, scaledTo = to * elemSize;
    memmove((char*)dynArr->data + scaledFrom, (char*)dynArr->data + scaledTo, scaledTo - scaledFrom);
    dynArr->size -= (to - from);
}