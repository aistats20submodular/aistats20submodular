//#pragma once

#include "utils.h"

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif


default_random_engine generator(static_cast<unsigned long>(time(0)));

// Begin Rand implementations

double Rand::nextRand() { return std::uniform_real_distribution<double>(0.0,1.0)(generator); }

unsigned int Rand::next(unsigned long bound) {
  assert(bound != 0);
  if (bound == 1)
    return 0;
  return std::uniform_int_distribution<unsigned int>(0, static_cast<unsigned int>(bound - 1))(generator);
}

bool Rand::check(double p) {return nextRand() <= p; }

// End Rand implementations

inline bool fileExist(const std::string &name) {
  ifstream in(name);
  return in.good();
}

void measureTime(const string& message, const std::function<void()> &f) {
  cout << "Start " << message << endl;
  auto startTime = std::chrono::system_clock::now();
  f();
  cout << message << " time: " << (std::chrono::duration<double>(std::chrono::system_clock::now() - startTime)).count() << endl;
}

void createDir(const string& dir) {
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(dir.c_str());
#else
  mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

inline int sign(double x) {
  return x < -1e-9 ? -1 : (x > 1e-9 ? 1 : 0);
}

const int BufSize = 10 * 1000 * 1000;

ifstream prepareStream(const string &path, char* buffer) {
  ifstream in;
  std::ios::sync_with_stdio(false);
  in.open(path);
  in.rdbuf()->pubsetbuf(buffer, BufSize);
  return in;
}

void readFile(const string& path, const std::function<void (const string)> &f) {
  string s;
  char *buffer = new char[BufSize];
  auto in = prepareStream(path, buffer);
  unordered_map<int64, int> map;
  while (getline(in, s)) {
    f(s);
  }
  delete[] buffer;
  in.close();
}

void split(const string& str, char delimiter, vector<string>& res) {
  int start = 0;
  int n = (int)str.length();
  int cnt = 0;
  for (int i = 0; i < n; ++i) {
    if (str[i] == delimiter) {
      res[cnt++] = str.substr(start, i - start);
      start = i + 1;
    }
  }
  res[cnt++] = str.substr(start, n - start);
}


vector<string> split(const string& str, char delimiter) {
  vector<string> res;
  int start = 0;
  int n = (int)str.length();
  for (int i = 0; i < n; ++i) {
    if (str[i] == delimiter) {
      res.emplace_back(str.substr(start, i - start));
      start = i + 1;
    }
  }
  res.emplace_back(str.substr(start, n - start));
  return res;
}

bool existsIndex(int n, const function<bool (int)>& pred) {
  for (int i = 0; i < n; ++i) {
    if (pred(i)) {
      return true;
    }
  }
  return false;
}

vector<int> range(int n) {
  vector<int> res(n);
  for (int i = 0; i < n; ++i) {
    res[i] = i;
  }
  return res;
}

bool hasTime(chrono::time_point<std::chrono::system_clock> startTime, double timeout)  {
  double duration = (std::chrono::duration<double>(std::chrono::system_clock::now() - startTime)).count();
  return duration < timeout;
}