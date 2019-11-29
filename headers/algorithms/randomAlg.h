#pragma once

#include "algorithms/Algorithm.h"
#include "utils.h"
#include "debugUtils.h"

template<typename T>
struct RandomAlg : Algorithm<T> {
  string name() const{
    std::ostringstream stringStream;
    stringStream<<"RandomAlg";
    return stringStream.str();
  }

  RandomAlg(){
	  cout << "RandomAlg initiated!" << endl;
  }

  Result<T> solve(const vector<T>& input, const Knapsack<T> &knapsack) const {//Working on this.
    assert(knapsack.constraintNum == 1);
	vector<T> elements(input);
	vector<T> solution;
    double K = knapsack.K[0];//napsack capacity
	int oracleCalls = 0;
	double p = 3 * K / input.size();
	double costTillNow = 0;
	for (int itemInd = 0; itemInd < (int)elements.size(); itemInd++){
		double randValue = (double)Rand::nextRand();
		if (randValue < p){
			double newCost = knapsack.c(0, elements[itemInd]);
			if (costTillNow + newCost < K){
				costTillNow += newCost;
				solution.emplace_back(elements[itemInd]);
			}
		}
	}
	oracleCalls++;
	double value = knapsack.f(solution);
	Result<T> finalResult(solution, oracleCalls, value, vector<double>(1, costTillNow));
	return finalResult;
  }

  Result<T> solveBeforeRemovals(const vector<T>& input, const Knapsack<T> &knapsack) const {
    return solve(input, knapsack);
  }
};

