/* $Id: regexp.cc,v 1.3 2007/08/27 00:29:21 avr Exp $ */
#include "regexp.h"
#include <iostream>

using namespace std;

regexp::regexp() : _max_matches (MAX_MATCHES), _init_string ("")
{
  _regex = NULL;
}

regexp::regexp(string &pattern, int cflags, unsigned int max_matches) 
  : _max_matches (max_matches) {
    int c;
    if ( !(_regex = (regex_t *) malloc (sizeof(regex_t))) ) {
        cerr << "Could not allocate memory for regex!" << endl;
        exit (1);
    }
    if ( (c = regcomp (_regex, pattern.c_str(), cflags)) ) {
      char buf[128];
      regerror (c, _regex, buf, sizeof(buf));
      cerr << "Regexp error: " << pattern.c_str() << ": " << buf << endl;
        //cerr << "Regexp error: " << endl;
        exit (1);
    }
    _init_string = pattern;
}

regexp::~regexp() {
    if (_regex) regfree(_regex);
}

int
regexp::match(string &msg, size_t times, 
        vector<string> &matched, int eflags) {
    if (!_regex) {
        cerr << "Regex inexplicably null!" << endl;
        return 1;
    }

    if (times > _max_matches) times = _max_matches;
    
    assert (times > 0);
    
    regmatch_t *pmatch = 
        (regmatch_t*) malloc(sizeof(regmatch_t) * times);

    if (regexec (_regex, msg.c_str(), times, pmatch, eflags) ==
            REG_NOMATCH) {
        if (pmatch) free (pmatch);
        return 0;
    } else {
        unsigned int i;
        matched.resize(times);  // output structure

        for (i = 0; i < times; ++i) {
            int c = pmatch[i].rm_so;
            matched[i] = "";
            do matched[i] += msg[c++]; while (c < pmatch[i].rm_eo);
        }
        if (pmatch) free (pmatch);
        return 1;
    }
}

int 
regexp::get_match_offsets(string &msg,
    pair<unsigned int, unsigned int> &offset, int eflags)
{
    if (!_regex) {
        cerr << "Regex inexplicably null!" << endl;
        return 1;
    }

    regmatch_t pmatch;
    if (regexec (_regex, msg.c_str(), 1, &pmatch, eflags) ==
            REG_NOMATCH) {
        return 0;
    } else { // regexec succeeded
      offset = make_pair(pmatch.rm_so, pmatch.rm_eo);
      return 1;
    }
}

int regexp::find_all_matches (string &msg, 
    vector<string> &matches, int eflags) {
  unsigned int off_start = 0, off_end = msg.length();
  int rval = 0;
  matches.clear();
  pair<unsigned int, unsigned int> off_curr;
  string substr = msg.substr(off_start, off_end);
  //cout << "trying: " << substr << endl;
  while ( get_match_offsets(substr, off_curr, eflags) ) {
    //cout << "got substr: " << substr << " first " << off_curr.first <<
      //", second " << off_curr.second << endl;

    matches.push_back(substr.substr 
        (off_curr.first, off_curr.second - off_curr.first));

    assert (off_curr.second <= msg.length());

    off_start = off_curr.second;

    substr = substr.substr(off_start, off_end);

    //cout << "trying: " << substr << endl;
    ++rval;
  }
  return rval;
}

int regexp::split (string &msg, 
    vector<string> &parts, int eflags) 
{ 
  unsigned int off_start = 0, off_end = msg.length();
  int rval = 0;
  parts.clear();
  pair<unsigned int, unsigned int> off_curr;
  string substr = msg.substr(off_start, off_end);
  //cout << "trying: " << substr << endl;
  while ( get_match_offsets(substr, off_curr, eflags) ) {
    //cout << "substr: " << substr << " start: " << off_start 
      //<< " first: " << off_curr.first << ", second " 
      //<< off_curr.second << endl;

    if (off_curr.first > 0)
      parts.push_back(substr.substr(0, off_curr.first));

    assert (off_curr.second <= msg.length());

    off_start = off_curr.second;

    substr = substr.substr(off_start, substr.length());

    //cout << "trying: " << substr << endl;
    ++rval;
  }
    //cout << "outsidexx: substr: " << substr << " start: " << off_start 
      //<< " first: " << off_curr.first << ", second " 
      //<< off_curr.second << endl;
  if (substr.length())
    parts.push_back(substr);

  return rval;
}


