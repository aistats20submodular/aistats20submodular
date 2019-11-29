#pragma once

#include "algorithms/Algorithm.h"

template<typename T>
struct Greedy : Algorithm<T> {
  string name() const {
    return "Greedy";
  }

  Result<T> solve(const vector<T>& input, const Knapsack<T> &knapsack) const {
    assert(knapsack.constraintNum == 1);
    vector<T> elements(input);
    Result<T> res(knapsack.constraintNum);
    //int it = 0; Unused?
    while (true) {
      int i = 0;
      int maxIndex = -1;
      double maxDif = 0, maxC = 0;
 //     if (debug && it++ % 10 == 0) {
 //       cout << elements.size() << " ";
 //       cout.flush();
 //     }
      while (i < (int)elements.size()) {
        const T &e = elements[i];
        double c = knapsack.c(0, e);
        if (c + res.c[0] > knapsack.K[0]) {
          elements[i] = elements[elements.size() - 1];
          elements.pop_back();
        } else {
          res.oracleCalls++;
          double dif = knapsack.dif(res.set, e, res.objective);
          if (maxIndex == -1 || (dif * maxC > maxDif * c)) {
            maxDif = dif;
            maxC = c;
            maxIndex = i;
          }
          i++;
        }
      }
      if (maxIndex == -1) {
        break;
      }
      //printf("\nAdding index=%d to knapsack\n", maxIndex);
      knapsack.addToResult(res, elements[maxIndex], maxDif);
//      cerr << knapsack.f(res.set) << " " << res.objective << endl;
//      assert(eq(knapsack.f(res.set), res.objective));
      elements[maxIndex] = elements[elements.size() - 1];
      elements.pop_back();
    }
//    if (debug) {
//      cout << endl;
//    }
    return res;
  }

  Result<T> solveBeforeRemovals(const vector<T>& input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack);
  }
};

