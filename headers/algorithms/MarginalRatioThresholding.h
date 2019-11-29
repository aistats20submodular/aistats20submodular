#pragma once

#include "movie_data.h"
#include "utils.h"
#include "algorithms/Algorithm.h"


template<typename T>
struct MarginalRatioThresholding : Algorithm<T> {
  double lowerBound;
  double upperBound;
  double eps;
  bool debug;

  MarginalRatioThresholding(double lowerBound, double upperBound, double eps) :
    lowerBound(lowerBound), upperBound(upperBound), eps(eps), debug(false) {
    assert(lowerBound > 1e-9);
    assert(!isinf(upperBound));
    assert(eps > 1e-9);
  }

  string name() const {
    return "MarginalThresholding";
  }

  /**
   * Here I tried to implement the most general version, with several constraints (just because it's simple).
   * The generalization is the following. For new incoming element:
   * * all constrains must be satisfied;
   * * for all constraints marginal-ratio threshold condition must hold.
   */
  Result<T> algorithmWithOptEstimation(double v, const vector<T> &input, const Knapsack<T> &knapsack) const {
    double alpha = 2. / 3;
    assert(v >= 0);
    int d = knapsack.constraintNum;
    assert(d == 1);
    Result<T> res(d);
    for (auto e : input) {
      vector<double> ce(d);
      for (int i = 0; i < d; ++i) {
        ce[i] = knapsack.c(i, e);
      }
      if (existsIndex(d, [&](int i) { return res.c[i] + ce[i] > knapsack.K[i]; })) {
        continue;
      }
      res.oracleCalls++;
      double difObj = knapsack.dif(res.set, e, res.objective);
      if (!existsIndex(d, [&](int i) {
        return difObj * (knapsack.K[i] - ce[i]) < ce[i] * (alpha * v - res.objective);
      })) {
        knapsack.addToResult(res, e, difObj);
      }
    }
    return res;
  }

  Result<T> solve(const vector<T> &input, const Knapsack<T> &knapsack) const {
    if (debug) cout << "v = " << lowerBound << ", ";
    cout.flush();
    auto res = algorithmWithOptEstimation(lowerBound, input, knapsack);
    for (double v = lowerBound * (1 + eps); v < upperBound * (1 + eps); v *= (1 + eps)) {
      if (debug) {
        cout << v << ", ";
        cout.flush();
      }
      auto newRes = algorithmWithOptEstimation(v, input, knapsack);
      auto totalOracleCalls = res.oracleCalls + newRes.oracleCalls;
      if (newRes.objective > res.objective) {
        res = newRes;
      }
      res.oracleCalls = totalOracleCalls;
    }
    if (debug) cout << endl;
    return res;
  }

  Result<T> solveBeforeRemovals(const vector<T> &input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack);
  }
};