#pragma once

template <typename T>
struct Result {
  vector<T> set;
  int oracleCalls;
  double objective;
  vector<double> c;
  double runTime;
  string algName;
  int nItems;
  double capacity;
  int condID;

  Result(int constraintNum) : set(), oracleCalls(0), objective(0), c(vector<double>(constraintNum)) {}

  Result(const vector<T> &set, int oracleCalls, double objective, const vector<double>& c) :
      set(set), oracleCalls(oracleCalls), objective(objective), c(c) {}
};

template <typename T>
struct Knapsack {
  /// Weight limits for constraint
  vector<double> K;
  /// Number of constraints
  const int constraintNum;

  /// Weight for i-th constraint for element e
  virtual double c(int i, const T &e) const = 0;

  /// Objective
  virtual double f(const vector<T> &) const = 0;

  /// How function value changes when we add new element
  /// I calculate dif instead of new objective, since sometimes it can depend only on added element.
  /// I pass the value before new element is added, since the value is almost always calculated
  virtual double dif(vector<T> &set, T newElement, double oldObjective) const {
    set.emplace_back(newElement);
    double newObj = f(set);
    set.pop_back();
    return newObj - oldObjective;
  }

  vector<double> costs(const T& element) const {
    vector<double> res(constraintNum);
    for (int i = 0; i < constraintNum; ++i) {
      res[i] = c(i, element);
    }
    return res;
  }

  virtual void reset() const {
    cout<<"reset1"<<endl;
  }

  void addToResult(Result<T>& res, const T& element, double difObj) const {
    assert(constraintNum == (int)res.c.size());
    res.objective += difObj;
    res.set.emplace_back(element);
    for (int i = 0; i < constraintNum; ++i) {
      res.c[i] += c(i, element);
    }
  }

  void recalculate(Result<T>& res) const {
    res.objective = f(res.set);
    for (int i = 0; i < constraintNum; ++i) {
      res.c[i] = 0;
      for (const T& x : res.set) {
        res.c[i] += c(i, x);
      }
    }
  }

  void filterResult(Result<T>& res, const set<T>& exclude) const {
    assert(constraintNum == (int)res.c.size());
    res.set = filter<T>(res.set, [&](const T& x) { return exclude.count(x) == 0; });
    recalculate(res);
  }



  Knapsack(const vector<double>& K) : K(K), constraintNum((int) K.size()) {}

  virtual ~Knapsack() = default;
};
