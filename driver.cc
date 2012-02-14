#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sstream>
#include "cbf.h"
#include "flexsample.h"
using namespace std;

extern int errno;
char *programname;


void usage ()
{
  cerr << "Usage: " << programname << " -c <config_file> [-e <ewma_factor> (default 0.75)]\n"
    "       " << "[-s <num_entries_per_bf> (default 100,000)] [-n <num_cbfs_in_cbfarray> (default 4)]\n"
    "       " << "[-r <base_sampling_rate> (default 0.01)] [-v (default: non-verbose)] <input_filename>\n"
    //"       " << "Example: '" << programname << " -a 0.1,5 -a 0.5,10 -n 8 -s 200000' constructs 3 sampling\n"
    //"       " << "classes: for T < 5, alpha = 0.01, for 5 <= T < 10, alpha = 0.5, for T >= 10, alpha = 0.4\n"
    << endl;
}

template <class T>
bool convert (char *to, T& from) 
{
  stringstream ss;
  ss << to; ss >> from;
  return true;
}
 
template <class T>
bool convert (string &to, T& from) 
{
  stringstream ss;
  ss << to; ss >> from;
  return true;
}

bool convert_pair (string &al_th, float &al, unsigned int &th, char delim = ',')
{
  // ',' is the delimiter
  string s1, s2;
  unsigned int delim_index = al_th.find_first_of (delim);
  if (delim_index > al_th.length() - 1)
    return false;
  s1 = al_th.substr(0, delim_index);
  convert (s1, al);
  s2 = al_th.substr(delim_index + 1, al_th.length());
  convert (s2, th);
  return true;
}

int main (int argc, char *argv[]) 
{
  // various flags to see if something was specified on the cmd line or not
  extern bool verbose;
  programname = argv[0];

  // default values 
  unsigned long bfsize = 1000000; // bf size
  float epsilon = 0.75; //default value of epsilon
  unsigned int numcbfs = 10; // number of CBFs in *each* CBFArray
  float bf_error_rate = 0.01; // error rate for bfs
  float hash_rate = 1.0; // hash rate. 1.0 => all packets are hashed
  float base_sampling_rate = 0.01; // sampling rate. 0.01 = 1/100 packets
  unsigned int rotate_time  = 30; // currently unused
  string configfile; // config file name

  // needed for getopt
  int c;
  extern int optind, optopt;
  extern char *optarg;

  // option parsing
  while ( (c = getopt (argc, argv, "c:e:s:n:r:v")) != -1 ) {
    switch (c) {
      case 'c': // config file
        convert (optarg, configfile);
        if (!is_readable (configfile)) {
          cerr << "Config file " << configfile << 
            " not readable" << endl;
          usage ();
          return 1;
        }
        break;

      case 'e':
        convert (optarg, epsilon);
        if (epsilon < 0.0 || epsilon > 1.0) { 
          cerr << "epsilon value should be b/w 0 and 1" << endl;
          usage();
          return 1;
        }
        break;

      case 's':
        convert (optarg, bfsize);
        if (bfsize < 0 || bfsize > 1000000) {
          cerr << "bf value not reasonable!" << endl;
          usage ();
          return 1;
        }
        break;

      case 'n':
        convert (optarg, numcbfs);
        if (numcbfs < 0 || numcbfs > 100) {
          cerr << "num_cbfs_in_cbfarray value insane" << endl;
          usage ();
          return 1;
        }
        break;

      case 'r':
        convert (optarg, base_sampling_rate);
        if (base_sampling_rate < 0.0 || base_sampling_rate > 1.0) { 
          cerr << "base sampling rate should be b/w 0 and 1" << endl;
          usage();
          return 1;
        }
        break;

      case 'v':
        verbose = true;
        break;

      default:
        cerr << "option -" << (char)optopt << " not recognized" << endl;
        usage ();
        return 1;
    }
  }

  argv += optind;
  argc -= optind;

  if (*argv == NULL)  {
    usage(); return 1;
  }

  /* input file */
  /* FIXME: currently assumes the input file is just one file (so
   * creates the sampling budget lookup table for just that */
  string filename = *argv;

  if (!is_readable(filename)) {
    cerr << "File: " << filename << " is not readable" << endl;
    usage ();
    return 1;
  }

  /* create sampling budget lookup table */
  ConditionParser cp (configfile);
  SamplingBudgetLookupTable *sbt = cp.parse();
  if (!sbt) {
    cerr << "Cannot create sampling budget lookup table, " 
      "likely due to a parse error" << endl;
    return 1;
  }

  sbt->print_table();

  FlexSample fs(filename,
      *sbt,
      sbt->num_vars(),
      base_sampling_rate,
      hash_rate,
      bf_error_rate,
      epsilon,
      bfsize,
      numcbfs, 
      rotate_time);

  fs.go();
  return 0;
}

