#pragma once

#include "utils.h"
#include "inputs/Knapsack.h"

template<typename T>
struct DelegateKnapsack : Knapsack<T> {
  const Knapsack<T>& delegate;
  DelegateKnapsack(const vector<double> &K, const Knapsack<T>& delegate) : Knapsack<T>(K), delegate(delegate) {}

  double c(int i, const T &e) const {
    return delegate.c(i, e);
  }

  double f(vector<T> & vec) const {
    return delegate.f(vec);
  }
};