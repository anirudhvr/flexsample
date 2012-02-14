#ifndef _FLEXSAMPLE_H
#define _FLEXSAMPLE_H

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <ctime>
#include <errno.h>

#include "cbf.h"
#include "conditions.h"
#include "sblookuptable.h"
#include "util.h"
#include "config.h"

/* #include "samplingbins.h" */
/* #include "errorestimation.h" */

#define CBF_TYPE unsigned int


/* i like this name */
class FlexSample {
  private:
    /* name of input file */
    string _filename;

    /* the smaplingbudgetlookup table we're going 
     * to be using */
    SamplingBudgetLookupTable &_sblt;

    /* one CBFArray for each variable */
    vector<CBFArray<CBF_TYPE>*> _counters;

    /* one array like the sblookuptable to store fractions */
    MDArray<float> *_fractions;

    /* another array like sblt and fractions to store instantaneous
     * sampling probabilities */
    MDArray<float> *_probabilities;

    /* a similar M-D array to count the packets per 
     * each sampling * class */
    MDArray<unsigned int> *_packets_per_sc;

    /* parameters related to reading packets */
    float _samplerate, _hashrate;

    /* EWMA parameter to vary the "fraction" matrix */
    float _epsilon;
    
    /* number of variables */
    unsigned int _num_vars;

    /* CBFArray parameters
     * currently the same for all CBFArrays 
     */
    float _bferrorrate;
    unsigned long _bfsize;
    unsigned int _numcbfs;
    unsigned long _rotatetime;

    /* alas, no built-in string tokenizer */
    void tokenize(const char* str, char delim, 
        vector<char*>& tokenList);

    /* get a random float number... duh! */
    inline float get_random_float() {
      return ((float)rand()/(float)(RAND_MAX)); 
    }

  public:

    FlexSample (string filename, /* input file */
        SamplingBudgetLookupTable &sblt, /* ref to SB lookup table*/
        unsigned int numvars, /* number of variables */
        float samplerate, /* overall rate of packet sampling */
        float hashrate,   /* rate at which packets are inspected */
        float bferrorrate, /* error rate for each bf */
        float epsilon,     /* the coefficient for EWMA */
        unsigned long bfsize, /* size of each bf */
        unsigned int numcbfs, /* number of CBFs per CBF array */
        unsigned int rotatetime /* time after which rotate a CBF*/
        );

    /* destructor */
    ~FlexSample ();

    bool go();  // go() ends

    inline float ewma(float first, float second, 
        float ewma_factor) {
      return (first * ewma_factor) + (second * (1-ewma_factor));
    }

    inline float max (float a, float b) {
      return (a > b) ? a : b;
    }

    inline float min (float a, float b) {
      return (a > b) ? b : a;
    }


}; // FlexSample ends

class FieldNameDereferencer {
  private:
    string _delim;
    vector<string> _fieldnames;
    string_int_hash _fieldindices;

  public:
    FieldNameDereferencer (string &delim, string &firstline) : _delim (delim) {
      _fieldnames.clear();
      if (!split (firstline, _delim, _fieldnames)) {
        cerr << "Could not split " << firstline << " based on "
          << _delim << endl;
        abort();
      }

      vector<string>::iterator it;
      int c;
      cerr << "field name mappings: ";
      for (c = 0, it = _fieldnames.begin();
          it != _fieldnames.end(); ++it) {
        _fieldindices[*it] = c++;
        cerr << "[" << *it << ": " << _fieldindices[*it] << "] ";
      }
      cerr << endl;
    }

    int operator[] (const string &fieldname) {
      string_int_hash::iterator it;
      if ((it = _fieldindices.find(fieldname)) != _fieldindices.end()) {
        return (it)->second;
      } else {
        return -1;
      }
    }
};


/*
struct RangeQuery {
  vector<pair<unsigned int, unsigned int> > ranges;
  RangeQuery() {}

  // FIXME: need to call add in sorted order of ranges, or things will
  // break badly
  void add (unsigned int lowerbound, unsigned int upperbound) {
    //cout << "adding " << lowerbound << " " << upperbound << endl;
    ranges.push_back(make_pair(lowerbound,upperbound));
  }

  unsigned int getrangeindex(unsigned int count) {
    unsigned int i;
    for (i = 0; i < ranges.size(); ++i) {
      if (ranges[i].first <= count && count < ranges[i].second) {
        return i;
      }
    }
    return i;
  }
};

*/

#endif
