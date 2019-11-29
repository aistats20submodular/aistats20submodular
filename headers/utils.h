#pragma once

#include <random>
#include <cassert>
#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <set>
#include <unordered_map>
#include <sstream>
#include <map>
#include <algorithm>

typedef long long int int64;

using namespace std;

extern default_random_engine generator;

struct Rand {
    static double nextRand();
    static unsigned int next(unsigned long bound);
    static bool check(double p);
};

inline bool fileExist(const std::string &name);

void measureTime(const string& message, const std::function<void()> &f);

template<typename T>
T measureTime(const string& message, const std::function<T()> &f) {
  cout << "Start " << message << endl;
  auto startTime = std::chrono::system_clock::now();
  T res = f();
  cout << message << " time: " << (std::chrono::duration<double>(std::chrono::system_clock::now() - startTime)).count() << endl;
  return res;
}

void createDir(const string& dir);

inline int sign(double x);

/**
 * Read file and apply function f to each line.
 */
void readFile(const string& path, const std::function<void (const string)>& f);

vector<string> split(const string& str, char delimiter);
void split(const string& str, char delimiter, vector<string>& res);

/**
 * Read csv file.
 */
template <typename T>
vector<T> readCsv(const string& path, const function<T (const vector<string>&)>& parser) {
  bool header = true;
  vector<T> res;
  readFile(path, [&](const string s) {
    if (!header) {
      vector<string> splitted;
      if (isspace(s[s.length() - 1])) { // rtrim the string
        int last = s.length() - 2;
        while (isspace(s[last])) {
          last--;
        }
        splitted = split(s.substr(0, last + 1u), ',');
      } else {
        splitted = split(s, ',');
      }
      res.emplace_back(parser(splitted));
    } else {
      header = false;
    }
  });
  return res;
}

/**
 * Check whether the number in range 0..n-1 exists, such that the predicate is true.
 */
bool existsIndex(int n, const function<bool (int)>& pred);

template <typename T, typename S, class F>
vector<T> vector_map(const vector<S>& source, const F& f) {
  vector<T> res;
  res.reserve(source.size());
  for (auto x : source) {
    res.emplace_back(f(x));
  }
  return res;
}

template<typename T>
inline T vector_sum_id(const vector<T>& vec) {
  T res = 0;
  for (const T& x : vec) {
    res += x;
  }
  return res;
}

template <typename T, typename S, class F>
inline T vector_sum(const vector<S>& vec, const F& f) {
  T res = 0;
  for (const S& x : vec) {
    res += f(x);
  }
  return res;
}

template<typename T>
inline void removeDuplicates(vector<T>& vec) {
  sort(vec.begin(), vec.end());
  vec.erase(unique(vec.begin(), vec.end()), vec.end());
}

template<typename T>
inline void addRange(vector<T>& to, const vector<T>& from) {
  for (const T& x : from) {
    to.emplace_back(x);
  }
}

template<typename T>
inline vector<T> filter(const vector<T>& vec, const function<bool (const T&)>& pred) {
  vector<T> res;
  for (const T& x : vec) {
    if (pred(x)) {
      res.emplace_back(x);
    }
  }
  return res;
}

template<typename T>
inline string vectorToStr(const vector<T>& vec, const string& join) {
  stringstream ss;
  for (int i = 0; i < (int) vec.size(); ++i) {
    if (i != 0) {
      ss << join;
    }
    ss << vec[i];
  }
  return ss.str();
}

vector<int> range(int n);

inline bool eq(double a, double b) {
  return abs(a - b) < 1e-9;
}

template <typename T>
vector<T> sortedUnion(const vector<T>& a, const vector<T>& b) {
/*
  for (int i = 1; i < (int)a.size(); ++i) {
    assert(a[i] > a[i - 1]);
  }
  for (int i = 1; i < (int)b.size(); ++i) {
    assert(b[i] > b[i - 1]);
  }
*/
  unsigned int i = 0;
  vector<T> res;
  res.reserve(a.size() + b.size());
  for (const T& x : a) {
    while (i < b.size() && b[i] < x) {
      res.push_back(b[i]);
      i++;
    }
    if (i < b.size() && b[i] == x) {
      i++;
    }
    res.push_back(x);
  }
  while (i < b.size()) {
    res.push_back(b[i]);
    i++;
  }
/*
  cout << "a = " << vectorToStr(a, ",") << endl;
  cout << "b = " << vectorToStr(b, ",") << endl;
  cout << "res = " << vectorToStr(res, ",") << endl;
  for (int i = 1; i < (int)res.size(); ++i) {
    assert(res[i] > res[i - 1]);
  }
*/
  return res;
};

template <typename T>
int unionSize(const vector<T>& a, const vector<T>& b) {
/*
  for (int i = 1; i < (int)a.size(); ++i) {
    assert(a[i] > a[i - 1]);
  }
  for (int i = 1; i < (int)b.size(); ++i) {
    assert(b[i] > b[i - 1]);
  }
*/
  int res = 0;
  unsigned int i = 0;
  for (const T& x : a) {
    while (i < b.size() && b[i] < x) {
      res ++;
      i++;
    }
    if (i < b.size() && b[i] == x) {
      i++;
    }
    res ++;
  }
  res += b.size() - i;
  return res;
}

template <typename T, class F>
void sortBy(vector<T>& vec, const F& f) {
  sort(vec.begin(), vec.end(), [&f](const T& a, const T& b) { return f(a) < f(b); } );
}

template<typename T>
T last(const vector<T>& a) {
  assert(a.size() > 0);
  return a[a.size() - 1];
}

enum DistEnum{
  UNIFORM = 0,
  EXP = 1,
  GAMMA = 2,
  EXT = 3
};

struct MyDistribution{
  DistEnum disttype;
  random_device rd;// So everytime it will be different
  default_random_engine generator{rd()};

  uniform_real_distribution<double>* unifDist;
  exponential_distribution<double>* expDist;
  gamma_distribution<double>* gammaDist;
  extreme_value_distribution<double>* extDist;

  MyDistribution(DistEnum disttype, double firstArg=1.0, double secondArg=3.0): disttype(disttype){
    switch(disttype){
      case UNIFORM:
        unifDist = new uniform_real_distribution<double>(firstArg, secondArg);
      break;
      case EXP:
        expDist = new exponential_distribution<double>(firstArg);// this is lambda to make expectation be around 3
        break;
      case GAMMA:
        gammaDist = new gamma_distribution<double>(firstArg, secondArg);
        break;
      case EXT:
        extDist = new extreme_value_distribution<double>(firstArg, secondArg);
        break;
      default:
        break;
    }
  }

  double next(){
    switch(disttype){
      case UNIFORM:
        return (*unifDist)(generator);
      case EXP:
        return (*expDist)(generator);
      case GAMMA:
        return (*gammaDist)(generator);
      case EXT:
        return (*extDist)(generator);
      default:
        return 0;
    }
  }
};

bool hasTime(chrono::time_point<std::chrono::system_clock> startTime, double timeout);

ifstream prepareStream(const string &path, char* buffer);