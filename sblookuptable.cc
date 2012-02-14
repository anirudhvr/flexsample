#include "sblookuptable.h"
#include <algorithm>
#include <iterator>
#include "util.h"

SamplingBudgetLookupTable :: SamplingBudgetLookupTable ( 
    unsigned int num_vars, unsigned int num_conditions) : 
  _var_ranges(num_vars), _storeall (0), variable_definitions (num_vars),
  counter_definitions(num_vars)
{
  _num_vars = num_vars;
  _num_conditions = num_conditions;
  _sb_values.clear();

  /* make clog not do any output if NDEBUG is not defined */
#ifndef NDEBUG
  std::clog.setstate (std::ios_base::badbit);
#endif

}

SamplingBudgetLookupTable :: ~SamplingBudgetLookupTable()
{
  delete [] _table;
}

bool
SamplingBudgetLookupTable :: add_range (unsigned int varnum,
    int lb, int ub, int rindex)
{
  pair<set<Range*>::iterator, bool> ret;

  Range *r = new Range (lb, ub, rindex);

  clog << "adding new range " << r->print()  <<
    " to var " << varnum << endl;

  ret = _var_ranges[varnum].insert (r);

  if (!ret.second) {
    cerr << "inserting duplicate?" << r->print() << endl;
    delete r;
    //return false;
  } else {
    clog << "inserting var " << varnum << ", "  
      << r->print() << endl;
  }
  return true;
}

bool
SamplingBudgetLookupTable :: add_range (unsigned int varnum,
    int lb, int ub)
{
  return add_range (varnum, lb, ub, -1);
}

void 
SamplingBudgetLookupTable :: fill_empty_ranges ()
{
  unsigned int varcount = 0;
  int curr_lb;
  unsigned int rangeindex;
  vector<set<Range*, RangeCompare> >::iterator vit;
  set<Range*, RangeCompare>::iterator sit;

  _range_sizes.clear();


  for (vit = _var_ranges.begin(); vit != _var_ranges.end(); 
      ++vit, ++varcount) {
    curr_lb = 0;

    clog << "range: " << varcount << ", range size before: " 
      << _var_ranges[varcount].size() << " " << endl;

    for (rangeindex = 0, sit = vit->begin(); 
        sit != vit->end(); ++sit) {

      clog << "inspecting range: " << (*sit)->print() << endl;

      /* add a filler range before the current range */
      if ((*sit)->low > curr_lb) 
        add_range (varcount, curr_lb, (*sit)->low, rangeindex++);

      (*sit)->index = rangeindex++;

      curr_lb = (*sit)->high;
    }

    /* FIXME: struct Range uses type int, but if that changes, 
     * this __INT_MAX__ limit will also need to change */
    if (curr_lb < __INT_MAX__)
      add_range (varcount, curr_lb, __INT_MAX__, rangeindex++);

    clog << "range size after: " << _var_ranges[varcount].size() 
      << endl;
    _range_sizes.push_back (_var_ranges[varcount].size());
  }

  clog << "\t\t ranges after: " << endl;
  for (vit = _var_ranges.begin(); vit != _var_ranges.end(); 
      ++vit, ++varcount) {
    for (rangeindex = 0, sit = vit->begin(); 
        sit != vit->end(); ++sit) {
      clog << (*sit)->print() << endl;
    }
    clog << endl;
  }
}

void
SamplingBudgetLookupTable :: add_sb 
(vector<pair<int, pair<int, int> > > in, float sb)
{
  /* store stuff for second pass */
  _storeall.push_back (in);
  _sb_values.push_back (sb);

  vector<pair<int, pair<int, int> > >::iterator it;

  /* iterate over each range spec in the condition */
  for (it = in.begin(); it != in.end(); ++it)
    /* the range spec to the appropriate variable */
    /* XXX: need to decrement because ocnf file says 
     * 'var_1' for the first var */
    add_range (it->first-1, (it->second).first, (it->second).second);
}

void
SamplingBudgetLookupTable :: get_next_permutation_all (int curr, 
    vector<int> &indices)
{
  unsigned int i;
  fill (indices.begin(), indices.end(), 0);

  clog << "getting permutation2 for " << curr << "\n";

  /* assumes that the indices are set to 0 by default, so that 
   * if curr hits 0 as it is depleted, we can simply quit */

  for (i = _num_vars - 1; curr && i; --i) {
    indices[i] = curr / _range_index_multipliers[i];
    clog << "setting index[" << i << "] to "
      << curr << " / " << _range_index_multipliers[i] << endl;
    curr %= _range_index_multipliers[i];
  }

  indices[0] = curr;

  copy (indices.begin(), indices.end(), 
      ostream_iterator<int>(clog, " "));
  clog << endl;
}

void
SamplingBudgetLookupTable :: get_next_permutation (int curr, 
    vector<int> &unspecified_indices, vector<int> &indices)
{
  unsigned int i;
  int multiplier = 1;

  assert (unspecified_indices.size() > 0);

  clog << "getting permutation for " << curr << " with ";
  copy (unspecified_indices.begin(), unspecified_indices.end(),
      ostream_iterator<int>(clog, " "));
  clog << endl;

  for (i = 0; i < unspecified_indices.size(); ++i) 
    multiplier *= _range_sizes[unspecified_indices[i]];

  /* assumes that the indices are set to 0 by default, so that 
   * if curr hits 0 as it is depleted, we can simply quit */
  for (i = unspecified_indices.size() - 1; curr && i; --i) {
    multiplier /= _range_sizes[unspecified_indices[i]];
    indices[unspecified_indices[i]] = curr / multiplier;
    clog << "setting index[" << unspecified_indices[i] << "] to "
      << curr << " / " << multiplier << endl;
    curr %= multiplier;
  }
  indices[unspecified_indices[0]] = curr;
}

void
SamplingBudgetLookupTable :: set_value_basic (vector<int> &indices,
    float value)
{
  unsigned int i, index = 0;
  for (i = 0; i < _num_vars; ++i)
    index += indices[i] * _range_index_multipliers[i];

  clog << "actually setting [";
  copy (indices.begin(), indices.end(),
      ostream_iterator<int>(clog, " "));
  clog << "] to " << value << endl;

  if (index >= _table_size) {
    clog << "table only " << _table_size << " but setting index " <<
      index << endl;
    abort();
  }

  if (_table[index] == 0.0) --_unset_cells;

  _table[index] += value;
}

int
SamplingBudgetLookupTable :: set_value (vector<int> &indices,
    int howmany_cells, float value)
{
  //int set_cells = 0;
  unsigned int i;
  bool no_unspecified_indices = true;
  vector<int> unspecified_indices;
  vector<int> indices_tmp (indices);
  for (i = 0; i < _num_vars; ++i)
    if (indices[i] == -1)  {
      indices_tmp[i] = 0;
      unspecified_indices.push_back (i);
      no_unspecified_indices = false;
      clog << "index " << i << " unspecified" << endl;
    }

  for (i = 0; i < (unsigned)howmany_cells; ++i) {
    /* this modifies indices_tmp */
    if (!no_unspecified_indices)
      get_next_permutation (i, unspecified_indices, indices_tmp);
    set_value_basic (indices_tmp, value);
  }
  return 0;
}

void
SamplingBudgetLookupTable :: finalize ()
{
  vector<vector<pair<int, pair<int, int> > > >::iterator crazyit;
  vector<pair<int, pair<int, int> > >::iterator condit;
  set<Range*, RangeCompare>::iterator sit;
  int condcount;
  int num_cells_in_table = 1;
  int sb_div_factor = 1;
  float sb_left = 1.0;
  int multiplier = 1;
  int i;
  vector<bool>::iterator bit;

  clog << "\tfinalize stage" << endl;

  /* all condition ranges have been added. 
   * now deduce and fill remaining ranges */
  fill_empty_ranges ();

  /* find how many cells the SB lookup table needs */
  for (vector<unsigned int>::iterator cvit = _range_sizes.begin();
      cvit != _range_sizes.end(); ++cvit) {
    num_cells_in_table *= *cvit;
    _range_index_multipliers.push_back (multiplier);
    multiplier *= *cvit;
  }

  clog << "SBLT needs " << num_cells_in_table << " cells\n";

  _num_cells_in_table = num_cells_in_table;

  /* allocate enough space for all condition ranges */
  _table_size = num_cells_in_table;
  _table = new float [_table_size];

  /* fill table with zeros for now */
  fill (_table, _table + num_cells_in_table, 0.0);

  /* will come in useful later */
  _unset_cells = num_cells_in_table;
  
  clog << "range index multipliers: ";
  copy (_range_index_multipliers.begin(), _range_index_multipliers.end(),
      ostream_iterator<int>(clog, " "));
  clog << endl;

  /* pass 2: run through the input again */
  /* FIXME: this stores all conditions in memory, which is not really a
   * good idea. But getting things working > unlikely complexity.
   */
  for (condcount = 0, crazyit = _storeall.begin(); 
      crazyit != _storeall.end(); ++crazyit, ++condcount) {
    vector<bool> count_vars_in_condition (_num_vars, false);
    vector<int> indices (_num_vars, -1);

    float sb_per_cell = _sb_values[condcount];

    clog << "condition: " << condcount << " sb value: " << 
      _sb_values[condcount] << endl;

    for (condit = crazyit->begin(); condit != crazyit->end(); 
        ++condit) {
      Range r ((condit->second).first, (condit->second).second);

      clog << "looking up range " << r.print() << endl;

      count_vars_in_condition[condit->first - 1] = true;

      assert ( (sit = _var_ranges[condit->first-1].find (&r)) !=
              _var_ranges[condit->first - 1].end() );
      //indices[condit->first - 1] = sit - _var_ranges.begin();
      assert ((*sit)->index >= 0);

      clog << "found range " << (*sit)->print() << endl;

      indices[condit->first - 1] = (*sit)->index;
    }

    /* we need to  calculate the number of unspecified variable ranges
     * in this condition, so we can distribute the sampling budget among
     * all the cells in the table that include the dontcare variables
     */
    sb_div_factor = 1;
    for (i = 0, bit = count_vars_in_condition.begin();
        bit != count_vars_in_condition.end(); ++bit, ++i)
      if (!(*bit)) {
        sb_div_factor *= _range_sizes[i];
        clog << "range for variable " << i << " unspecified\n";
      }
          
    assert (sb_div_factor > 0.0);
    sb_left -= sb_per_cell;
    sb_per_cell /= (float)sb_div_factor;

    /* now run over the array storing the indices of specified variable
     * ranges in order to determine the cells that need to be stuffed
     * with sb_per_cell above (XXX EDIT: new function for this shit)
     */

    /* sets the provided value to one or more elements of '_table'
     * according to how many entries of 'indices' are -1 */

    /* returns the number of cells set */
    clog << "trying to set " << sb_per_cell << " for " <<
      sb_div_factor << " cells " << endl;
    set_value (indices, sb_div_factor, sb_per_cell);
  }

  if (sb_left < 0.0) { 
    cerr << "\tsb_left should be >= 0 but is " << sb_left << endl;
    abort();
  }
  if (sb_left > 0.0) {
    assert (_unset_cells > 0);
    sb_left /= (float)_unset_cells;

    clog << "setting " << sb_left << " for " <<
      _unset_cells << " cells" << endl;
    for (int i = 0; i < num_cells_in_table; ++i)
      if (_table[i] == 0)
        _table[i] += sb_left;
  }

  /* clear the _storeall var */
  _storeall.clear();

  debug1(stderr, "%s", print_table().c_str());
}

unsigned int 
SamplingBudgetLookupTable :: calculate_index (vector<int> &indices)
{
  unsigned int i;
  unsigned int index = 0;

  assert (indices.size() == _num_vars);

  for (i = 0; i < _num_vars; ++i) 
    index += _range_index_multipliers[i] * indices[i];

  return index;
}

float
SamplingBudgetLookupTable :: get_value (vector<int> &indices)
{
  unsigned int i;
  unsigned int index = 0;

  assert (indices.size() == _num_vars);

  for (i = 0; i < _num_vars; ++i) 
    index += _range_index_multipliers[i] * indices[i];

  assert (_num_cells_in_table > index);
  return _table[index];
}

float
SamplingBudgetLookupTable :: get_value_raw (unsigned int index)
{
  if (index >= _table_size) {
    cerr << "Index " << index << " too big for SBLT" << endl;
    abort();
  }
  return _table[index];
}

string
SamplingBudgetLookupTable :: print_table ()
{
  stringstream ss;
  unsigned int i;
  float foo, total = 0;
  vector<int> indices(_num_vars, 0);
  vector<int> unsp_indices_hack(_num_vars, 0);

  for (i = 0; i < _num_cells_in_table; ++i) {
    get_next_permutation_all (i, indices);
    ss << "indices: ";
    copy (indices.begin(), indices.end(), 
        ostream_iterator<int>(ss, " "));
    ss << ", value: " << (foo = get_value (indices)) << endl;
   total += foo; 
  }

  ss << "total for table " << total << endl;

  return ss.str();
}

/* the main lookup function */
float
SamplingBudgetLookupTable :: lookup (vector<int> &var_counts)
{
  unsigned int i;
  set<Range*, RangeCompare>::iterator it;
  vector<int> indices (_num_vars, 0);

  for (i = 0; i < _num_vars; ++i) {
    Range r (var_counts[i], var_counts[i]); /* dummy */
    it = _var_ranges[i].upper_bound (&r);
    if (it != _var_ranges[i].end()) {
      // the looked-up number is not 
      // within the very last range
      indices[i] = (*it)->index - 1;
      // the -1  is because we're doing _upper bound_, so
      // we get the index of the range above the one we matched
    } else { 
      // the looked up number is the 
      // very last range 
      indices[i] = (*_var_ranges[i].rbegin())->index;
    }
    assert (indices[i] >= 0);
  }

  clog << "lookup: got indices: ";
  copy (indices.begin(), indices.end(), 
      ostream_iterator<int>(clog, " "));

  return get_value (indices);
}

/* the lookup function tricked to also return the 
 * index of the sampling class  */
float
SamplingBudgetLookupTable :: lookup (vector<int> &var_counts, 
    unsigned int *samplingclass_index)
{
  unsigned int i;
  set<Range*, RangeCompare>::iterator it;
  vector<int> indices (_num_vars, 0);

  for (i = 0; i < _num_vars; ++i) {
    Range r (var_counts[i], var_counts[i]); /* dummy */
    it = _var_ranges[i].upper_bound (&r);
    if (it != _var_ranges[i].end()) {
      // the looked-up number is not 
      // within the very last range
      indices[i] = (*it)->index - 1;
      // the -1  is because we're doing _upper bound_, so
      // we get the index of the range above the one we matched
    } else { 
      // the looked up number is the 
      // very last range 
      indices[i] = (*_var_ranges[i].rbegin())->index;
    }
    assert (indices[i] >= 0);
  }

  clog << "lookup: got indices: ";
  copy (indices.begin(), indices.end(), 
      ostream_iterator<int>(clog, " "));

  *samplingclass_index = calculate_index (indices);
  return get_value_raw (*samplingclass_index);
}

/* the lookup function but returning only the index of the sampling
 * class  */
unsigned int
SamplingBudgetLookupTable :: lookup_index (vector<int> &var_counts)
{
  unsigned int i;
  set<Range*, RangeCompare>::iterator it;
  vector<int> indices (_num_vars, 0);

  for (i = 0; i < _num_vars; ++i) {
    Range r (var_counts[i], var_counts[i]); /* dummy */
    //cout << "looking up " << var_counts[i] << endl;
    it = _var_ranges[i].upper_bound (&r);
    if (it != _var_ranges[i].end()) {
      // the looked-up number is not 
      // within the very last range
      indices[i] = (*it)->index - 1;
      // the -1  is because we're doing _upper bound_, so
      // we get the index of the range above the one we matched
    } else { 
      // the looked up number is the 
      // very last range 
      indices[i] = (*_var_ranges[i].rbegin())->index;
    }
    assert (indices[i] >= 0);
  }

  clog << "lookup: got indices: ";
  copy (indices.begin(), indices.end(), 
      ostream_iterator<int>(clog, " "));

  return calculate_index (indices);
}
 
