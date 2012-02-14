#include "conditions.h"
#include "sblookuptable.h"
#include "regexp.h"
#include "util.h"

#include <iostream>
using namespace std;


int main () {
    ConditionParser cp ("input.conf");
    SamplingBudgetLookupTable *sbt = cp.parse();
    cout << sbt->print_table() << endl;
    
    // int lookup[] = {60, 350, 25};
    int lookup[] = {49, 300, 20};
    vector<int> var_counts(lookup, lookup+3);
    cout << "got lookup result " 
      << sbt->lookup(var_counts) << endl;
    return 0;
}
