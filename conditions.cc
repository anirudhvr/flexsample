#include "conditions.h"

#include <unistd.h>
#include <ctype.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <cassert>

/* regexes for comments and empty lines */
 string skip_chr = "^\\s*(#.*)?$";
/* regex for the vars stmt */
 string vars_chr = "^\\s*vars\\s*=\\s*([[:digit:]]+)\\s*(#.*)?$";
/* regex for the definition of each variable */
 string vardef_chr = "^\\s*var_([[:digit:]]+)\\s*:=\\s*([[:alpha:]\\.]+)\\s*$";
/* regex for counter stuff */
 string counterdef_chr = "^\\s*counter var_([[:digit:]]+)\\s*:=\\s*"
   "([[:digit:]]+)\\s*,\\s*([[:digit:]]+)\\s*,\\s*([[:digit:]]*\\.[[:digit:]]+)"
   "\\s*,\\s*([[:digit:]]+)\\s*$";
/* regexes for condition count */
 string conditions_chr = 
"^\\s*conditions\\s*=\\s*([[:digit:]]+)\\s*(#.*)?$";
/* regexes for each condition */
 string cond_chr1 = "\\s*:\\s*";
 string cond_chr2 = "\\s+(AND|and)\\s+";
 string cond_chr3 = "var_([[:digit:]]+)\\s+(IN|in)\\s+\\"
"(\\s*([[:digit:]]+)\\s*,\\s*(([[:digit:]]+|INF))\\s*\\]";
 string cond_chr4 = "\\s*([[:digit:]]+?\\.[[:digit:]]+)\\s*";

using namespace std;

ConditionParser :: ConditionParser (string filename) :
  _filename (filename) {
}

void* 
ConditionParser :: error (string message) 
{
  cerr << message << endl;
  return NULL;
}

template <class T>
void 
ConditionParser ::  convert (string& foo, T& toreturn)
{
      stringstream ss;
      ss << foo; 
      ss >> toreturn;
}

SamplingBudgetLookupTable *
ConditionParser :: parse () {

  if (access(_filename.c_str(), R_OK)) {
    cerr << "Cannot read input file " << _filename << endl;
    return NULL;
  }

  bool num_vars_read = false;
  bool num_conditions_read = false;
  bool all_vardefs_read = false;
  bool all_counterdefs_read = false;
  bool all_conditions_read = false;
  SamplingBudgetLookupTable *sbt = NULL;

  unsigned int num_vars;
  unsigned int num_conditions;
  unsigned int num_vardefs_read = 0;
  unsigned int num_counterdefs_read = 0;

  /* regexes for comments and empty lines */
  regexp skip_reg (skip_chr);

  /* regexes for variable count */
  regexp vars_reg (vars_chr);

  /* regexes for condition count */
  regexp conditions_reg (conditions_chr);

  /* regexp for variable definitions */
  regexp vardef_reg (vardef_chr);

  /* regexp for counter definitions */
  regexp counterdef_reg (counterdef_chr);
  
  /* regexes for each condition */
  regexp cond_reg1 (cond_chr1);
  regexp cond_reg2 (cond_chr2);
  regexp cond_reg3 (cond_chr3);
  regexp cond_reg4 (cond_chr4);

  ifstream input;
  input.open (_filename.c_str(), ios::in);
  if (input.is_open()) {
    string str;
    while (getline(input, str)) { /* the main loop */

      vector<string> matches (1);

      /* skip spaces and comments */
      if (skip_reg.match (str, 1, matches)) {
        // cerr << "Skipping [" << str;
        continue;
      }

      /* first, we expect the number of vars */
      if (!num_vars_read) {
        if (!vars_reg.match (str, 2, matches))
          error ("Error in num vars parsing");
        convert (matches[1], num_vars);
        debug1 (stderr, "num_vars: %d\n", num_vars);
        num_vars_read = true;

      /* next, we expect the number of conditions */
      } else if (!num_conditions_read) {
        if (!conditions_reg.match (str, 2, matches))
          error ("Error in num conditions parsing");
        convert (matches[1], num_conditions);
        num_conditions_read = true;
        assert(num_vars_read = true); // should have already happened
        debug1 (stderr, "num_conditions: %d\n", num_conditions);

      /* next, we expect the descriptions of each variable */
      } else if (!all_vardefs_read) {
        unsigned int varidx;
        int numfields;

        /* create a new SamplingBudgetLookupTable if 
         * one doesnt already exist. We plan to store the 
         * variable definitions in there */
        if (!sbt)
          sbt = 
            new SamplingBudgetLookupTable (num_vars, num_conditions);

        /* try matching for a variable definition */
        if (!vardef_reg.match (str, 3, matches))
          error ("Error in parsing a vardef");

        //cout << "got matches " << 
          //matches[1] << ", " << matches[2] << endl;

        /* find which var it is */
        convert (matches[1], varidx);

        /* split the strings in the RHS of the vardef, 
         * and put them into the corresponding vector in
         * the sampling budget lookup table */

        numfields = split (matches[2], '.', 
            sbt->variable_definitions[varidx-1]);

        assert (numfields > 0); /* we got at least one bit */

        //for (int j = 0; j < numfields; ++j)
          //cout << sbt->variable_definitions[varidx-1][j] << ",";
        //cout << endl;

        if (++num_vardefs_read >= num_vars) 
          all_vardefs_read = true;
        

      /* finally, we expect the conditions themselves */
      } else if (!all_counterdefs_read) {
        unsigned int varidx;
        
        /* create a new SamplingBudgetLookupTable if 
         * one doesnt already exist. We plan to store the 
         * variable definitions in there */
        if (!sbt)
          sbt = 
            new SamplingBudgetLookupTable (num_vars, num_conditions);

        /* try matching the counter definitoin regex */
        if (!counterdef_reg.match (str, 6, matches))
          error ("Error parsing a counterdef");

        //cout << "got matches (";
        //copy (matches.begin(), matches.end(), 
            //ostream_iterator<string>(cout, "|"));
        //cout << ")" << endl;

        /* find which var it is for */
        convert (matches[1], varidx);

        /* split RHS and assign to structure */
        convert (matches[2], 
            sbt->counter_definitions[varidx-1].numcbfs);
        convert (matches[3], 
            sbt->counter_definitions[varidx-1].numelements);
        convert (matches[4], 
            sbt->counter_definitions[varidx-1].errorrate);
        convert (matches[5], 
            sbt->counter_definitions[varidx-1].rotatetime);

        if (++num_counterdefs_read >= num_vars)
          all_counterdefs_read = true;

      } else if (!all_conditions_read) {
        /* split on the ':' separating condition and sampling budget */
        if (!cond_reg1.split (str, matches)) 
          error ("Cannot split condition with ':'");
        assert (matches.size() == 2);

        /* parse the condition */
        vector<string> clauses (1);

        /* split may or may not work; we do not care */
        cond_reg2.split (matches[0], clauses); 
        assert (clauses.size() >= 1);

        /* the structure to provide to add_sb function */
        vector<pair<int, pair<int, int> > > cond(0);

        /* parse each clause */
        for (unsigned int i = 0; i < clauses.size(); ++i) {
          vector<string> clause_bits (1);
          if (!cond_reg3.match (clauses[i], 5, clause_bits) ||
              clause_bits.size() != 5) {
#if 0
            copy (clause_bits.begin(), clause_bits.end(), 
                ostream_iterator<string>(cerr, " " ));
            cerr << endl;
#endif
            error ("malformed clause");
          }

          int varnum, lb, ub;

          /* clause_bits[1] = variable number */
          /* clause_bits[3] = lowerbound of range */
          /* clause_bits[4] = upperbound of range */
          convert (clause_bits[1], varnum);
          convert (clause_bits[3], lb);
          if (clause_bits[4] == "INF")
            ub = __INT_MAX__;
          else
            convert (clause_bits[4], ub);

          cond.push_back (make_pair (varnum, make_pair (lb, ub)));

          for (vector<string>::const_iterator it2 = clause_bits.begin();
              it2 != clause_bits.end(); ++it2) {
            //TODO
            debug1 (stderr, "\tgot [%d]: %s\n", (it2 - clause_bits.begin()),
                (*it2).c_str());
          }
        }

        /* parse sampling budget */ 
        vector<string> sbvec (1);
        if (!cond_reg4.match(matches[1], 2, sbvec))
          error ("malformed RHS of condition");
        //TODO
        debug1 (stderr, "\t\tSB: %s\n", sbvec[0].c_str());

        float sb;
        convert (sbvec[0], sb);

        sbt->add_sb (cond, sb);

        if (++num_conditions_read == num_conditions) 
          all_conditions_read = true;
      }
    } /* main while loop ends */
    input.close();
  } else {
    fprintf (stderr, "Cannot open input file %s\n", _filename.c_str());
    return NULL;
  }

  sbt->finalize();

  return sbt;
} /* parse() ends */


