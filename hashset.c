#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn){
	assert(h != NULL && elemSize != 0 && numBuckets != 0 && hashfn != NULL && comparefn != NULL);
	h -> elems = malloc(numBuckets * sizeof(vector));
	h -> elem_size = elemSize;
	h -> num_buckets = numBuckets;
	h -> freefn = freefn;
	h -> comp = comparefn;
	h -> hash = hashfn;
	h -> log_len = 0;
	for (int i=0; i<numBuckets; ++i){
		VectorNew(&h -> elems[i], elemSize, freefn, 0);
	}
}

void HashSetDispose(hashset *h){
	assert(h != NULL);
	for (int i=0; i<h -> num_buckets; ++i){
		VectorDispose(&h -> elems[i]);
	}
	free(h -> elems);
}

int HashSetCount(const hashset *h){
	assert(h != NULL);
	return h -> log_len;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData){
	assert(h != NULL && mapfn != NULL);
	for (int i=0; i<h -> num_buckets; ++i){
		VectorMap(&h -> elems[i], mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr){
	assert(h != NULL && elemAddr != NULL);
	int cur_hash = h -> hash(elemAddr, h -> num_buckets);
	int pos = VectorSearch(&h -> elems[cur_hash], elemAddr, h -> comp, 0, 0);
	if (pos == -1){
		h -> log_len++;
		VectorAppend(&h -> elems[cur_hash], elemAddr);
	} else {
		VectorReplace(&h -> elems[cur_hash], elemAddr, pos);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr){
	assert(h != NULL && elemAddr != NULL);
	int cur_hash = h -> hash(elemAddr, h -> num_buckets);
	int pos = VectorSearch(&h -> elems[cur_hash], elemAddr, h -> comp, 0, 0);
	if (pos == -1){
		return NULL;
	} else {
		return VectorNth(&h -> elems[cur_hash], pos);
	}
}