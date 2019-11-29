#pragma once

#include "Graph.h"
#include "utils.h"
#include "inputs/Knapsack.h"
#include "debugUtils.h"
#include <random>


struct CoverageKnapsack : Knapsack<int> {
  const Graph graph;
  vector<vector<double>> cost;
  mutable unordered_map<int64, vector<int>> mem;
  mutable set<int64> was;
  mutable long totalSize;
  const int n;

  const int64 emptySetKey = 5;


  void setCosts(){
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        cost[c][i] = (double)Rand::nextRand() * 2 + 1;
      }
    }
  }


  void setCostsCostum(){
    MyDistribution distribution(GAMMA, 0.5, 0.5); // mean = 3 sd = 3
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        cost[c][i] = distribution.next();
      }
    }
  }

  void setRanDepCosts(){
    double eps = 0.0001;
    vector<int> temp(1);
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        temp[0] = i;
        cost[c][i] = ((double)Rand::nextRand())*(f(temp)-eps);
      }
    }
  }

  void setDetDepCosts(){
    
    vector<int> temp(1);
    for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        temp[0] = i;
        cost[c][i] = f(temp);
        // cost[c][i] = pow(cost[c][i],1);
        // cout<<"i="<<i<<" = > f = "<<cost[c][i]<<endl;
      }
    }
    // vector<int> temp(1);
    double test = cost[0][11];
    debugPrint(test);
    double mnValue =  getMinCost();
    debugPrint(mnValue);
    double coef = 20.0;
    double eps = mnValue/coef;
    double magnifier = coef/((coef-1)*mnValue);
    debugPrint(eps);
    // double magnifier = 1.0/eps;
    // eps = 0.000005;
    // magnifier = 1000.0;
       for (int c = 0; c < (int) K.size(); ++c) {
      for (int i = 0; i < n; ++i) {
        cost[c][i] = magnifier * (cost[c][i]-eps);
        cost[c][i] = pow(cost[c][i], 1);
        // cout<<"i="<<i<<" = > c = "<<cost[c][i]<<endl;
      }
    }
    // exit(0);
  }
  CoverageKnapsack(const vector<double> &K, const Graph &graph) : Knapsack(K), graph(graph), n((int)graph.vertices.size()) {
    cost = vector<vector<double>>(K.size(), vector<double>(n));
    // setCosts();
    // setCostsCostum();
    setDetDepCosts();
    //setRanDepCosts();
    mem[emptySetKey] = vector<int>();
    totalSize = 0;
  }

  double c(int i, const int &e) const override {
    return cost[i][e];
  }
  void reset() const override {
    mem.clear();
    mem[emptySetKey] = vector<int>();
    totalSize = 0;
  }

  void checkMem() const {
    if (totalSize > 4e9) {
      cerr << "clear ";
      mem.clear();
      mem[emptySetKey] = vector<int>();
      totalSize = 0;
    }
  }

  int bitNum(const vector<bool>& mask) const {
    int res = 0;
//#pragma omp parallel reduction (+:res)
    for (int i = 0; i < n; ++i) {
      if (mask[i]) {
        res ++;
      }
    }
    return res;
  }
  double compute(vector<int> z) const {
    // cout<<z[0]<<endl;
    set<int> all;
    for(int i = 0; i<(int)(z.size()); i++){
      all.insert(z[i]);
      const auto& edges = graph.vertices[z[i]].edges;
      for(int j = 0; j< (int)(edges.size()); j++){
        auto edge = edges[j];
        // cout<<edge<<endl;
        all.insert(edge);
      }
    }
    return all.size()/((double)n);
  }
//   double compute(vector<int> z, bool remember) const {
//     if (z.empty()) {
//       return 0;
//     }
//     int64 mul = 1343157;
//     // without last element
//     int64 prevKey = emptySetKey;
//     for (int i = 0; i < (int) z.size() - 1; ++i) {
//       prevKey = prevKey * mul + z[i];
//     }
//     int lastVertex = z[(int) z.size() - 1];
//     int64 key = prevKey * mul + lastVertex;
//     if (mem.count(key) > 0) {
//       return (double)mem[key].size() / n;
//     }
//     if (mem.count(prevKey) == 0) {
//       z.pop_back();
//       compute(z, true);
//       z.push_back(lastVertex);
//     }
// //    assert(mem.count(prevKey) > 0);
//     const auto& edges = graph.vertices[lastVertex].edges;
//     if (remember || was.count(key) > 0) {
//       was.erase(key);
//       auto newMask = sortedUnion(mem[prevKey], edges);
//       checkMem();
//       mem[key] = newMask;
//       int cnt = (int)newMask.size();
//       totalSize += cnt * 4 + 30;
//       assert(cnt <= n);
//       return (double)cnt / n;
//     } else {
//       was.insert(key);
//       return (double)unionSize(mem[prevKey], edges) / n;
//     }
//   }
  double getMinCost(){
    return *min_element(cost[0].begin(), cost[0].end());
  }
  double f(const vector<int> &z) const{
    return pow(compute(z), 1);
  }
};