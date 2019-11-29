#pragma once

#include "utils.h"
#include "inputs/Knapsack.h"

struct NonrobustKiller : Knapsack<int> {
  vector<double> vals;

  NonrobustKiller(const vector<double> &K, int size) : Knapsack(K) {
    vals = vector<double>(size);
    for (int i = 0; i < size / 100; ++i) {
      vals[i] = 100;
    }
    for (int i = size / 100; i < size; ++i) {
      vals[i] = 1;
    }
    shuffle(vals.begin(), vals.end(), generator);
  }

  double c(int, const int &) const override {
    return 1;
  }

  double f(vector<int> &z) const override {
    double res = 0;
    for (int x : z) {
      res = max(res, vals[x]);
    }
    return res;
  }
};