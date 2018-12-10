#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

const int INITIAL_LEN = 10;

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
    assert(v != NULL);
    assert(initialAllocation >= 0 && elemSize > 0);
    if (initialAllocation == 0){
        v -> alloc_len = INITIAL_LEN;
    } else {
        v -> alloc_len = initialAllocation;
    }
    v -> elem_size =  elemSize;
    v -> log_len = 0;
    v -> elems = malloc(v -> alloc_len * v -> elem_size);
    v -> freefn = freeFn;
    assert(v -> elems != NULL);
}

void VectorDispose(vector *v){
    assert(v != NULL);
    if (v -> freefn != NULL){
        for (int i=0; i<v -> log_len; ++i){
            void * cur_elem = (char*) v -> elems + i * v -> elem_size;
            v -> freefn(cur_elem);
        }
    }
    free(v -> elems);
}

int VectorLength(const vector *v){
    assert(v != NULL);
    return v -> log_len;
}

void *VectorNth(const vector *v, int position){
    assert(v != NULL);
    assert(position < v -> log_len && position >= 0);
    void * cur_elem = (char *) v -> elems + position * v -> elem_size;
    return cur_elem;
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(v != NULL && elemAddr != NULL);
    assert(position >= 0 && position < v -> log_len);
    void * cur_elem = (char*) v -> elems + position * v -> elem_size;
    if (v -> freefn != NULL){
        v -> freefn(cur_elem);
    }
    memcpy(cur_elem, elemAddr, v -> elem_size);
}

/**
 * Expands memory of the vector when logical length of memory is
 * equal to allocated length of memory
*/

void expand(vector *v){
    v -> alloc_len <<= 1;
    assert(v -> alloc_len > 0);
    void * help = malloc(v -> alloc_len  * v -> elem_size);
    assert(help != NULL);
    memcpy(help, v -> elems, v -> log_len * v -> elem_size);
    free(v -> elems);
    v -> elems = help;
}

void VectorInsert(vector *v, const void *elemAddr, int position){
    assert(v != NULL && elemAddr != NULL);
    assert(position >= 0 && position <= v -> log_len);
    if (v -> log_len == v -> alloc_len){
        expand(v);
    }
    void* source = (char*) v -> elems + position * v -> elem_size;
    void* dest  = (char*) v -> elems + (position + 1) * v -> elem_size;
    int size = (v -> log_len - position) * v -> elem_size;
    memmove(dest, source, size);
    memcpy(source, elemAddr, v -> elem_size);
    v -> log_len++;
}

void VectorAppend(vector *v, const void *elemAddr){
    assert(v != NULL && elemAddr != NULL);
    if (v -> log_len == v -> alloc_len){
        expand(v);
    }
    void * dest = (char*) v -> elems + v -> log_len * v -> elem_size;
    memcpy(dest, elemAddr, v -> elem_size);
    v -> log_len++;
}

void VectorDelete(vector *v, int position){
    assert(v != NULL);
    assert(position >= 0 && position < v -> log_len);
    void * source = (char*) v -> elems + (position + 1) * v -> elem_size;
    void * dest = (char*) v -> elems + position * v -> elem_size;
    if (v -> freefn != NULL){
        v -> freefn(dest);
    }
    int size = (v -> log_len - position - 1) * v -> elem_size;
    memmove(dest, source, size);
    v -> log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    assert(v != NULL && compare != NULL);
    qsort(v -> elems, v -> log_len, v -> elem_size, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(v != NULL);
    assert(mapFn != NULL);
    for (int i=0; i<v -> log_len; ++i){
        void * cur_elem = (char*) v -> elems + i * v -> elem_size;
        mapFn(cur_elem, auxData);
    }
}

/**
 * Searches the key element in vector when it is sorted
 * and returns the pointer to finded element or NULL if it can't be found
*/

void* searchInSorted(const vector * v, const void* key, VectorCompareFunction searchFn, int startIndex){
    void * base = (char*) v -> elems + startIndex * v -> elem_size;
    void * ans = bsearch(key, base, v -> log_len - startIndex, v -> elem_size, searchFn);
    return ans;
}

/**
 * Searches the key element in vector when it is't sorted
 * and returns the pointer to finded element or NULL if it can't be found
*/

void* searchNotSorted(const vector * v, const void* key, VectorCompareFunction searchFn, int startIndex){
    void * base = (char*) v -> elems + startIndex * v -> elem_size;
    size_t size = v -> log_len - startIndex;
    void * ans = lfind(key, base, &size, v -> elem_size, searchFn);
    return ans;
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted){
    assert(v != NULL && key != NULL && searchFn != NULL);
    if (startIndex == 0 && v -> log_len == 0){
        return kNotFound;
    }
    assert(startIndex >= 0 && startIndex < v -> log_len);
    void * ans;
    if (isSorted){
        ans = searchInSorted(v, key, searchFn, startIndex);
    } else {
        ans = searchNotSorted(v, key, searchFn, startIndex);
    }
    if (ans == NULL){
        return kNotFound;
    }
    int pos = ((char*)ans - (char*)v -> elems) / v -> elem_size;
    return pos;
}