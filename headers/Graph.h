#pragma once

#include <unordered_set>
#include "utils.h"


struct Vertex {
  int id; // vertex index
  int degree; // degree
  vector<int> edges; // list of neighbors
  vector<double> c; // Cost function
};

const static int BufSize = 10 * 1000 * 1000;
char static buffer[BufSize];

struct Graph {

  inline static int getOrAdd(const int64& u, unordered_map<int64, int>& map) {
    auto p = map.find(u);
    if (p == map.end()) {
      auto size = static_cast<int>(map.size());
      map[u] = size;
      return size;
    } else {
      return p->second;
    }
  }

  unordered_map<int, int64> map;
  int n;
  vector<Vertex> vertices;
  vector<double> imbalance;
  int edgeCount;
  Graph():Graph(unordered_map<int64, int>(),vector<tuple<int, int>>()) {}
  Graph(unordered_map<int64, int> map, const vector<tuple<int, int>>& e) : n((int)map.size()) {
    for (auto p : map) {
      this->map[p.second] = p.first;
    }
    edgeCount = (int)e.size();
    vertices = vector<Vertex>(n);
    for (int i = 0; i < n; ++i) {
      vertices[i].id = i;
    }
    for (auto p : e) {
      int u, v;
      tie(u, v) = p;
      vertices[u].edges.push_back(v);
      vertices[v].edges.push_back(u);
    }
    int sum = 0;
    for (auto& v : vertices) {
      removeDuplicates(v.edges);
      v.degree = (int)v.edges.size();
      sum += v.degree;
    }
    cerr << "Graph is read. " << n << " vertices, " << sum / 2 << " edges." << endl;
  }

  static ifstream prepareStream(const string &path) {
    ifstream in;
    std::ios::sync_with_stdio(false);
    in.open(path);
    in.rdbuf()->pubsetbuf(buffer, BufSize);
    return in;
  }

  static int64 toInt64(const string& s) {
    if (s.length() < 20) {
      return (int64)stoull(s);
    } else {
      return (int64)stoull(s.substr(s.length() - 19));
    }
  }

  static Graph read(const string &path) {
    string a, b;
    auto in = prepareStream(path);
    unordered_map<int64, int> map;
    vector<tuple<int, int>> e;
    //cout<<"before while loop"<<endl;
    while (in >> a) {
      in >> b;
      //cout<<"Inside whileLoop a="<<a<<" and b="<<b<<endl;
      int u = getOrAdd(toInt64(a), map);
      int v = getOrAdd(toInt64(b), map);
      if (u == v) {
        continue;
      }
      e.emplace_back(u, v);
    }
    in.close();
    Graph res = Graph(map, e);
    return res;
  }
};
