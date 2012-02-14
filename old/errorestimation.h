#ifndef _ERRORESTIMATION_H
#define _ERRORESTIMATION_H

#include <iostream>
#include <string>
#include <ext/hash_map>

using namespace std;
namespace __gnu_cxx
{
  template<> struct hash < std::string >
  {
    size_t operator()( const std::string& x ) const
    {
      return hash< const char* >()( x.c_str() );
    }
  };
}

/*
   struct eqstr
   {
   bool operator()(const char * s1, const char* s2) const
   {
   return strcmp(s1, s2) == 0;
   }
   }; 
   struct eqstr
   {
   bool operator()(string &s1, string &s2) const
   {
   return (s1 == s2);
   }
   }; 

   template <> class hash<std::string> {
   public:
   size_T operator(const std::string& h) {
   return hash<const char*>(h.c_str());
   }
   }; 
   template <>
   struct hash<string> : public hash<const char*> {
   size_t operator () (const string& x) const {
   return this->hash<const char*>::operator()(x.c_str());
   }
   };
   */

typedef __gnu_cxx::hash_map <string, unsigned int> Hmap;
typedef __gnu_cxx::hash_map <string, unsigned int> :: value_type Hmap_t;


class ErrorEstimation {

  private:
    unsigned int _epoch;
    unsigned long _count_inserts, _count_lookups;
    //hash_map <const char *, unsigned int, hash<const char*>, eqstr> _hmap;
    Hmap _hmap;
  public:
    ErrorEstimation() {
      _epoch = _count_inserts = _count_lookups = 0;

    }

    unsigned int insert(string flow_key) {
      Hmap :: iterator it;
      unsigned int rval;
      if ( (it = _hmap.find(flow_key) ) 
          == _hmap.end() ) { 
        _hmap[flow_key] = 1;
        //_hmap[flow_key] = 1;
        rval = 1;
      } else {
        rval = ++(it->second);
      } 
      _count_inserts++; _count_lookups++;
      return rval;
    }

    unsigned int get_flow_size(string flow_key) {
      Hmap :: iterator it;
      _count_lookups++;
      return (
          ((it = _hmap.find(flow_key) ) == _hmap.end()) 
          ?  0 
          : it->second 
          );
    }

    unsigned int get_num_flows() { return _hmap.size(); }

    void print() {
      Hmap :: const_iterator it;
      for (it = _hmap.begin();
          it != _hmap.end();
          ++it) {
        cout << "flow_count|key:" << it->first << "|count:" << it->second << endl;

      }
    }

    void next_epoch() {
      _epoch++;
      clear();
    }

    unsigned int get_epoch() const {  return _epoch; }

    void clear() {
      _hmap.clear();
      _count_lookups = _count_inserts = 0;
    }

};

/* 
   inline ostream& operator<<(ostream &o, ErrorEstimation &e) {
   hash_map <const char *, unsigned int, hash<const char*>, eqstr> :: const_iterator it;
   o << "flow_count|epoch:" << e.get_epoch() << endl;
   for (it = e.cbeginIt();
   it != e.cendIt();
   ++it) {
   o << "flow_count|key:" << it->first << "|count:" << it->second << endl;
   }
   return o;
   }
   */

#endif /* _ERRORESTIMATION_H */
