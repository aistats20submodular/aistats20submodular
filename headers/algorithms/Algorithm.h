#pragma once

#include "movie_data.h"
#include "inputs/Knapsack.h"

template<typename T>
struct Algorithm {
  //virtual Result<T> solveBeforeRemovals(const vector<int>& elements, const Knapsack<T>& knapsack) const = 0;
  virtual Result<T> solve(const vector<T>& input, const Knapsack<T>& knapsack, double timeout) const = 0;

  virtual string name() const = 0;

  // virtual Result<T> finish(const Result<T>& input, const set<T>& exclude, const Knapsack<T> &knapsack, double lowerBound, double upperBound, double eps) const {
  //   (void)lowerBound; (void)upperBound; (void)eps;
  //   Result<T> res(input.set, input.oracleCalls, -1, vector<double>(knapsack.constraintNum));
  //   knapsack.filterResult(res, exclude);
  //   return res;
  // }

  // virtual ~Algorithm() { }
};