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
struct Distributed : Algorithm<T> {
  double lowerBound;
  double upperBound;
  double eps;

  Distributed(double lowerBound, double upperBound, double eps) :
      lowerBound(lowerBound), upperBound(upperBound), eps(eps) {
    assert(lowerBound > 1e-9);
    assert(!isinf(upperBound));
    assert(eps > 1e-9);
  }

  string name() const {
    std::ostringstream stringStream;
    stringStream << "Distributed_"
                 << lowerBound << "_"
                 << upperBound << "_"
                 << eps;
    return stringStream.str();
  }

  vector<vector<T>> getPartition(vector<T> elements, int parts) const {
    vector<int> splits;
    for (int i = 0; i <= parts; i++) {
      splits.emplace_back((int64) i * elements.size() / parts);
    }
    shuffle(elements.begin(), elements.end(), generator);
    vector<vector<T>> partition(parts);
    for (int i = 0; i < parts; i++) {
      for (int j = splits[i]; j < splits[i + 1]; j++) {
        partition[i].emplace_back(elements[j]);
      }
    }
    return partition;
  }

  Result<T> getSummary(const vector<T> &input, const Knapsack<T> &knapsack) const {
    int oracleCalls = 0;
    double K = knapsack.K[0]; // knapsack capacity
    int machineCount = (int) ceil(sqrt(input.size() / K));
    cerr << "machineCount: " << machineCount << endl;

    Result<T> summary(1);
    auto elements = input;

    for (double t = upperBound; t > lowerBound; t /= (1 + eps)) {
      knapsack.reset();
      double maxDensity = 0;

      /// Add only the elements with high enough threshold
      auto addElementWithHighThreshold = [&oracleCalls, &knapsack, &maxDensity, t, K]
          (const Result<T> &prevResult, const vector<T> &set) {
        Result<T> newResult(prevResult.set, 0, prevResult.objective, prevResult.c);
        for (const T &e: set) {
          if (knapsack.c(0,e) + newResult.c[0] > K) {
            continue;
          }
          auto dif = (oracleCalls++, knapsack.dif(newResult.set, e, newResult.objective));
          auto density = dif / knapsack.c(0, e);
          maxDensity = max(maxDensity, density);
          if (density > t) {
            knapsack.addToResult(newResult, e, dif);
          }
        }
        return newResult;
      };

      // Sample Gamma
      vector<T> gamma;

      double sampleProb = 4 * sqrt(K / input.size());
      for (const T &e: input) {
        if (Rand::check(sampleProb)) {
          gamma.emplace_back(e);
        }
      }

      Result<T> gammaResult = addElementWithHighThreshold(summary, gamma);
      if (gammaResult.set.size() > summary.set.size() + K) {
        summary = gammaResult;
        cerr << "Summary size: " << summary.set.size() << endl;
        continue;
      }

      // Items to exclude from each machine
      set < T > toExclude;
      for (const T &e: gammaResult.set) {
        toExclude.insert(e);
      }

      vector<T> collectedItems;
      for (const auto &part: getPartition(input, machineCount)) {
        Result<T> partResult = addElementWithHighThreshold(summary, part);
        knapsack.filterResult(partResult, toExclude);
        for (const T &e : partResult.set) {
          collectedItems.emplace_back(e);
        }
      }
      summary = addElementWithHighThreshold(summary, collectedItems);
      if (maxDensity < t) {
        t = maxDensity * (1 + 0.99 * eps);
      }
      cerr << "Summary size: " << summary.set.size() << endl;
    }
    summary.oracleCalls = oracleCalls;
    return summary;
  }

  Result<T> bestGreedySolution(const vector<T> &input, const Knapsack<T> &knapsack, const Result<T> summary) const {
    double K = knapsack.K[0]; // knapsack capacity
    int machineCount = (int) ceil(sqrt(input.size() / K));

    int oracleCalls = summary.oracleCalls;
    double bestAugObj = 0;
    Result<T> bestAugSolution(1);
    auto partition = getPartition(input, machineCount);

    for (auto &part: partition) {
      sort(part.begin(), part.end(), [&](const T &a, const T &b) { return knapsack.c(0, a) < knapsack.c(0, b); });
    }
    auto localSummary = summary.set;
    Result<T> greedySolution(1);
    while (true) {// Greedy loop:
      for (const auto &part: partition) {
        // Augmented solution
        int augIndex = -1;
        double augMaxDif = 0;
        for (int i = 0; i < (int) part.size(); i++) {
          const T &e = part[i];
          if (knapsack.c(0, e) + greedySolution.c[0] > K) {
            break;
          }
          double dif = (oracleCalls++, knapsack.dif(greedySolution.set, e, greedySolution.objective));
          if (augIndex == -1 || dif > augMaxDif) {// if this is better in terms of value
            augMaxDif = dif;
            augIndex = i;
          }
        }
        if (augIndex != -1 && augMaxDif + greedySolution.objective > bestAugObj) {
          bestAugSolution = greedySolution; //This is supposed to copy.
          knapsack.addToResult(bestAugSolution, part[augIndex], augMaxDif);
        }
      }

      // Greedy solution
      int i = 0;
      int maxIndex = -1;
      double maxDif = 0, maxC = 0;
      while (i < (int) localSummary.size()) {//Find highest density
        const T &e = localSummary[i];
        double c = knapsack.c(0, e);

        if (c + greedySolution.c[0] > K) {// if there is no capacity left for this item, remove it.
          localSummary[i] = localSummary[localSummary.size() - 1];
          localSummary.pop_back();
        } else {
          double dif = (oracleCalls++, knapsack.dif(greedySolution.set, e, greedySolution.objective));
          //Greedy solution
          if (maxIndex == -1 || dif * maxC > maxDif * c) {// if dif/c > maxDif/maxC, if this is better in terms of value
            maxDif = dif;
            maxC = c;
            maxIndex = i;
          }
          i++;
        }
      }
      if (maxIndex == -1) {// There is no item smaller than remaining capacity left
        break;
      }

      knapsack.addToResult(greedySolution, localSummary[maxIndex], maxDif);
      localSummary[maxIndex] = localSummary[localSummary.size() - 1];
      localSummary.pop_back();
    }
    if (greedySolution.objective > bestAugSolution.objective) {
      bestAugSolution = greedySolution;
    }

    bestAugSolution.oracleCalls = oracleCalls;
    return bestAugSolution;
  }

  Result<T> solve(const vector<T> &input, const Knapsack<T> &knapsack, double timeout) const {
    (void) (timeout);
    assert(knapsack.constraintNum == 1);

    auto summary = getSummary(input, knapsack);
    return bestGreedySolution(input, knapsack, summary);
  }
};