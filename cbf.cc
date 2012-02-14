#include "cbf.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <openssl/sha.h>
#include <string.h>
#include <queue>
#include "util.h"
#include "config.h"
using namespace std;

/*
 * Counting Bloom Filter
 */
// constructor 
template <class T>
CountingBloomFilter<T>::CountingBloomFilter(unsigned long num_elements, 
    float error_rate) {
  double lowest_m = -1.0;
  float best_k = 1.0;
  int i;

  // compute number of hash functioins 
  // for given error rate
  for (i = 1; i <= 100; ++i) {
    double m = (-1.0 * i * num_elements) /
      log( (1.0 - pow((double)error_rate, (double)(1.0/i))) );
    if (lowest_m == -1.0 || (m < lowest_m)) {
      lowest_m = m;
      best_k = (float)i;
    }
  }

  debug1(stderr, "lowest m: %lf, best_k: %f\n", lowest_m , best_k);

  _size = (unsigned long) lowest_m;

  //cout << "ctor called --- allocating bf with size " << _size << endl;
  // alloc space, zero'ed
  if ( (_filter = (T *) calloc (_size, sizeof(T))) == NULL) {
    cerr << "memory allocation failed for BF";
    exit(1);
  }
  _hash_count = (unsigned int) best_k;

  /* set the max val for the chosen type */
  switch (sizeof (T)) {
    case 1:
      _T_max_val = 255;
      break;
    default: /* dont expect it will be greater than that */
      _T_max_val = 4294967295L;
      break;
  }

  // build hashing functions
  //seeds.resize(hash_count);
  //i = 1;
  //for (vector<unsigned>::const_iterator it = seeds.begin();
  //it != seeds.end();
  //++it, ++i) 
  //seeds[i-1] = i;
}

/* to avoid template errors */
template
CountingBloomFilter<CBF_TYPE1>::CountingBloomFilter(unsigned long num_elements, 
    float error_rate);
template
CountingBloomFilter<CBF_TYPE2>::CountingBloomFilter(unsigned long num_elements, 
    float error_rate);


// this dude below is private
template <class T>
unsigned int 
CountingBloomFilter<T>::get_index(char *key, unsigned int k)
{
  // FIXME
  // This is so lame... since we want independent hashing
  // functions, and i couldn't think of a 'provably' good way
  // to build independent hashing fns out of the same key and
  // function call (SHA1()), i'm adding the integer 'k' to
  // the first byte of the key, in the hope that SHA1() will
  // generate an 'independent' value -- kind of like a seed
  // for the hashing function. The value of k is the # of the
  // hash function (1...num_hash_functions).
  // FWIW, it does give us different-looking keys, but since
  // i'm not a theory guy, i cant tell you any more than that
  // :)
  char *newkey = strdup (key);
  if (k >= 255) 
    k = k % 256;
  newkey[0] += k;

  unsigned int *ptr = (unsigned int*)_sha_output, index;
  SHA1((const unsigned char*)newkey, strlen(newkey), _sha_output);
  // Ok so SHA1 returns a 160 bit string (ie, 5 chars). Since we cant
  // address that much memory, i break it up into 5 chunks, XOR them,
  // and use that as my "index". This index is further modulo'ed with
  // the BF size to finally get the index into the BF to be updated
  index = *ptr ^ *(ptr+1) ^ *(ptr+2) ^ *(ptr+3) ^ *(ptr+4);
  index %= _size;
  free(newkey);
  return index;
}
template unsigned int 
CountingBloomFilter<CBF_TYPE1>::get_index(char *key, unsigned int k);
template unsigned int 
CountingBloomFilter<CBF_TYPE2>::get_index(char *key, unsigned int k);

// returns the average of the values (after the insert)
// for each bf entry this key hashes to
template <class T>
unsigned long  
CountingBloomFilter<T>::insert(char *key) {
  unsigned int i;
  unsigned long rval = 0;
  //cout << "inserting key " << key << endl;
  for (i = 0; i < _hash_count; ++i) {
    unsigned int index = get_index(key, i);
    //cout << "index = " << index << "filter val: " <<  (unsigned int)_filter[index] << endl;
    if (_filter[index] < _T_max_val) 
      ++_filter[index];
    rval += _filter[index];
  }
  //cout << "rval: " << rval << ", hash_count: " << _hash_count << endl; 
  return (rval/_hash_count);
}
template unsigned long  
CountingBloomFilter<CBF_TYPE1>::insert(char *key);
template unsigned long  
CountingBloomFilter<CBF_TYPE2>::insert(char *key);

template <class T>
bool
CountingBloomFilter<T>::remove(char *key) {
  unsigned int i;
  for (i = 0; i < _hash_count; ++i) {
    unsigned int index = get_index(key, i);
    if (_filter[index] > 0) 
      _filter[index]--;
  }
  return true;
}
template bool
CountingBloomFilter<CBF_TYPE1>::remove(char *key);
template bool
CountingBloomFilter<CBF_TYPE2>::remove(char *key);

template <class T>
bool 
CountingBloomFilter<T>::has(char *key) {
  unsigned int i;
  for (i = 0; i < _hash_count; ++i) {
    unsigned int index = get_index(key, i);
    if (_filter[index] == 0) 
      return false;
  }
  return true;
}
template bool 
CountingBloomFilter<CBF_TYPE1>::has(char *key);
template bool 
CountingBloomFilter<CBF_TYPE2>::has(char *key);

template <class T>
bool 
CountingBloomFilter<T>::clear() {
  return (memset ((void*) _filter, 0, _size * sizeof(T))) ? true : false;
}
template bool 
CountingBloomFilter<CBF_TYPE1>::clear();
template bool 
CountingBloomFilter<CBF_TYPE2>::clear();

/*
 * CBFArray - vector of CBFs
 */

template <class T>
CBFArray<T>::CBFArray(unsigned int numcbfs,
    unsigned long numelements,
    float errorrate,
    unsigned int rotatetime) : _cbfarray(numcbfs) {

  debug1 (stderr, "creating %d counting bloom filters\n", numcbfs);
  //cout << "size of cbfarray : " << _cbfarray.size() << endl;
  for (unsigned int i = 0; i < numcbfs; ++i) 
    _cbfarray[i] = new CountingBloomFilter<T>(numelements, errorrate);
  _numelements = numelements;
  _rotatetime = rotatetime;
  _numcbfs = numcbfs;
  _current = _num_inserts = 0;
}
template CBFArray<CBF_TYPE1>::CBFArray(unsigned int numcbfs,
    unsigned long numelements, float errorrate,
    unsigned int rotatetime);
template CBFArray<CBF_TYPE2>::CBFArray(unsigned int numcbfs,
    unsigned long numelements, float errorrate,
    unsigned int rotatetime);



// inserts the key  into each of the cbfs
// returns the value from the currently-used BF
template <class T>
unsigned long  
CBFArray<T>::insert(char *key) {
  // XXX not sure we need this
  // just to prevent too many collisions
  if (++_num_inserts >= _numelements) {
    rotate();
  }
  unsigned long rval = 0;

  for (unsigned int i = 0; i < _numcbfs; ++i) {
    unsigned long temp = _cbfarray[i]->insert(key);
    if (i == _current)
      rval = temp;
  }
  return rval;
}
template unsigned long  
CBFArray<CBF_TYPE1>::insert(char *key);
template unsigned long  
CBFArray<CBF_TYPE2>::insert(char *key);

template <class T>
bool
CBFArray<T>::remove(char *key) {
  bool rval = false;

  for (unsigned int i = 0; i < _numcbfs; ++i) {
    bool temp = _cbfarray[i]->remove(key);
    if (i == _current)
      rval = temp;
  }
  return rval;
}
template bool
CBFArray<CBF_TYPE1>::remove(char *key);
template bool
CBFArray<CBF_TYPE2>::remove(char *key);

template <class T>
bool
CBFArray<T>::has(char *key) {
  return _cbfarray[_current]->has(key);
}
template bool
CBFArray<CBF_TYPE1>::has(char *key);
template bool
CBFArray<CBF_TYPE2>::has(char *key);

template <class T>
unsigned int
CBFArray<T>::rotate() {
  _cbfarray[_current]->clear();
  _num_inserts = 0;
  _current = (_current+1) % _numcbfs;
  return _current;
}
template unsigned int
CBFArray<CBF_TYPE1>::rotate();
template unsigned int
CBFArray<CBF_TYPE2>::rotate();

template <class T>
unsigned int
CBFArray<T>::currentbf() const {
  return _current;
}
template unsigned int
CBFArray<CBF_TYPE1>::currentbf() const;
template unsigned int
CBFArray<CBF_TYPE2>::currentbf() const;

