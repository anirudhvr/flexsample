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
#include "samplingbins.h"
using namespace std;

extern int errno;
char *programname;


void usage ()
{
  cerr << "Usage: " << programname << " [-a <alpha,threshold> (default 0.5,50)] [-e <ewma_factor> (default 0.75)]\n"
    "       " << "[-s <num_entries_per_bf> (default 100,000)] [-n <num_cbfs_in_cbfarray> (default 4)]\n"
    "       " << "[-r <base_sampling_rate> (default 0.01)] [-v (default: verbose)] <input_filename>\n"
    "       " << "Example: '" << programname << " -a 0.1,5 -a 0.5,10 -n 8 -s 200000' constructs 3 sampling\n"
    "       " << "classes: for T < 5, alpha = 0.01, for 5 <= T < 10, alpha = 0.5, for T >= 10, alpha = 0.4\n"
    << endl;
}

bool is_readable (string &file) { 
  return ( (access (file.c_str(), R_OK) < 0) ? false : true );
}

bool is_readable (char * file) { 
  return ( (access (file, R_OK) < 0) ? false : true );
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

int main (int argc, char *argv[]) {
  // various flags to see if something was specified on the cmd line or not
  extern bool verbose;
  bool alpha_threshold_given = false;
  programname = argv[0];

  // default values 
  unsigned long bfsize = 100000; // bf size
  float epsilon = 0.75; //default value of epsilon
  float alpha_default = 0.5; unsigned int threshold_default = 50;
  vector< pair<float,unsigned int> > alpha_threshold_v; // to store the provided alpha,threshold pairs
  unsigned int numcbfs = 4;// number of CBFs in the CBFArray
  float bf_error_rate = 0.01;// error rate for bfs
  float hash_rate = 1.0; // hash rate. 1.0 => all packets are hashed
  float base_sampling_rate = 0.01; // sampling rate. 0.01 = 1/100 packets
  unsigned int rotate_time  = 30; // currently unused

  // needed for getopt
  int c;
  extern int optind, optopt;
  extern char *optarg;

  // option parsing
  while ( (c = getopt (argc, argv, "a:e:s:n:r:v")) != -1 ) {
    switch (c) {
      case 'a': {// this is an input of the form 'alpha,threshold'
        string alpha_threshold = optarg;
        float alpha_tmp; unsigned int threshold_tmp;
        if (!convert_pair (alpha_threshold, alpha_tmp, threshold_tmp))  {
          cerr << "arguments to '-a' not in the right form" << endl;
          usage();
          return 1;
        }
        alpha_threshold_v.push_back (make_pair (alpha_tmp, threshold_tmp));
        alpha_threshold_given = true;
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

  string filename = *argv;

  if (!is_readable(filename)) {
    cerr << "File: " << filename << " is not readable" << endl;
    usage ();
    return 1;
  }

  // make sure we have atleast one entry in alpha_threshold_v
  if (!alpha_threshold_given) 
    alpha_threshold_v.push_back (make_pair (alpha_default, threshold_default));

  assert (alpha_threshold_v.size () > 0);
  //SamplingBins sb(2, base_sampling_rate, epsilon);
  SamplingBins sb(alpha_threshold_v.size(), base_sampling_rate, epsilon);

  for (unsigned int i = 0; i < alpha_threshold_v.size(); ++i) 
    sb.add_bin(alpha_threshold_v[i].second, alpha_threshold_v[i].first);

  //sb.add_bin(threshold, alpha); // budget for flows < threshold => alpha% mice
  //sb.add_bin(2,0.01);
  //sb.add_bin(4,0.5);
  //sb.add_bin(threshold, 0.5); // budget for flows < threshold => 50% mice
  sb.print();

  //cout << *(sb[30]) << endl;
  //cout << *(sb[50]) << endl;
  //cout << *(sb[80]) << endl;
  //cout << *(sb[12380]) << endl;

  FlexSample fs(filename,
      sb,
      bfsize,
      numcbfs, 
      bf_error_rate,
      hash_rate,
      base_sampling_rate,
      rotate_time);

  fs.go();
  return 0;
}

