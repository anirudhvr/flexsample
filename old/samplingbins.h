#ifndef _SAMPLINGBINS_H
#define _SAMPLINGBINS_H

#include <map>
#include <cassert>
#include <iostream>
#include "samplingclass.h"
using namespace std;

class SamplingBins
{
  private:
    unsigned int _num_bins;
    map<unsigned int, SamplingClass*> _bins;
    float _samplerate;
    float _ewma_factor;
    ProbabilityLine _prline;
    SamplingClass _high_bin;
  public:
    SamplingBins(unsigned int num_bins, float samplerate,
        float ewma_factor) : _num_bins(num_bins), 
    _samplerate(samplerate), _ewma_factor(ewma_factor), _prline(samplerate),
    _high_bin(INT_MAX, 1.0, ewma_factor, (1.0/(float)_num_bins), samplerate, _prline) {}

    bool add_bin(unsigned int high, float alpha) {
      bool rval = true;
      try {
        SamplingClass *s = new SamplingClass(high, alpha, _ewma_factor, 
            (1.0/(float)_num_bins), _samplerate, _prline);
        _bins[high] = s;
        _high_bin.decrement_alpha(alpha);
        //_high_bin._alpha -= alpha;
        assert (_high_bin.get_alpha() >= 0.0);
      } catch (...) {
        rval = false;
      }
      return rval;
    }

    float get_alpha(unsigned int value) const {
      map<unsigned int, SamplingClass*>::const_iterator it = _bins.upper_bound(value);
      if (it == _bins.end()) {
        return _high_bin.get_alpha();
      } else {
        return it->second->get_alpha();
      }
    }

    unsigned int find_bin(float prob) const {
      return _prline[prob]; 
    }

    float get_probability(unsigned int value) const {
      map<unsigned int, SamplingClass*>::const_iterator it = _bins.upper_bound(value);
      if (it == _bins.end()) {
        return _high_bin.get_probability();
      } else {
        return it->second->get_probability();
      }
    }

    SamplingClass * operator[](unsigned int value) const { 
      map<unsigned int, SamplingClass*>::const_iterator it = _bins.upper_bound(value);
      if (it == _bins.end()) {
        return const_cast<SamplingClass*>(&_high_bin);
      } else {
        return it->second;
      }
    }

    void next_epoch (unsigned int pktcount_thisepoch) {
      _prline.clear();
      for (map<unsigned int, SamplingClass*>::const_iterator it = _bins.begin();
          it != _bins.end(); ++it) {
        it->second->next_epoch(pktcount_thisepoch);
      } 
      _high_bin.next_epoch(pktcount_thisepoch);
    }

    bool verify() {
      cout << "verification" << endl;
      for (map<unsigned int, SamplingClass*>::const_iterator it = _bins.begin();
          it != _bins.end(); ++it) {
        cout << *(it->second) << endl;
      } 
      cout << _high_bin << endl;
      return true;
    }

    void print() {
      for (map<unsigned int, SamplingClass*>::const_iterator it = _bins.begin();
          it != _bins.end(); ++it) {
        cout << *(it->second) << endl;
      } 
      cout << _high_bin << endl;
    }
};

#endif // _SAMPLINGBINS_H
