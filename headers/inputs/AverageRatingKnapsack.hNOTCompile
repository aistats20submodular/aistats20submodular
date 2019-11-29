#pragma once

#include "utils.h"
#include "movie_data.h"
#include "inputs/Knapsack.h"

struct AverageRatingKnapsack : Knapsack<int> {

  const vector <Movie> movies;
  const vector <vector<string>> genres;
  const vector<double> K;
  vector<double> averageRating;

  AverageRatingKnapsack(
      const vector <Movie> &movies,
      const vector <Rating> &ratings,
      const vector <vector<string>>& genres,
      const vector<double> &K
  ) :
      Knapsack(K),
      movies(movies),
      genres(genres),
      K(K) {
    assert(K.size() == genres.size());
    vector<int> cnt(movies.size());
    averageRating = vector<double>(movies.size());
    for (const Rating& rating : ratings) {
      cnt[rating.movie]++;
      averageRating[rating.movie] += rating.rating;
    }
    for (int i = 0; i < (int)movies.size(); ++i) {
      if (cnt[i] != 0) {
        averageRating[i] /= cnt[i];
      }
    }
  }

  double c(int i, const int &e) const override {
    int sum = 1;
    for (const string& s : genres[i]) {
      sum += movies[e].genres.count(s);
    }
    return sum;
  }

  double f(vector<int> &z) const override {
    double res = 0;
    for (int movie : z) {
      res += averageRating[movie];
    }
    return res;
  }

  double dif(vector<int> &, int newElement, double) const override {
    return averageRating[newElement];
  }
};