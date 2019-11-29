#pragma once

#include "algorithms/Algorithm.h"
#include "utils.h"
#include "debugUtils.h"
#include "Mode.h"

template<typename T>
struct GreedyPP : Algorithm<T> {
  Mode mode;
  int preGreedySize;

  string name() const {
    std::ostringstream stringStream;
    stringStream << "Greedy" << modeName[mode] << "_" << preGreedySize;
    return stringStream.str();
  }

  GreedyPP(int preGreedySize, Mode mode) : mode(mode), preGreedySize(preGreedySize) {}

  Result<T> rec(vector<T> &input, const Knapsack<T> &knapsack, vector<T> &preGreedy, double greedyCost, double timeout) const {
    auto startTime = std::chrono::system_clock::now();
    if ((int)preGreedy.size() == preGreedySize) {
      return finalSolve(input, knapsack, preGreedy, greedyCost);
    }
    Result<T> best({}, -1, 0, {0});
    double K = knapsack.K[0]; // Knapsack capacity
    int lastI = input.size() - 1;
    for (int i = 0; i < (int)input.size(); i++) {
      if (!hasTime(startTime, timeout)) {
        best.objective = -1;
        return best;
      }
      auto e = input[i];
      double cost = knapsack.c(0, e);
      if (cost + greedyCost > K) {
        continue;
      }
      swap(input[i], input[lastI]);
      input.pop_back();
      preGreedy.push_back(e);
      auto sol = rec(input, knapsack, preGreedy, greedyCost + cost, timeout);
      preGreedy.pop_back();
      input.push_back(e);
      auto totalOracleCalls = sol.oracleCalls + best.oracleCalls;
      if (sol.objective > best.objective) {
        best = sol;
      }
      best.oracleCalls = totalOracleCalls;
      swap(input[i], input[lastI]);
    }
    return best;
  }

  Result<T> finalSolve(const vector<T> &input, const Knapsack<T> &knapsack, const vector<T> &preGreedy, double greedyCost) const {
    vector<T> bestAugSolution;
    auto elements(input);
    auto greedySolution(preGreedy);
    double bestAugObj = 0;
    double bestAugCost = 0;
    double K = knapsack.K[0]; // Knapsack capacity

    T maxElement;
    double maxObj = -1;
    int oracleCalls = 0;
    double greedyObj = (oracleCalls++, knapsack.f(greedySolution));

    while (true) {// Greedy loop:
      int i = 0;
      int maxIndex = -1;
      double maxDif = 0, maxC = 0;


      int augIndex = -1;
      double augMaxDif = 0, augMaxC = 0;

      while (i < (int) elements.size()) {//Find highest density
        const T &e = elements[i];
        double c = knapsack.c(0, e);

        if (c + greedyCost > K) {// if there is no capacity left for this item, remove it.
          elements[i] = elements[elements.size() - 1];
          elements.pop_back();
        } else {
          double dif = (oracleCalls++, knapsack.dif(greedySolution, e, greedyObj));
          //Augmented solution:
          if ((mode == PLUS_MAX && (augIndex == -1 || (dif > augMaxDif)))
              || (mode == OR_MAX && greedySolution.size() == 0u)) {// if this is better in terms of value
            augMaxDif = dif;
            augMaxC = c;
            augIndex = i;
          }
          //Greedy solution
          if (maxIndex == -1 ||
              (dif * maxC > maxDif * c)) {// if dif/c > maxDif/maxC, if this is better in terms of value
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
      if (mode == PLUS_MAX && augMaxDif + greedyObj > bestAugObj) {
        bestAugSolution = greedySolution; //This is supposed to copy.
        bestAugSolution.emplace_back(elements[augIndex]);
        bestAugObj = greedyObj + augMaxDif;
        bestAugCost = greedyCost + augMaxC;
      }
      if (mode == OR_MAX && greedySolution.size() == 0u) {
        maxElement = elements[augIndex];
        maxObj = augMaxDif;
      }

      //printf("\nAdding index=%d to knapsack\n", maxIndex);
      greedySolution.emplace_back(elements[maxIndex]);
      greedyCost += maxC;
      greedyObj += maxDif;
      //      cerr << knapsack.f(res.set) << " " << res.objective << endl;
      //      assert(eq(knapsack.f(res.set), res.objective));
      elements[maxIndex] = elements[elements.size() - 1];
      elements.pop_back();
    }
    switch (mode) {
      case PLUS_MAX: {
        return Result<T>(bestAugSolution, oracleCalls, bestAugObj, {bestAugCost});
      }
      case OR_MAX: {
        if (maxObj > greedyObj) {
          return Result<T>({maxElement}, oracleCalls, maxObj, {knapsack.c(0, maxElement)});
        } else {
          return Result<T>(greedySolution, oracleCalls, greedyObj, {greedyCost});
        }
      }
      case NONE: {
        return Result<T>(greedySolution, oracleCalls, greedyObj, {greedyCost});
      }
    }
  }

  Result<T> solve(const vector<T> &input, const Knapsack<T> &knapsack, double timeout) const {//Working on this.
    // cout << "solving" << endl;
    assert(knapsack.constraintNum == 1);
    auto copy(input);
    vector<T> preGreedy;
    return rec(copy, knapsack, preGreedy, 0, timeout);
  }

  Result<T> solveBeforeRemovals(const vector<T> &input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack);
  }
};

