#pragma once

#include "utils.h"
#include "movie_data.h"
#include "inputs/Knapsack.h"

struct Genres {
  vector<string> good;
  vector<string> bad;

  Genres(const vector<string> &good, const vector<string> &bad) : good(good), bad(bad) {}
};

struct SubtractAverageKnapsack : Knapsack<int> {

  const vector<Movie> movies;
  const vector<double> K;
  vector<vector<pair<int, double>>> subtractedRatings;
  vector<vector<float>> products;
  mutable map<int64, vector<float>> mem;
  mutable set<int64> was;
  const vector<int> toCover;

  const int64 emptySetKey = 5;

  SubtractAverageKnapsack(
    const vector<Movie> &movies,
    const vector<Rating> &ratings,
    const vector<double> &K,
    const vector<int>& toCover
  ) :
    Knapsack(K),
    movies(movies),
    K(K),
    toCover(toCover)
  {
    double average = 0;
    for (const Rating &rating : ratings) {
      average += rating.rating;
    }
    average /= ratings.size();
    subtractedRatings = vector<vector<pair<int, double>>>(movies.size());
    for (const Rating &rating : ratings) {
      subtractedRatings[rating.movie].emplace_back(make_pair(rating.user, rating.rating - average));
    }
    for (auto &rates : subtractedRatings) {
      sort(rates.begin(), rates.end(), [](auto a, auto b) { return a.first < b.first; });
    }
    computeProducts(false);
    mem[emptySetKey] = vector<float>(toCover.size(), 0);
  }

  void computeProducts(bool cached) {
    (void)(cached);
    int n = (int) movies.size();
    products = vector<vector<float>>(n, vector<float>(n));
//    if (cached) {
//      char *buffer = new char[100*1000*100];
//      ifstream cache = prepareStream("moviesCache", buffer);
//      for (int i = 0; i < n; ++i) {
//        for (int j = 0; j < n; ++j) {
//          cache >> products[i][j];
//        }
//      }
//      delete[] buffer;
//      cache.close();
//    } else {
#pragma omp parallel for
      for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
          products[j][i] = products[i][j] = product(i, j);
        }
      }
//      ofstream cache("moviesCache");
//      for (int i = 0; i < n; ++i) {
//        for (int j = 0; j < n; ++j) {
//          cache << products[i][j] << " ";
//        }
//        cache << endl;
//      }
//      cache.close();
//    }
  }

  float product(int m1, int m2) const {
    const vector<pair<int, double>> &b = subtractedRatings[m2];
    int j = 0;
    double sum = 0;
    for (const auto &p : subtractedRatings[m1]) {
      int ind = p.first;
      while (j < (int) b.size() && b[j].first < ind) {
        j++;
      }
      if (j < (int) b.size() && ind == b[j].first) {
        sum += p.second * b[j].second;
      }
    }
    return (float) sum;
  }

  void reset() const override {
    mem.clear();
    mem[emptySetKey] = vector<float>(toCover.size(), 0);
  }

  double c(int i, const int &e) const override {
    // const Genres &genres = this->genres[i];
    // double res = 2;
    // for (const string &s : genres.good) {
    //   res -= movies[e].genres.count(s) * 0.5;
    // }
    // for (const string &s : genres.bad) {
    //   res += movies[e].genres.count(s) * 0.5;
    // }
    // return max(res, 1.);
    (void)i;
    vector<int> temp(1,e);
    return max(1., 2 * f(temp) - 1);
  }

  void checkMem() const {
    if (mem.size() > 7e9 / (sizeof(float) * toCover.size())) {
      cerr << "clear ";
      mem.clear();
      mem[emptySetKey] = vector<float>(toCover.size(), 0);
    }
  }

  double f(const vector<int> &zz) const {
    auto z = zz;
    if (z.empty()) {
      return 0;
    }
    int64 mul = 1343157;
    // without last element
    int64 prevKey = emptySetKey;
    for (int i = 0; i < (int) z.size() - 1; ++i) {
      prevKey = prevKey * mul + z[i];
    }
    int lastMovie = z[(int) z.size() - 1];
    int64 key = prevKey * mul + lastMovie;
    if (mem.count(key) > 0) {
//      cerr << "match ";
      return vector_sum_id(mem[key]) / toCover.size();
    }
    if (mem.count(prevKey) == 0) {
      z.pop_back();
      f(z);
      // Call f twice so that it remembers prevKey
      f(z);
//      addToHash(z, prevKey);
      z.push_back(lastMovie);
    }
    assert(mem.count(prevKey) > 0);
    const auto& prevMax = mem[prevKey];
//    vector<float> newMax(movies.size());
//    double res = 0;
//#pragma omp parallel for reduction(+:res)
//    for (int m = 0; m < (int) movies.size(); ++m) {
//      newMax[m] = max(prevMax[m], products[lastMovie][m]);
//      res += newMax[m];
////      res += max(prevMax[m], products[lastMovie][m]);
//    }
    if (was.count(key) > 0) {
      was.erase(key);
      vector<float> newMax(toCover.size());
      double res = 0;
//#pragma omp parallel for reduction(+:res)
      for (int i = 0; i < (int) toCover.size(); ++i) {
        newMax[i] = max(prevMax[i], products[lastMovie][toCover[i]]);
        res += newMax[i];
      }
      checkMem();
      mem[key] = newMax;
      return res / toCover.size();
    } else {
      if (was.size() > 1e8) {
        cerr << "clearWas ";
        was.clear();
      }
      was.insert(key);
      double res = 0;
//#pragma omp parallel for reduction(+:res)
      for (int i = 0; i < (int) toCover.size(); ++i) {
        res += max(prevMax[i], products[lastMovie][toCover[i]]);
      }
      return res / toCover.size();
    }
  }
};
