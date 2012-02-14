#include "flexsample.h"
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

bool verbose;

FlexSample::FlexSample(string filename,
    SamplingBudgetLookupTable &sblt, 
    unsigned int num_vars,
    float samplerate = 0.01,
    float hashrate = 1.0,
    float bferrorrate = 0.01,
    float epsilon = 0.75,
    unsigned long bfsize = 100000,
    unsigned int numcbfs = 4,
    unsigned int rotatetime = 100000)
: _filename (filename), 
  _sblt (sblt),
  _counters(num_vars,  NULL)
{
  unsigned int i;
  _samplerate = samplerate;
  _hashrate = hashrate;

  _epsilon = epsilon;

  _num_vars = num_vars;

  _bferrorrate = bferrorrate;
  _bfsize = bfsize;
  _numcbfs = numcbfs;
  _rotatetime = rotatetime;

  /* 
   * Initialize counters using the 
   * counter_definitions structure part of the 
   * sampling budget lookup table 
   */

  for (i = 0; i < num_vars; ++i)
    _counters[i] = new CBFArray<CBF_TYPE> 
      (_sblt.counter_definitions[i].numcbfs,
       _sblt.counter_definitions[i].numelements,
       _sblt.counter_definitions[i].errorrate,
       _sblt.counter_definitions[i].rotatetime);

      /* NOTE: because we're friends with with 
       * SamplingBudgetLookupTable, we're allowed to 
       * look at its private members ;) */

    /* initialize _fractions and _probabilities MD arrays. Both 
     * have the same size as the SBLT.
     */ 

    /* _fractions initialized to 1/number_of_sampling_classes */
    _fractions = new MDArray<float> (_sblt._num_vars,
        _sblt._range_sizes, 1/((float)_sblt._table_size));

  /* probabilities initialized to 0; will be re-initialized later */
  _probabilities = new MDArray<float> (_sblt._num_vars,
      _sblt._range_sizes, (float)0.0);  

  /* create a similar M-D array to count the packets per 
   * each sampling class */
  _packets_per_sc = new MDArray<unsigned> (_sblt._num_vars,
      _sblt._range_sizes, (unsigned int)0); 

  /* 
   * initializations of structures 
   */ 

  /* initialize PRNG */
  srand((unsigned)time(NULL));

  /* initialize probabilities array */
  for (i = 0; i < _probabilities->size(); ++i) {
    /* ideally need an assert for the denom here to not be zero, 
     * but we just set it above to non-zero. trust the machine! */
    _probabilities->set_value_raw (i, 
        ((_sblt.get_value_raw(i) * _samplerate) / 
         _fractions->get_value_raw(i)) );
  }
}

FlexSample::~FlexSample()
{
  delete _fractions;
  delete _probabilities;
  delete _packets_per_sc;
}

/* The function with the main flexsample loop */
bool FlexSample::go() {
  /* Integer to count total packets seen in each epoch, to
   * decide when to end one epoch and start the next */
  unsigned int pktcount_thisepoch = 0;
  /* to count total packets seen */
  unsigned int packetcount = 0;
  /* count the number of sampled packets */
  unsigned int sampled_packets = 0;
  /* the number of packets when we rotate *all* CBFArrays */
  /* XXX will need fixing when each var's CBFs arent the same size */
  unsigned long bf_rotate_pktcount = _bfsize / _numcbfs;
  unsigned int i, j;

  vector<string> vars(_sblt._num_vars); // buffer to read in vars
  vector<string> fields;
  vector<string> keys (_sblt._num_vars);
  vector<int> varcounts (_sblt._num_vars);
  string delim("|"); /* XXX do not hardcode */

  /* open input file */
  ifstream infile;
  cerr << "reading file " << _filename.c_str() << endl;
  infile.open(_filename.c_str(), ios::in);

  if (infile.is_open()) {
    float newfrac;
    //float samplingbudget;
    float inst_sampling_prob;
    float random_float;
    unsigned int samplingclass_index;

#if 0
    unsigned int botnetpackets = 0;
#endif

    cerr << "file open" << endl;
    string line;
    // Read first line to get field annotations
    infile >> line;
    chomp (line);

#if 0
    bool skip = true;
#endif

    /* Construct a struct that will allow easy 
     * dereferencing of field names
     * allowed field names:
     * time, srcip, srcport, dstip, dstport, 
     * prot, tcpflags, ip_len
     */
    FieldNameDereferencer fn (delim, line); 
    //cerr << "setting up FieldNameDereferencer: " << delim << ", " 
      //<< line << endl;

    /* print first output line, containing headers */
    cout << "class|prob|time|srcip|srcport|dstip|dstport|ipflags|prot|pktlen|tcpflags" << endl;

    /* Read file */
    while (infile >> line) { /* XXX: beware, will only read till the first space */
      chomp (line);
      split (line, delim, fields);

#if 0
      if (fields[fn["time"]] == "1209761640") 
        skip = false;
      if (skip) continue;
#endif

      /*
       * FIXME
       * Things to do
       *  - Rotate each counter independently of _everything_ else 
       *  - Recalculate inst. sampling probability at some period, 
       *    _independent_ of counter rotation
       */
      /* 
       * Test whether it is time to rotate 
       */
      if (pktcount_thisepoch >= bf_rotate_pktcount) {
        /* rotate all CBF arrays
         * XXX this should change if we rotate some 
         * variables' CBFs independently
         */
        for (vector<CBFArray<CBF_TYPE>*>::iterator it = _counters.begin();
            it != _counters.end(); ++it)
          (*it)->rotate();

        cerr << "rotating all cbfs at " << fields[fn["time"]] 
          << ", packets seen = " << packetcount << endl;
#if 0
          cerr << "botnet packets: " << botnetpackets << endl;
          botnetpackets = 0;
#endif

        for (i = 0; i < _fractions->size(); ++i) {

          debug1 (stderr, "%d f: [before: %f, ", i, 
              _fractions->get_value_raw(i));

          // Update fractions matrix with current counts using EWMA
          newfrac = 
            ( (_fractions->get_value_raw (i) * (1-_epsilon)) +  // term 1
              (((float)_packets_per_sc->get_value_raw (i) / 
                (float)pktcount_thisepoch) * _epsilon) // term 2
            );
          _fractions->set_value_raw (i, newfrac);

          debug1 (stderr, "new: %f] ", newfrac);

          debug1 (stderr, "p: [before: %f, ", _probabilities->get_value_raw(i));

          //re-calculate sampling probabilities for each sampling class
          _probabilities->set_value_raw (i, 
              min( (_sblt.get_value_raw (i) * _samplerate / newfrac), 1.0) );

          debug1 (stderr, "new: %f] ", _probabilities->get_value_raw(i));

          debug1 (stderr, "pkts: %d\n\t", _packets_per_sc->get_value_raw(i));


        }

        // zero out all entries of packets_per_sc
        _packets_per_sc->zero();
        pktcount_thisepoch = 0;
      }

      /* 
       * Not yet time to rotate. Do normal stuff 
       */

      /*
      cout << "variable defitions: ";
      copy (_sblt.variable_definitions[0].begin(), 
          _sblt.variable_definitions[0].end(), 
          ostream_iterator<string>(cout, " "));
      cout << endl << "fields: " ;
      copy (fields.begin(), 
          fields.end(), 
          ostream_iterator<string>(cout, " "));
      cout << endl;
      */


      /* assert all the requested variable definitions
       * exist in the input file that was provided */
      for (i = 0; i < _sblt._num_vars; ++i) 
        for (j = 0; j < _sblt.variable_definitions[i].size(); ++j)
          assert (fn[_sblt.variable_definitions[i][j]] >= 0);

      /* do the following stuff for each variable */
      for (i = 0; i < _sblt._num_vars; ++i) {
        // Create keys for each variable by concatenating the
        // fields of each line (packet) according to variable definitions
        // in the input conf file
        keys[i] = "";
        for (j = 0; j < _sblt.variable_definitions[i].size(); ++j) {
          //cout << i << ", " << _sblt.variable_definitions[i][j] << 
            //", " << fn[_sblt.variable_definitions[i][j]] << ", " << 
            //fields[1] << endl;
          if (fn[_sblt.variable_definitions[i][j]] > 0 &&
             (unsigned)fn[_sblt.variable_definitions[i][j]] < fields.size())
            keys[i] += "|" + fields[fn[_sblt.variable_definitions[i][j]]];
        }

        //cout << "key " << i << " looks like " << keys[i] << endl;

        // Insert / Lookup each key in the respective CBFArray 
        varcounts[i] = _counters[i]->insert ((char*)keys[i].c_str());
      }

      //cout << "got varcounts ";
      //copy (varcounts.begin(), varcounts.end(), 
          //ostream_iterator<int> (cout, " "));
      //cout << endl;

      /* now that we have got estimates of counts of each variable,
       * query the varcounts in the sampling budget lookup table to
       * get the sampling budget of this particular packet */
      samplingclass_index = _sblt.lookup_index (varcounts);

      /* samplingclass_index indicates the index which we can use to
       * lookup the elements of _fractions and _probabilities in order
       * to calculate the instantaneous sampling probability for this
       * packet */

      /* increment packets seen for this sampling class */
      _packets_per_sc->set_value_raw (samplingclass_index,
          _packets_per_sc->get_value_raw (samplingclass_index) + 1);

      /* find the instantaneous probability this packet should be
       * sampled at */
      inst_sampling_prob = 
        _probabilities->get_value_raw (samplingclass_index);

#if 0
      if (fields[fn["srcip"]] == "143.215.129.45" && 
          fields[fn["dstip"]].find("206.197.119.") == 0) { 
        // portscan packet 
        cerr << "sampling portscan packet at prob: " << inst_sampling_prob
          << " in class " << samplingclass_index; 
        cerr << ". counts: var_1: " << varcounts[0] << ", var_2: " 
          << varcounts[1] << endl;
      }
#endif

#if 0
      /* if (fields[fn["dstip"]] == "143.215.15.107") */ 
      if (fields[fn["tcpflags"]] == "B") {
        // botnet packet 
        cerr << "sampling botnet packet at prob: " << inst_sampling_prob
          << " in class " << samplingclass_index; 
        cerr << ". counts: ";
        for (unsigned int j = 0; j < _sblt._num_vars; ++j)
          cerr << "var_" << j+1 << ": " << varcounts[j] << ", ";
        cerr << endl;
        ++botnetpackets;
      }
#endif


      /* generate random value, and sample 
       * according to how that turns out */
      random_float = get_random_float();

      //cout << "inst. samping prob: " << inst_sampling_prob 
        //<< ", random float : " << random_float << endl;

      /* find out whether we need to sample this packet or not */
      if (random_float <= inst_sampling_prob) {
        if (!verbose) {
          cout << samplingclass_index << "|" << inst_sampling_prob << "|" << line << endl;
        } else {
          cout << "sampling in class " << samplingclass_index 
            << ": " << line << ", prob: " << inst_sampling_prob << endl;
        }
        ++sampled_packets;

        // XXX print other stuff
      }

      // Increment counters
      packetcount++;
      pktcount_thisepoch++;

    } // file read ends

    cerr << "Total pkts seen: " << packetcount 
      << ", total sampled: " << sampled_packets << endl;
    infile.close();
  } // file end
  else {
    cerr << "file not opened for some strange reason" << endl;
    return false;
  }
  return true;
} // go() ends



/* alas, no built-in string tokenizer */
void FlexSample::tokenize(const char* str, char delim, vector<char*>& tokenList)
{
  tokenList.clear();
  int start = 0, i;
  for (i = 0; str[i]; i++)
  {
    if (str[i] == delim)
    {
      char* token = new char[i-start+1];
      memcpy(token, &str[start], i-start);
      token[i-start] = '\0';
      tokenList.push_back(token);
      start = i+1; //to skip reading the delimiter
    }

  }
  // to read a leftover token at the end
  if (start <= (i-1)) {
    char *token = new char[i-start+1];
    memcpy(token, &str[start], i-start);
    token[i-start] = '\0';
    tokenList.push_back(token);
  }
}

