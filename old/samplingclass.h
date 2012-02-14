#ifndef _SAMPLINGCLASS_H
#define _SAMPLINGCLASS_H

#include <vector>
#include <iostream>
#include <map>
#include <cassert>
using namespace std;


class ProbabilityLine {
  private:
    map <float, unsigned int> _prline;
    float _count;
    float _samplerate;
  public:
    ProbabilityLine(float s) { _samplerate = s; _count = 0.0;}
    void add(float pr, unsigned int high_thresh_for_pr) {
      assert (pr > 0.0);
      assert (_count + pr <= 1.0);
      _count += pr;
      _prline[_count] = high_thresh_for_pr;
    }
    bool check() {
      return (_count <= 1.0 && _count >= 0.0);
    }

    void clear() {
      _prline.clear();
      _count = 0.0;
    }

    unsigned int operator[](float pr_to_check) const {

      if (pr_to_check > _samplerate) return 0;

      map <float, unsigned int>::const_iterator it;
      if ( (it = _prline.lower_bound(pr_to_check)) != _prline.end()) {
        return it->second;
      } else {
        return INT_MAX;
      }
    }

    void set(float pr_to_set, unsigned int value) {
      _prline[pr_to_set] = value;
    }
};

class SamplingClass 
{
  friend class SamplingBins;
  private:
  // this is the upperbound for this sampling class. an upperbound
  // of INT_MAX implies this is the highest sampling class.
  // NOTE: no lowerbounds needed in this impl
  unsigned int _upperbound;
  // alpha is the target fraction of sampled traffic that we want
  // this sampling class to occupy (sampling budget)
  float _alpha;
  // sampling probability for this class for the current epoch
  float _curr_probability;
  // estimate per packet	
  float _estimate;
  // delta is the *estimate* for the fraction of actual traffic
  // mix that this sampling class seems to use up. this is ewma'd
  // with the factor below
  float _curr_delta;
  // this is the rate at which we modify delta
  float _ewma_factor;
  // this is the default netflow sampling rate
  float _samplerate;
  // the current epoch (epoch count)
  unsigned int _epoch;
  // packets seen in the current epoch
  unsigned long _packets_seen;
  // packet counts for each epoch
  vector<unsigned long> _packet_counts; 

  ProbabilityLine &_prline;

  //SamplingBins &_samplingbins;
  float recalculate_probability(float fraction);
  void set_probability(float prob) { _curr_probability =  prob; }
  void set_estimate(float est) { _estimate =  est; }

  public:
  SamplingClass(unsigned int high, float alpha, float ewma_factor,
      //SamplingBins &samplingbins, float initial_delta);
    float initial_delta, float samplerate, ProbabilityLine &prline);

  void next_epoch(unsigned int totpktcount_thisepoch);

  float get_probability() const { return _curr_probability; }

  float get_estimate() const { return _estimate; }

  unsigned int get_threshold() const { return _upperbound; }

  float get_delta() const { return _curr_delta; }

  float get_alpha() const { return _alpha; }
  void set_alpha(float alpha) { _alpha =  alpha; }
  void decrement_alpha(float val) { _alpha -= val;}

  unsigned long get_packets_seen() const { return _packets_seen; }
  void set_packets_seen (unsigned int count) { _packets_seen = count; }
  unsigned long inc_packets_seen() { return _packets_seen++; }
  unsigned long inc_packets_seen(unsigned int value) { return (_packets_seen += value); }
  unsigned long dec_packets_seen() { return _packets_seen--; }
  unsigned long dec_packets_seen(unsigned int value) { 
    //assert (_packets_seen > value); 
    if (_packets_seen <= value) {
      cout << "errro: pktsseen: " << _packets_seen << " upperbnd: " << _upperbound << " val: " << value << endl;
      assert(0);
    }

    return (_packets_seen -= value) ;  
  }
}; 

inline ostream& operator<<(ostream &o, SamplingClass &s) {
  o << "[upperbnd:" << s.get_threshold() << ", alpha:" 
    << s.get_alpha() << ", curr_pr: " << s.get_probability() 
    << ", fract: " << s.get_delta() << "]";
  return o;
}


#endif // _SAMPLINGCLASS_H

