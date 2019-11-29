#pragma once

#include "movie_data.h"
#include "utils.h"
#include "algorithms/Algorithm.h"

/// Algorithm 4 from http://drops.dagstuhl.de/opus/volltexte/2017/7560/pdf/LIPIcs-APPROX-RANDOM-2017-11.pdf
template<typename T>
struct BranchingMRT : Algorithm<T> {
  double lowerBound;
  double upperBound;
  double eps;
  bool debug;

  BranchingMRT(double lowerBound, double upperBound, double eps) :
      lowerBound(lowerBound), upperBound(upperBound), eps(eps), debug(false) {
    assert(lowerBound > 1e-9);
    assert(!isinf(upperBound));
    assert(eps > 1e-9);
  }

  string name() const {
    std::ostringstream stringStream;
    stringStream << "BranchingMRT_"
                 << lowerBound << "_"
                 << upperBound << "_"
                 << eps;
    return stringStream.str();
  }

  Result<T> algorithmWithOptEstimation(double v, double c1,
                                       const vector<T> &input, const Knapsack<T> &knapsack,
                                       const vector<double> &costs) const {
    double alpha = 1. / (2 - 2 * c1);
    double b = min(c1 * (1 + eps), 0.5);
    double lam = v * alpha * (1 - b) / 2;
    double K = knapsack.K[0];
    assert(v >= 0);
    assert(knapsack.constraintNum == 1);
    Result<T> res(1);
    int i = 0;
    for (; i < (int) input.size(); i++) {
      const T &e = input[i];
      double ce = costs[i];
//      assert(ce == knapsack.c(0, e));
      if (ce + res.c[0] > K || ce > b * K) {
        continue;
      }
      double difObj = (res.oracleCalls++, knapsack.dif(res.set, e, res.objective));
      if (difObj * (knapsack.K[i] - res.c[0]) < ce * (alpha * v - res.objective)) {
        knapsack.addToResult(res, e, difObj);
      }
      if (res.objective > lam) {
        break;
      }
    }
    vector<T> s0prime_vec =
        res.c[0] >= (1 - b) * K
        ? vector<T>({last(res.set)})
        : res.set;
    auto s0primeCost = vector_sum<double>(s0prime_vec, [&](const T &e) { return knapsack.c(0, e); });
    Result<T> s0prime(s0prime_vec, 0.0, knapsack.f(s0prime_vec), {s0primeCost});
    Result<T> sprime = s0prime;
    for (; i < (int) input.size(); i++) {
      const T &e = input[i];
      double ce = costs[i];
//      assert(ce == knapsack.c(0, e));
      if (ce > b * K || (s0prime.c[0] + ce > K && ce + sprime.c[0] > K)) {
        continue;
      }
      double difObj = (res.oracleCalls++, knapsack.dif(res.set, e, res.objective));
      if (difObj * (knapsack.K[0] - ce) < ce * (alpha * v - res.objective)) {
        if (ce + sprime.c[0] <= K) {
          knapsack.addToResult(sprime, e, difObj);
          continue;
        }
        double difObjS = (res.oracleCalls++, knapsack.dif(s0prime_vec, e, s0prime.objective));
        if (sprime.objective < s0prime.objective + difObjS) {
          sprime = s0prime;
          knapsack.addToResult(sprime, e, difObjS);
        }
      }

    }
    sprime.oracleCalls = res.oracleCalls;
    return (sprime.objective > res.objective) ? sprime : res;
  }

  Result<T> solve(const vector<T> &input, const Knapsack<T> &knapsack, double timeout) const {
    auto startTime = std::chrono::system_clock::now();
    if (debug) cout << "v = " << lowerBound << ", ";
    auto costs = vector_map<double>(input, [&](int e) { return knapsack.c(0, e); });
    cout.flush();
    Result<T> res(1);
    double K = knapsack.K[0];
    for (int c1 = 1; c1 <= K; c1++) {
      cerr << c1 << " ";
      for (double v = lowerBound; v < upperBound * (1 + eps); v *= (1 + eps)) {
        if (debug) {
          cout << v << ", ";
          cout.flush();
        }
        auto newRes = algorithmWithOptEstimation(v, c1 / K, input, knapsack, costs);
        auto totalOracleCalls = res.oracleCalls + newRes.oracleCalls;
        if (newRes.objective > res.objective) {
          res = newRes;
        }
        res.oracleCalls = totalOracleCalls;
        if (!hasTime(startTime, timeout)) {
          res.objective = -1;
          return res;
        }
        auto isEmpty = newRes.set.empty();
        double p = 2;
        newRes = localMRT(v, c1 / K, p, input, knapsack, costs);
        totalOracleCalls = res.oracleCalls + newRes.oracleCalls;
        if (newRes.objective > res.objective) {
          res = newRes;
        }
        res.oracleCalls = totalOracleCalls;
        if (isEmpty && newRes.set.empty()) {
          break;
        }
      }
    }
    T bestElem;
    double bestObj = res.objective;
    for (const T &e : input) {
      if (knapsack.c(0, e) > K) {
        continue;
      }
      double obj = (res.oracleCalls++, knapsack.f({e}));
      if (obj > bestObj) {
        bestObj = obj;
        bestElem = e;
      }
    }
    return (bestObj == res.objective) ? res : Result<T>({bestElem}, res.oracleCalls, bestObj,
                                                        {knapsack.c(0, bestElem)});

  }

  Result<T> localMRT(double v, double c1, double p, const vector<T> &input, const Knapsack<T> &knapsack,
                     const vector<double> &costs) const {
    double alpha = 1. / (p + 1 - c1);
    assert(v >= 0);
    Result<T> res(1);
    for (int i = 0; i < (int) input.size(); i++) {
      const T &e = input[i];
      double ce = costs[i];
      if (res.c[0] + ce > knapsack.K[0]) {
        continue;
      }
      double difObj = (res.oracleCalls++, knapsack.dif(res.set, e, res.objective));
      if (difObj * (p * knapsack.K[0] - res.c[0]) < ce * (alpha * p * v - res.objective)) {
        knapsack.addToResult(res, e, difObj);
      }
    }
    return res;
  }

  Result<T> solveBeforeRemovals(const vector<T> &input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack);
  }
};