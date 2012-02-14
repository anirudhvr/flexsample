/* $Id: util.cc,v 1.7 2007/12/15 15:36:30 avr Exp $ */
#include "util.h"
#include "config.h"

#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <cmath>
#include <algorithm>

#include <unistd.h>
#include <cstdio>
#include <cstdarg>

/* multi-dimensional array functions */

/*
 * debug functions 
 */

void
debug1 (FILE *stream, const char *fmt, ...) {
#ifdef DEBUG1
  va_list ap;
  va_start (ap, fmt);
  vfprintf (stream, fmt, ap);
  va_end (ap);
#endif
}

void
debug2 (FILE *stream, const char *fmt, ...) {
#ifdef DEBUG2
  fprintf (stderr, "coming here\n");
  va_list ap;
  va_start (ap, fmt);
  vfprintf (stream, fmt, ap);
  va_end (ap);
#endif
}

/* url-related stuff, i think */
const char *hexd= "0123456789ABCDEF";

const char path_enc_type1[256]=
{
  /*00*/  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /*10*/  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /*20*/  2,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,  /*  !"#$%&'()*+,-./ */
  /*30*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0103456789:;<=>? */
  /*40*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* @ABCDEFGHIJKLMNO */
  /*50*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* PQRSTUVWXYZ[\]^_ */
  /*60*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* `abcdefghijklmno */
  /*70*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,  /* pqrstuvwxyz{|}~  */
  /*80*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*90*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*A0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*B0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*C0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*D0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*E0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*F0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};


#if 0
static char path_enc_type[256]=
{
  /*00*/  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /*10*/  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /*20*/  2,0,2,2,0,2,4,0,0,0,0,0,0,0,0,0,  /*  !"#$%&'()*+,-./ */
  /*30*/  0,0,0,0,0,0,0,0,0,0,0,2,2,0,2,2,  /* 0123456789:;<=>? */
  /*40*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* @ABCDEFGHIJKLMNO */
  /*50*/  0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,0,  /* PQRSTUVWXYZ[\]^_ */
  /*60*/  2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* `abcdefghijklmno */
  /*70*/  0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,1,  /* pqrstuvwxyz{|}~  */
  /*80*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*90*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*A0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*B0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*C0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*D0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*E0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /*F0*/  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};
#endif

  void 
require (bool condition, string message)
{
#ifndef NDEBUG
  if (!condition) {
    std::cerr << message << endl;
    //abort();
  }
#endif
}

  bool 
is_readable (string &input_file)
{
  return (access(input_file.c_str(), R_OK) < 0) ? false : true;
}

  bool 
is_readable (char * file) 
{
  return ( (access (file, R_OK) < 0) ? false : true );
}

  void 
error (string file, unsigned int line, string message)
{
  std::cerr << "Error: " << file << ":" 
    << line << " - " << message << std::endl;
}

string* 
get_line (ifstream &in) {
  string *s = new string("");
  std::getline (in, *s);
  return s;
}

bool 
  get_line(ifstream &in, string &s) {
    if (std::getline (in, s) < 0) 
      return false;
    else 
      return true;
  }

string* 
get_line(FILE *in) {
  char *lineptr = NULL;
  size_t size;
  if (getline (&lineptr, &size, in) < 0)
    return NULL;
  else {
    require ( (lineptr != NULL), "Null line");
    string *toreturn = new string(lineptr);
    if (lineptr) free (lineptr);
    return toreturn;
  }
}

bool 
get_line(FILE *in, string &s) {
  char *lineptr = NULL;
  size_t size;
  s.erase();
  if (getline (&lineptr, &size, in) > 0) {
    require ( (lineptr != NULL), "Null line");
    s = lineptr;
    if (lineptr) free (lineptr);
    return true;
  } else
    return false;
}

void chomp (string &s) {
  unsigned int t;
  if ( (t = s.find_first_of('\n')) < s.length()) { // found newline
    s.erase(t, s.length() - 1);
  }
}

int 
split (string &msg, string &delim, vector<string> &parts) {
  unsigned int msg_length = msg.length();
  unsigned int delim_length = delim.length();
  unsigned int offset = 0;
  unsigned int rval = 0;
  unsigned int off_curr;

  if (! (msg_length && delim_length && msg_length >= delim_length) ) {
    std::cout << "msg: " << msg << "; msg len: " << msg_length 
      << "; delim len: " << delim_length << endl;
    abort();
  }
  parts.clear();
  while ( ((off_curr = msg.find (delim, offset)) 
        < msg_length) ) {
    if (off_curr > offset)
      parts.push_back(msg.substr(offset, off_curr - offset));
    offset = off_curr + delim_length;
    ++rval;
  }

  if (offset < msg_length && ++rval)
    parts.push_back(msg.substr(offset, msg_length - offset));

  return rval;
}

int 
split (string &msg, char delim, vector<string> &parts) {
  char tmp[] = {delim, '\0'};
  string delim_str(tmp);
  return split (msg, delim_str, parts);
}


int timer :: start ()
{
  if (time_valid) time_valid = false;
  if (!gettimeofday (&begin, NULL)) {
    begin_called = true;
    return 1;
  }
  return 0;
}

int timer :: stop ()
{
  if (time_valid) time_valid = false;
  if (!begin_called) {
    ERROR ("Timer not started yet");
    return 0;
  }

  if (!gettimeofday (&end, NULL)) {
    begin_called = false; /* reset */
    time_valid = true; /* set */
    return 1;
  }
  return 0;
}

int timer :: reset () 
{
  time_valid = false;
  begin_called = false;
  return 1;
}

double timer :: get_tdiff (int mul_factor, int automatic_reset) 
{
  if (!time_valid)  {
    ERROR ("Timer not started/finished counting");
    return 0.0;
  }
  time_t sec_diff = end.tv_sec - begin.tv_sec;
  suseconds_t usec_diff = end.tv_usec - begin.tv_usec;

  if (automatic_reset) reset();

  return ( (double)sec_diff * 1000.0 + (double)usec_diff / 1000.0 );
}


template <class T>
void vsort (vector<T> &v) {
  unsigned int size = v.size();
  for (unsigned int i = 0; i < size; ++i) {
    for (unsigned int j = 1; j < size; ++j) {
      if (v[i] > v[j]) {
        unsigned int tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
      }
    }
  }
}

