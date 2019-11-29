#pragma once

#include <algorithm>
#include <iostream>

#include <vector>


#include "movie_data.h"
#include "utils.h"
#include "algorithms/Algorithm.h"
#include "debugUtils.h"
#include "Mode.h"

template<typename T>
struct SievePP : Algorithm<T> {
  double lowerBound;
  double upperBound;
  double eps;
  Mode mode;

  SievePP(double lowerBound, double upperBound, double eps, Mode mode) :
      lowerBound(lowerBound), upperBound(upperBound), eps(eps), mode(mode) {
    assert(lowerBound > 1e-9);
    assert(!isinf(upperBound));
    assert(eps > 1e-9);
  }

  string name() const {
    std::ostringstream stringStream;
    stringStream << "Sieve" << modeName[mode] << "_"
                 << lowerBound << "_"
                 << upperBound << "_"
                 << eps;
    return stringStream.str();
  }

  /**
   * Here I tried to implement the most general version, with several constraints (just because it's simple).
   * The generalization is the following. For new incoming element:
   * * all constrains must be satisfied;
   * * for all constraints marginal-ratio threshold condition must hold.
   */


  Result<T> solve(const vector<T> &input, const Knapsack<T> &knapsack, double timeout) const {
    (void)(timeout);
    assert(knapsack.constraintNum == 1);
    int oracleCalls = 0;
    double K = knapsack.K[0]; // knapsack capacity

    vector<T> elements(input);
    double t = upperBound; // Threshold
    vector<T> sieveSolution;
    double sieveCost = 0;
    double sieveObj = 0;

    while (t > lowerBound && !elements.empty()) {
      // int ss1 = (int)elements.size();
      // int ss2 = (int)sieveSolution.size();
      // cout<<"before while"<<endl;
      // debugPrint(t);
      // debugPrint(ss1);
      // debugPrint(ss2);
      int ind = 0;
      double maxDensity = -1;
      while (ind < (int) elements.size()) {// make a new pass on data with new threshold
        const T &e = elements[ind];
        double newItemCost = knapsack.c(0, e);
        if (newItemCost + sieveCost > K) {// if there is no capacity left for this item, remove it.
          elements[ind] = elements[elements.size() - 1];
          elements.pop_back(); // we do not increment ind in this case
        } else {// If we have enough space for this item, investigate its marginal gain
          oracleCalls++;
          double marginalGain = knapsack.dif(sieveSolution, e, sieveObj);
          // If the density of this item is higher than the current threshold
          if (marginalGain > t * newItemCost) {
            // Add it to the solution
            sieveSolution.emplace_back(e);
            sieveCost += newItemCost;
            sieveObj += marginalGain;
            // Remove the element
            elements[ind] = elements[elements.size() - 1];
            elements.pop_back(); // we do not increment ind in this case
          }
          if (marginalGain > maxDensity * newItemCost) {// making sure we adopt the threshold with maxGain
            maxDensity = marginalGain / newItemCost;
          }
          ind++;
        }
      }
      // debugPrint(maxDensity);
      // debugPrint(t);
      t /= (1 + eps);
      // debugPrint(t);
      if (maxDensity < t) {// making sure we dont repeat empty passes
        t = maxDensity - eps / 1000.0;
      }
    }
    // exit(0);
    switch (mode) {
      case NONE: {
        Result<T> finalResult(sieveSolution, oracleCalls, sieveObj, vector<double>(1, sieveCost));
        return finalResult;
      }
      case PLUS_MAX: {
        vector<T> bestAugSolution(sieveSolution);
        double bestCost = sieveCost;
        double bestObj = sieveObj;
        vector<double> costs;
        costs.emplace_back(0);
        vector<T> allElements(input);
        for (int indCost = 0; indCost < (int) sieveSolution.size(); indCost++) {
          costs.emplace_back(costs[indCost] + knapsack.c(0, sieveSolution[indCost]));
          // printf("indCost=%d, sieveSolution[indCost]=%d, costs[indCost+1]=%f\n", indCost,sieveSolution[indCost], costs[indCost+1]);
        }

        for (const auto &elem: allElements) {
          double thisCost = knapsack.c(0, elem);
          if (K < thisCost)
            continue;
          // The size of the prefix we can augment to
          auto upper = upper_bound(costs.begin(), costs.end(), K - thisCost) - costs.begin() - 1;

          vector<T> newSolution;
          newSolution.reserve(upper + 1);
          for (int i = 0; i < upper; ++i) {
            newSolution.emplace_back(sieveSolution[i]);
          }
          newSolution.emplace_back(elem);
          oracleCalls++;
          double newObj = knapsack.f(newSolution);
          if (bestObj < newObj) {
            bestAugSolution = newSolution;
            bestObj = newObj;
            bestCost = thisCost + costs[upper];
          }
        }

        Result<T> finalResult(bestAugSolution, oracleCalls, bestObj, vector<double>(1, bestCost));
        return finalResult;
      }
      case OR_MAX: {
        vector<T> bestSolution(sieveSolution);
        double bestCost = sieveCost;
        double bestObj = sieveObj;

        for (const auto &elem: input) {
          double thisCost = knapsack.c(0, elem);
          if (K < thisCost)
            continue;
          // The size of the prefix we can augment to
          vector<T> newSolution(1, elem);
          oracleCalls++;
          double newObj = knapsack.f(newSolution);
          if (bestObj < newObj) {
            bestSolution = newSolution;
            bestObj = newObj;
            bestCost = thisCost;
          }
        }

        Result<T> finalResult(bestSolution, oracleCalls, bestObj, vector<double>(1, bestCost));
        return finalResult;
      }
    }
  }
};