/* $Id: regexp.h,v 1.5 2007/08/21 23:46:37 avr Exp $ */
#ifndef __REGEXP_H
#define __REGEXP_H 1

#include <regex.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <cassert>
#include <string>
#include <iostream>

const int MAX_MATCHES = 20;

using namespace std;
/*
 * A C++ wrapper for the POSIX regex library
 */

class regexp {
  private:
    regex_t *_regex;
    const unsigned int _max_matches;
    string _init_string;

  public:
    regexp();
    regexp(string &pattern, int cflags = REG_EXTENDED | REG_ICASE,
        unsigned int max_matches = MAX_MATCHES);
    ~regexp();
    int match(string &msg, size_t times, 
        vector<string> &matched, int eflags = 0);
    int get_match_offsets(string &msg,
        pair<unsigned int, unsigned int> &offset, int eflags = 0);
    int find_all_matches  (string &msg, 
        vector<string> &parts, int eflags = 0);
    int split (string &msg, 
        vector<string> &parts, int eflags = 0);
    void populate_from_file (string &filename);
    friend inline ostream& operator<<(ostream &out, regexp &r);
    string to_string () const { return _init_string; }


};

inline ostream& 
operator<<(ostream &out, regexp &r) 
{
  out.flush();
  return (out << r._init_string);
}

#endif /* __REGEXP_H */
