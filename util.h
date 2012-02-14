/* $Id: util.h,v 1.6 2007/12/15 15:36:30 avr Exp $ */
#ifndef __UTIL_H
#define __UTIL_H 1

#include <stdio.h>
#if _PACKAGE_ast
#undef	getline
#endif
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <ext/hash_map>
using namespace std;

#define ERROR(x) \
  error(__FILE__, __LINE__, x)

/* multi-dimensional array */
template <typename T>
class MDArray {
  private:
    unsigned int _dims;
    vector<unsigned int> _dim_sizes;
    vector<unsigned int> _dim_index_multipliers;
    T *data;
    unsigned int _total_size;

  public:
    /* constructor */

    MDArray (unsigned int dims, vector<unsigned int> &dim_sizes) :
      _dim_sizes(dim_sizes), _dim_index_multipliers(dims,1) {
        unsigned int i;
        _total_size = 1;
        _dims = dims;
        for (i = 1; i < _dims; ++i) {
          _total_size *= _dim_sizes[i - 1];
          _dim_index_multipliers[i] =
            _dim_index_multipliers[i - 1] * _dim_sizes[i - 1];
        }
        _total_size *= _dim_sizes[_dims - 1];
        assert (_total_size > 0); // ensures all _dim_sizes are > 0

        data = new T [_total_size];
      }

    /* alternate constructor */
    MDArray (unsigned int dims,
        vector<unsigned int> &dim_sizes,
        T default_val) :  _dim_sizes(dim_sizes), 
    _dim_index_multipliers(dims,1) {
      unsigned int i;
      _total_size = 1;
      _dims = dims;
      for (i = 1; i < _dims; ++i) {
        _total_size *= _dim_sizes[i - 1];
        _dim_index_multipliers[i] =
          _dim_index_multipliers[i - 1] * _dim_sizes[i - 1];
      }
      _total_size *= _dim_sizes[_dims - 1];
      assert (_total_size > 0); /* ensures all _dim_sizes are > 0 */

      data = new T [_total_size];
      fill (data, data+_total_size, default_val); /* only difference */
    }


    ~MDArray () {
      delete [] data;
    }

    /* FIXME 
     * create a copy constructor and assignment operator 
     */

    /* FIXME: assumes T is a basic type like int or string */
    bool set_value (vector<unsigned int> &indices, T value) {
      unsigned int i, index = 0;
      for (i = 0; i < _dims; ++i)
        index += indices[i] * _dim_index_multipliers[i];

      //clog << "actually setting [";
      //copy (indices.begin(), indices.end(),
      //ostream_iterator<int>(clog, " "));
      //clog << "] to " << value << endl;

      if (index >= _total_size) {
        cerr << "table only " << _total_size << 
          " but setting index " << index << endl;
        abort();
      }

      data[index] += value; /* XXX add instead of simply set */
    }

    /* function to set teh value 'value' to all entries specified by
     * listing a subset of the indices (ie, not completely specifying
     * all '_dims' indices). this function, given the unspecified
     * indices, sets all appropriate cells in the real table */
    bool set_value_multiple (int howmany, vector<int> &indices,
        T value)
    {
      //int set_cells = 0;
      unsigned int i;
      bool no_unspecified_indices = true;
      vector<int> unspecified_indices;
      /* potentially unnecessary copy, but needed if original indices
       * array needs to be preserved */
      vector<int> indices_tmp (indices); 
      for (i = 0; i < _dims; ++i)
        if (indices[i] == -1)  { 
          indices_tmp[i] = 0;
          unspecified_indices.push_back (i);
          no_unspecified_indices = false;
          //cout << "index " << i << " unspecified" << endl;
        }

      for (i = 0; i < (unsigned)howmany; ++i) {
        /* this modifies indices_tmp */
        if (!no_unspecified_indices)
          get_next_permutation (i, unspecified_indices, indices_tmp);
        set_value (indices_tmp, value);
      }
      return 0;
    }

    /* fetch the value of a cell using a set of provided indices
     * all indices must be specified */
    T get_value (vector<unsigned int> indices) 
    {
      unsigned int i;
      unsigned int index = 0;

      assert (indices.size() == _dims);

      for (i = 0; i < _dims; ++i) 
        index += _dim_index_multipliers[i] * indices[i];

      assert (_total_size > index);
      return data[index];
    }

    /* get value using the raw index in the linear storage array */
    T get_value_raw (unsigned int index)
    {
      if (index >= _total_size) {
        cerr << "Index " << index << " too big for MDArray" << endl;
        abort();
      }
      return data[index];
    }
    /* set counterpart of the above */
    void set_value_raw (unsigned int index, T value)
    {
      if (index >= _total_size) {
        cerr << "Index " << index << " too big for MDArray" << endl;
        abort();
      }
      data[index] = value;
    }

    void get_next_permutation (int curr, 
        vector<int> &unspecified_indices, 
        vector<int> &indices)
    {
      unsigned int i;
      int multiplier = 1;

      assert (unspecified_indices.size() > 0);

      //cout << "getting permutation for " << curr << " with ";
      //copy (unspecified_indices.begin(), unspecified_indices.end(),
      //ostream_iterator<int>(cout, " "));
      //cout << endl;

      for (i = 0; i < unspecified_indices.size(); ++i) 
        multiplier *= _dim_index_multipliers[unspecified_indices[i]];

      /* assumes that the indices are set to 0 by default, so that 
       * if curr hits 0 as it is depleted, we can simply quit */
      for (i = unspecified_indices.size() - 1; curr && i; --i) {
        multiplier /= _dim_sizes[unspecified_indices[i]];
        indices[unspecified_indices[i]] = curr / multiplier;
        //cout << "setting index[" << unspecified_indices[i] << "] to "
        //<< curr << " / " << multiplier << endl;
        curr %= multiplier;
      }
      indices[unspecified_indices[0]] = curr;
    }

    /* number of dimensions in this multi-dimensinoal array */
    unsigned int num_dimensions () const { return _dims; }
    /* total number of elements in this array */
    unsigned int size () const { return _total_size; }

    /* zero out the struct */
    void zero()
    {
      for (unsigned int i = 0; i < _total_size; ++i) 
        data[i] = (T)0; // cast
    }
};


/* timing */
class timer {
  struct timeval begin, end;
  bool time_valid;
  bool begin_called;
  public: 
  timer() { time_valid = false; begin_called = false; }
  int start();
  int stop();
  int reset();
  /* get time in multiples of microseconds */
  double get_tdiff (int mul_factor = 1000, int automatic_reset = 1); 
};


// debugging
void require (bool condition, string message);
void error (string file, unsigned int line, string message);
void debug1 (FILE *stream, const char *fmt, ...);
void debug2 (FILE *stream, const char *fmt, ...);


// file operations
bool is_readable (string &filename); 
bool is_readable (char *filename); 
string *get_line (ifstream &in);
bool get_line (ifstream &in, string &s);
string* get_line (FILE *in);
bool get_line (FILE *in, string &s);

// string operations
void chomp (string &s);
int split (string &msg, string &delim, vector<string> &parts);
int split (string &msg, char delim, vector<string> &parts);

/* for a string -> integer hash map */
namespace __gnu_cxx
{
  template<> struct hash < std::string >
  {
    size_t operator()( const std::string& x ) const
    {
      return hash<const char*>()( x.c_str());
    }
  };
}

typedef __gnu_cxx::hash_map <string, int> string_int_hash;
typedef __gnu_cxx::hash_map <string, string> string_string_hash;

/* sort */
template <class T> void vsort (vector<T> &v); 




#endif /* __UTIL_H */
