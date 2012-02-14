#ifndef __CONDITIONS_H
#define __CONDITIONS_H 1

#include "regexp.h"
#include "util.h"
#include "sblookuptable.h"
#include <map>


/* regexes for comments and empty lines */
extern  string skip_chr;
/* regex for the vars stmt */
extern  string vars_chr;
/* regexes for condition count */
extern  string conditions_chr;
/* regexes for vardefs */
extern  string vardef_chr;
/* regexes for each condition */
extern  string cond_chr1;
extern  string cond_chr2;
extern  string cond_chr3;
extern  string cond_chr4;

class ConditionParser {
  private:
    string _filename;
    void* error (string message);

    template <class T> 
    void convert (string& foo, T& toreturn);

  public:
    ConditionParser (string filename);

    // this method creates and returns the filled sampling budget
    // lookup table
    SamplingBudgetLookupTable* parse ();
};


#endif /* __CONDITIONS_H */
