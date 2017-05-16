//
// This is the implementation of a generic symbol table. A table stores
// (symbol, data) pairs.
//
// Debajyoti Dash


#include <stdlib.h>
#include <string.h>
#include "symtab.h"

#define DEBUG 0

struct entry_s {
	char *key;
	void *value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
	int size;
	struct entry_s **table;	
};

typedef struct hashtable_s hashtable_t;

struct hash_elem_it_s {
	struct hashtable_s *ht; 
	unsigned int index;		
	struct entry_s *elem; 	
};

typedef struct hash_elem_it_s hash_elem_it_t;

static unsigned int hash(hashtable_t *hashtable, const char *str)
{
  	const unsigned int p = 16777619;
  	unsigned int hash = 2166136261u;
  	while (*str)
  	{
    	hash = (hash ^ *str) * p;
    	str += 1;
  	}
  	hash += hash << 13;
  	hash ^= hash >> 7;
  	hash += hash << 3;
  	hash ^= hash >> 17;
  	hash += hash << 5;
  	return hash % hashtable->size;
  	
}

static int *lookup ( hashtable_t *hashtable, const char *key ) {
	int bin = hash(hashtable, key);
	entry_t *pair;
#if DEBUG
	printf("HashValue: [%i] Key: [%s]\n", bin, key);
#endif
	pair = hashtable->table[bin];

	if( pair == NULL || pair->key == NULL || strcmp( key, pair->key ) != 0 ) {
#if DEBUG
		printf("Lookup Key->Data is NULL\n");
#endif
		return NULL;

	} else {
#if DEBUG
		printf("Lookup Key->Data: [%i]\n", pair->value);
#endif
		return pair->value;
	}
	
}


void *symtabCreate(int sizeHint)
{
 	hashtable_t *hashtable = NULL;
	int i;

	if( sizeHint < 1 ) return NULL;

	if( ( hashtable = malloc( sizeof( hashtable_t ) ) ) == NULL ) {
#if DEBUG
		printf("Test _err01\n");
#endif
		return NULL;
	}

	if( ( hashtable->table = malloc( sizeof( entry_t * ) * sizeHint ) ) == NULL ) {
#if DEBUG
		printf("Test _err02\n");
#endif
		return NULL;
	}
	for( i = 0; i < sizeHint; i++ ) {
		hashtable->table[i] = NULL;
	}

	hashtable->size = sizeHint;
	return hashtable;
}

// Deletes a symbol table.
// Reclaims all memory used by the table.
// Note that the memory associate with data items is not reclaimed since
//   the symbol table does not know the actual type of the data. It only
//   manipulates pointers to the data.
// Also note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void symtabDelete(void *symtabHandle)
{
	hashtable_t *hashtable = symtabHandle;
  	entry_t *iterate = NULL;

	for (int i=0; i<=hashtable->size;i++ )
  	{
  		iterate = hashtable->table[i];

  		if(iterate->key != NULL){
	      	free(iterate->key);
		}
  	}
  	free(iterate);
  	
}

// Install a (symbol, data) pair in the table.
// If the symbol is already installed in the table, then the data is
//   overwritten.
// If the symbol is not already installed, then space is allocated and
//   a copy is made of the symbol, and the (symbol, data) pair is then
//   installed in the table.
// If successful, returns 1.
// If memory cannot be allocated for a new symbol, then returns 0.
// Note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
int symtabInstall(void *symtabHandle, const char *symbol, void *data)
{
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;

	hashtable_t *hashtable = symtabHandle;
	bin = hash (hashtable, symbol);
	next = hashtable->table[bin];

#if DEBUG
	printf("symtabInstall function called\n");
#endif

	while( next != NULL && next->key != NULL && strcmp( symbol, next->key ) > 0 ) {
		last = next;
		next = next->next;
	}


	/* There's already a pair.  Let's replace that string. */
	if( next != NULL && next->key != NULL && strcmp( symbol, next->key ) == 0 ) {
#if DEBUG
		//printf("Test 21 Update Value: [%s] and [%i]\n", next->key, next->value );
		//printf("Test 21 Update to Value: [%s] and [%i]\n", symbol, data );
#endif
		next->value = data;
#if DEBUG
		printf("Updated Key->Value: [%s] and [%i]\n", next->key, next->value );
#endif

	/* Nope, could't find it.  Time to grow a pair. */
	}
	else {
#if DEBUG
		printf("NEW ENTRY to HASH TABLE\n");
#endif

		if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
#if DEBUG
			printf("Test _err03\n");
#endif
			return 0;
		}

		newpair->next = NULL;
		newpair->value = data;
		newpair->key = malloc(strlen(symbol) + 1);
        	strcpy(newpair->key, symbol);

		if( next == hashtable->table[ bin ] ) {
			newpair->next = next;
			hashtable->table[ bin ] = newpair;
	
		/* We're at the end of the linked list in this bin. */
		} else if ( next == NULL ) {
			last->next = newpair;
	
		/* We're in the middle of the list. */
		} else  {
			newpair->next = next;
			last->next = newpair;
		}

#if DEBUG	
		printf("New Entry Key->Value: [%s] and [%i]\n", newpair->key, newpair->value );
#endif
	}

	return 1;
}


void *symtabLookup(void *symtabHandle, const char *symbol)
{
#if DEBUG
	printf("symtabLookup function called\n");
#endif
	int data;
	data = lookup (symtabHandle, symbol);
	if (data != NULL) {
#if DEBUG
		printf("Lookup Key->Data: [%i]\n", data);
#endif
		return data;
	}
#if DEBUG
	printf("Lookup Key->Data is NULL\n");
#endif
	return NULL;
}

// Create an iterator for the contents of the symbol table.
// If successful, a handle to the iterator is returned which can be
// repeatedly passed to symtabNext to iterate over the contents
// of the table.
// If memory cannot be allocated for the iterator, returns NULL.
// Note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
// hash_elem_it_s
void *symtabCreateIterator(void *symtabHandle)
{
	hashtable_t *hashtable = symtabHandle;
	hash_elem_it_t *iterator = NULL;
	if( ( iterator = malloc( sizeof( hash_elem_it_t ) ) ) == NULL ) {
#if DEBUG
		printf("Test _err05\n");
#endif
		return NULL;
	}

	iterator->ht = hashtable;
	iterator->index = 0;
	iterator->elem = iterator->ht->table[iterator->index];
	do{
		
		if(iterator->elem != NULL && iterator->elem->key != NULL){
			break;
		}else{
			iterator->index++;
			iterator->elem = iterator->ht->table[iterator->index];
		}
	}while(iterator->index < (iterator->ht->size - 1));

	entry_t *elem = malloc( sizeof( entry_t ) );
	elem->next = iterator->elem;
	iterator->elem = elem;

  	return iterator;
}

// Returns the next (symbol, data) pair for the iterator.
// The symbol is returned as the return value and the data item
// is placed in the location indicated by the second parameter.
// If the whole table has already been traversed then NULL is
//   returned and the location indicated by the second paramter
//   is not modified.
// Note that no validation is made of the iterator table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
// Also note that if there has been a symbtabInstall call since the
//   iterator was created, the behavior is undefined (but probably
//   benign).
const char *symtabNext(void *iteratorHandle, void **returnData)
{
	
	hash_elem_it_t *it = iteratorHandle;
	entry_t *e = it->elem;
#if DEBUG	
	printf("Next Pair: [%s] and [%i]\n",e->key, e->value );
#endif
	if(it->elem->next != NULL){
		it->elem = it->elem->next;
	}else{
		it->index++;
		if(it->index >= it->ht->size) return NULL;
		it->elem = it->ht->table[it->index];
		do{
		
			if(it->elem != NULL && it->elem->key != NULL){
#if DEBUG
				printf("Index: [%i]\n", it->index);
#endif
				break;
			}else{
				it->index++;
				it->elem = it->ht->table[it->index];
			}
		}while(it->index < (it->ht->size - 1));
		
	}
	if(it->elem == NULL) return NULL;
	e = it->elem;

	*returnData = e->value;
	return e->key;
  
}

// Delete the iterator indicated by the only parameter.
// Reclaims all memory used by the iterator.
// Note that no validation is made of the iterator table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void symtabDeleteIterator(void *iteratorHandle)
{
}

