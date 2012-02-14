#ifndef __SBLOOKUPTABLE_H
#define __SBLOOKUPTABLE_H

#include <set>
#include <vector>
#include <ios>
#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

//#include "flexsample.h"

/* #define NDEBUG 1 */

/* 
 * null output stream: 
 * http://groups.google.com/group/comp.lang.c++/browse_frm/thread/1e411f7dddc679be/130c419481f40c40?lnk=gst&q=null+output+stream#
 */
/*
struct nullstream:
  std::ostream {
    struct nullbuf: std::streambuf {
      int overflow(int c) { return traits_type::not_eof(c); }
    } m_sbuf;
    nullstream(): std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
  }; 
 
*/

/* This class maps ranges of integers to float 
 * The class should be extensible to map any type, actually */
struct Range {
  int low, high;
  int index;
  //float data;

  Range(int l, int h)
  { 
    low = l; high = h; 
    index = -1;
    //data = d;
  }

  Range(int l, int h, int ind)
  { 
    low = l; high = h; 
    index = ind;
    //data = d;
  }

  string print()
  {
    stringstream ss;
    ss << "index: " << index << ": [" << low << ", " << high << ")";
    return ss.str();
  }
};

struct RangeCompare {
  bool operator() (Range *r1, Range *r2) const {
    return (r1->low < r2->low);
  }
};

/* XXX this is stuff from the input conf file that must be
 * passed to flexsample. this is a bad place to put this, but what hte
 * hell...
 */
struct counterdef {
  unsigned int numcbfs;
  unsigned long numelements;
  float errorrate;
  unsigned int rotatetime;
};


/*
 * The sampling budget lookup table class
 */
class SamplingBudgetLookupTable {
  /* friends */
  friend class FlexSample;
  private:
    float *_table;
    unsigned int _table_size;
    unsigned int _num_vars;
    unsigned int _num_conditions;
    unsigned int _num_cells_in_table;
    unsigned int _unset_cells;
    vector<float> _sb_values; /* temp storage kinda */

    vector<set<Range*, RangeCompare> > _var_ranges;

    /* as the name suggests, this is clearly a hack to do 2-pass
     * processing on the inputs without having to make the caller
     * provide the input twice */
    vector<vector<pair<int, pair<int, int> > > > _storeall; 
    vector<unsigned int> _range_sizes;
    /* we need this array to look up multipliers for each 
     * index */
    vector<int> _range_index_multipliers;


  public:
    /* yep, it's public */
    vector<vector<string> > variable_definitions;

    /* this one too */
    vector<counterdef> counter_definitions;

    /* ctor */
    SamplingBudgetLookupTable (unsigned int num_vars, 
        unsigned int num_conditions); 

    ~SamplingBudgetLookupTable ();

    bool add_range (unsigned int varnum, 
        int lb, int ub);

    bool add_range (unsigned int varnum, 
        int lb, int ub, int rindex);

    void fill_empty_ranges ();

    void add_sb (vector<pair<int, pair<int, int> > > in,
        float sb);

    void get_next_permutation (int curr, 
        vector<int> &unspecified_indices, vector<int> &indices);

    void get_next_permutation_all (int curr, 
        vector<int> &indices);

    void set_value_basic (vector<int> &indices,
        float value);

    int set_value (vector<int> &indices, 
        int howmany_cells, float value);

    float get_value (vector<int> &indices);

    float get_value_raw (unsigned int index);

    float lookup (vector<int> &var_counts);

    unsigned int lookup_index (vector<int> &var_counts);

    float lookup (vector<int> &var_counts,
        unsigned int *samplingclass_index);

    void finalize ();

    unsigned int calculate_index (vector<int> &indices);

    string print_table();

    /* getters and setters */
    unsigned int num_vars () const {
      return _num_vars;
    }
};

#endif /* __SBLOOKUPTABLE_H */
