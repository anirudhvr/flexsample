/*
 * Hack file. Include a file like this to flexsample for a new
 * application with new dimensions / sizes of dimensions (ie, number of
 * ranges)
 */

#include <limits.h>
#include <vector>

unsigned int num_dimensions = 2;

unsigned int dim_sizes[2] = {4, 3};

unsigned int num_sampling_classes = 12;

// this matrix tells FS what fields in each input line (ie, packet) to
// look for. the number of rows = # of dimensions, and each row
// indicates the fields that should be concatenated to get this variable
unsigned int reqfields0[] =  {2, 3, 4, 5}; // srcip, srport, dstip, dstport
unsigned int reqfields1[] =    {4, 5}; // dstip, dstport
unsigned int *required_fields[] = {reqfields0, reqfields1 };

// this is the hack to create an array of arrays (each row with a
// differet number of elements). ranges[][] is the only thing that
// should be used externally
unsigned int range0[] =  {0, 200, 300, 500, UINT_MAX};
unsigned int range1[] =  {0, 90, 100, UINT_MAX};
unsigned int *ranges[] = {range0, range1};

// values of sampling budgets corresponding to ranges above
float sblut[4][3] = {
  { 0.025, 0.025, 0.025 },
  { 0.2, 0.025, 0.025 },
  { 0.025, 0.025, 0.025 },
  { 0.2, 0.2, 0.2 }
};

//dummy gamma matrix
float gamma[4][3] = {
  { 0.025, 0.025, 0.025 },
  { 0.2, 0.025, 0.025 },
  { 0.025, 0.025, 0.025 },
  { 0.2, 0.2, 0.2 }
};


//dummy fraction matrix -- initially set all equal
float fraction[4][3] = {
  {0.08333333333333333333, 0.08333333333333333333, 0.08333333333333333333},
  {0.08333333333333333333, 0.08333333333333333333, 0.08333333333333333333},
  {0.08333333333333333333, 0.08333333333333333333, 0.08333333333333333333},
  {0.08333333333333333333, 0.08333333333333333333, 0.08333333333333333333}
};

//dummy packets_per_sampling_class matrix
unsigned int pkts_per_sc[4][3] = {
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 }
};




