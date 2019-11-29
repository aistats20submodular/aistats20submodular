#pragma once

#include "movie_data.h"
#include "utils.h"
#include "algorithms/Algorithm.h"


template<typename T>
struct MultiDimensional : Algorithm<T> {
  double lowerBound;
  double upperBound;
  double eps;

  MultiDimensional(double lowerBound, double upperBound, double eps) :
      lowerBound(lowerBound), upperBound(upperBound), eps(eps) {
    assert(lowerBound > 1e-9);
    assert(!isinf(upperBound));
    assert(eps > 1e-9);
  }

  string name() const {
    return "Multidimensional";
  }

  Result<T> algorithmWithOptEstimation(double v, const vector<T>& input, const Knapsack<T> &knapsack) const {
    double K = knapsack.K[0];
    for (double k : knapsack.K) {
      assert(K == k);
    }
    int d = knapsack.constraintNum;
    Result<T> res(d);
    for (const T& e : input) {
      vector<double> ce(d);
      for (int i = 0; i < d; ++i) {
        ce[i] = knapsack.c(i, e);
      }
      if (!existsIndex(d, [&](int i) { return ce[i] > 0; })) {
        res.oracleCalls++;
        double dif = knapsack.dif(res.set, e, res.objective);
        knapsack.addToResult(res, e, dif);
        continue;
      }
      if (existsIndex(d, [&](int i) { return ce[i] > res.c[i] + K; })) {
        continue;
      }

      vector<T> forObj = {e};
      res.oracleCalls++;
      double f = knapsack.f(forObj);
      if (existsIndex(d, [&](int i) { return ce[i] >= K / 2 && f * K * (2 * d + 1) >= ce[i] * 2 * v; })) {
        res.set.clear();
        res.set.emplace_back(e);
        res.objective = f;
        for (int i = 0; i < d; ++i) {
          res.c[i] = ce[i];
        }
        return res;
      }
      double dif = knapsack.dif(res.set, e, res.objective);
      res.oracleCalls++;
      if (!existsIndex(d, [&](int i) {
        return ce[i] + res.c[i] > K || dif * K * (2 * d + 1) < ce[i] * 2 * v;
      })) {
        knapsack.addToResult(res, e, dif);
      }
    }
    return res;
  }

  Result<T> solve(const vector<T>& input, const Knapsack<T> &knapsack, bool debug) const {
    if (debug) cout << "v = " << lowerBound << ", ";
    auto res = algorithmWithOptEstimation(lowerBound, input, knapsack);
    for (double v = lowerBound * (1 + eps); v < upperBound * (1 + eps); v *= (1 + eps)) {
      if (debug) cout << v << ": ";
      cout.flush();
      auto newRes = algorithmWithOptEstimation(v, input, knapsack);
      auto totalOracleCalls = res.oracleCalls + newRes.oracleCalls;
      if (debug) cout << newRes.objective << ", ";
      if (newRes.objective > res.objective) {
        res = newRes;
      }
      res.oracleCalls = totalOracleCalls;
    }
    if (debug) cout << endl;
    return res;
  }

  Result<T> solveBeforeRemovals(const vector<T>& input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack, true);
  }
};