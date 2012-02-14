#include "samplingclass.h"
#include "samplingbins.h"

using namespace std;

SamplingClass::SamplingClass (unsigned int high, float alpha, 
    float ewma_factor, float initial_delta, float samplerate,
    ProbabilityLine &prline) : _prline(prline) {
  _upperbound = high;
  _ewma_factor = ewma_factor;
  _epoch = 1;
  _alpha = alpha;
  _samplerate = samplerate;
  _packets_seen = 0;
  _curr_delta = initial_delta;
  assert (_curr_delta > 0.0);
  assert (_alpha >= 0.0);
  assert (samplerate > 0.0);
  _curr_probability = _samplerate * _alpha / _curr_delta;
  _estimate = 1.0/_curr_probability;
  _prline.set(_curr_probability, _upperbound);
  //cout << *this << " created\n";
} 

float SamplingClass::recalculate_probability(float fraction) {
  assert (fraction >= 0.0 && fraction <= 1.0);
  _curr_delta = _curr_delta * (1 - _ewma_factor) + fraction * _ewma_factor;
  _curr_probability = _samplerate * _alpha / _curr_delta;
  _estimate = 1.0/_curr_probability;
  _prline.set(_curr_probability,_upperbound);
  return _curr_probability;
}

void SamplingClass::next_epoch(unsigned int totpktcount_thisepoch) {
  cout << "next epoch: pkts seen: " << _packets_seen << " totpkts: " << totpktcount_thisepoch << endl;
  recalculate_probability((float)_packets_seen/(float)totpktcount_thisepoch);
  _packet_counts.push_back(_packets_seen);
  _packets_seen = 0;
  _epoch++;
  cout << "epoch: " << _epoch << "; " << *this << endl;
  cout.flush();
}
