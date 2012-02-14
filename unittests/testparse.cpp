#include <iostream>
#include <vector>
#include "regexp.h"
#include "util.h"

using namespace std;

int main() {

  vector<string> v1;
  vector<string> v2;
  vector<string> v3(1);
  unsigned int i;

  string skip_str ("^\\s*#");
  regexp skip_regex (skip_str);
  string skip ("     #  ## # vars = 5 aksjdlksa jd \n");
  cout << "matches: " << skip_regex.match(skip, 5, v1) << endl;;


  string vars_str ("^\\s*vars\\s*=\\s*([[:digit:]]+)\\s*(#.*)?$");
  regexp vars_reg (vars_str);
  string vars ("   vars     = 20# lkajsdlkjsa \n");
  cout << "vars matches " << vars_reg.match(vars, 5, v1) << endl;


  string regex1 ("\\s*:\\s*");
  string regex2 ("\\s+(AND|and)\\s+");
  string regex3 ("var_([[:digit:]]+)\\s+(IN|in)\\s+\\(\\s*([[:digit:]]+)\\s*,\\s*(([[:digit:]]+|INF))\\s*\\]");

  regexp r1 (regex1);
  regexp r2 (regex2);
  regexp r3 (regex3);

  string cond ("var_1 in (25, INF]    and var_2 in (30, 40]        : 0.2");
  string cond1 ("var_1 in (25, INF] : 0.2");

  r1.split(cond, v1);
  cout << "first split: " << v1.size() << endl;
  for (i = 0; i < v1.size(); ++i) {
    cout << "|" << v1[i] << "|";
  }

  cout << endl;

  unsigned int val = r2.split (v1[0], v2);
  cout << "val: " << val << " v2 size " << v2.size() << endl;
  for (i = 0; i < v2.size(); ++i) {
    cout << "^" << v2[i] << "^";
  }
  cout << endl;

  cout << "str: " << v2[1] << ", num match: " << r3.match (v2[1], 5, v3) << endl;
  for (i = 0; i < v3.size(); ++i) {
    cout << v3[i] << " ## ";
  }

  cout << v3.size() << endl;


  return 0;
}
