#pragma once

#include "Graph.h"
#include "utils.h"
#include "inputs/Knapsack.h"
#include "debugUtils.h"
#include <random>


struct LinearKnapsack : Knapsack<int> {
  vector<double> values;
  vector<vector<double>> cost;
  const int n;

  
  
  void setCostsCostum(){
    MyDistribution distribution(UNIFORM, 0.1, 3); 
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        cost[c][i] = distribution.next();
      }
    }
  }

  void setEvenCosts(){
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        cost[c][i] = 1.0;
      }
    }
  }

  static LinearKnapsack* readLinearKnapsack(const vector<double>& KK, string path) // read from file
  {
    // printLine;
    vector<double> V;
    vector<double> costsPrime;
    string a, b;
    auto in = Graph::prepareStream(path);// from util
    // printLine;
    while (in >> a) {
      // printLine;
      in >> b;
      auto u = stod(a);
      auto v = stod(b);
      costsPrime.emplace_back(u);
      V.emplace_back(v);
      // debugPrint(a);
      // debugPrint(b);

    }
    in.close();
    // printLine;
    return new LinearKnapsack(KK, V, costsPrime);
  }



  LinearKnapsack(const vector<double>& K, const vector<double> &V, const vector<double> &c) :
                  Knapsack(K),
                  values(V),
                  n((int)V.size())
  {
    cost = vector<vector<double>>(K.size(), vector<double>(n));
    cost[0] = c;
    // setCostsCostum();
    // setEvenCosts();
  }

  double c(int i, const int &e) const override {
    return cost[i][e];
  }
  void reset(){
    // Do nothing
  }

  void checkMem() const {
    // Do nothing
  }


  double compute(vector<int> z) const {
    double agre = 0;
    for(int i=0; i < (int)z.size(); i++){
      agre += values[z[i]];
    }
    return agre;
  }

  double f(const vector<int> &z) const override {
    return compute(z);
  }
};