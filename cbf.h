#ifndef _CBF_H
#define _CBF_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <openssl/sha.h>
#include <string.h>
#include <queue>
using namespace std;

/* change this to make the CBF implement something else */
#define CBF_TYPE1 unsigned char
#define CBF_TYPE2 unsigned int


template <class T>
class CountingBloomFilter {
  private:
    // char => 8 bits per counter 
    /* unsigned char  *_filter; */
    T  *_filter; 

    unsigned long _T_max_val;

    unsigned long _size;
    unsigned int _hash_count;
    unsigned char _sha_output[20];
    //vector <unsigned> seeds;


    unsigned int get_index(char *key, unsigned int k);

  public:
    CountingBloomFilter(unsigned long num_elements, 
        float error_rate) ;

    /* FIXME
       Define Copy constructor and assignment operator! 
     */

    ~CountingBloomFilter() {
      if (_filter) 
        free (_filter);
      //cout << "dest called" << endl;
    }

    // returns the average of the values (after the insert)
    // for each bf entry this key hashes to
    unsigned long insert(char *key);

    bool remove(char *key);


    bool has(char *key);

    bool clear();
};

template <class T>
class CBFArray {
  private:
    vector<CountingBloomFilter<T>*> _cbfarray;
    unsigned int _rotatetime, _numcbfs, _current, _num_inserts;
    unsigned long _numelements;

  public:
    CBFArray(unsigned int numcbfs,
        unsigned long numelements,
        float errorrate,
        unsigned int rotatetime) ;

    ~CBFArray() {
      for (unsigned int i = 0; i < _numcbfs; ++i) 
        delete _cbfarray[i];
    }


    // inserts the key  into each of the cbfs
    // returns the value from the currently-used BF
    unsigned long insert(char *key);

    bool remove(char *key);

    bool has(char *key);

    unsigned int rotate(); 

    unsigned int currentbf() const ;
};


#endif /* _CBF_H */
