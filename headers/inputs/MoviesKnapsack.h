#pragma once

#include "movie_data.h"
#include "inputs/Knapsack.h"
#include "utils.h"

struct MoviesKnapsack : Knapsack<int> {

  const vector<Movie> movies;
  const SvdData svdData;
  const vector<vector<string>> genres;
  const vector<double> K;
  const int user;
  const vector<int> userMovies;
  const double alpha;

  mutable map<int64, double> memSum;
  mutable vector<map<int64, double>> memMax;

  const int64 emptySetKey = 5;

  MoviesKnapsack(
      const vector<Movie> &movies,
      const SvdData &svdData,
      const vector<vector<string>>& genres,
      const vector<double>& K,
      int user,
      const vector<int> &userMovies,
      double alpha
  ) :
      Knapsack(K),
      movies(movies),
      svdData(svdData), // TODO: optimize?
      genres(genres),
      K(K),
      user(user),
      userMovies(userMovies),
      alpha(alpha),
      memMax(vector<map<int64, double>>(userMovies.size()))
  {
    assert(K.size() == genres.size());
    memSum[emptySetKey] = 0;
    for (int i = 0; i < (int)userMovies.size(); ++i) {
      memMax[i][emptySetKey] = 0; // TODO: it actually can be < 0
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
    if (z.empty()) {
      return 0;
    }
    int64 mul = 1343157;
    int64 prevKey = emptySetKey; // without last element
    for (int i = 0; i < (int)z.size() - 1; ++i) {
      prevKey = prevKey * mul + z[i];
    }
    int64 key = prevKey * mul + z[(int) z.size() - 1];
    int last = z[(int) z.size() - 1];
    if (memSum.count(prevKey) == 0) {
      // Compute function for everything except the last element
      // Actually, this branch should never happen when we only add one element in the end
      z.pop_back();
      f(z);
      z.emplace_back(last);
    }
    int movie = movies[last].movie;
    double userPref = memSum[prevKey] + svdData.userMovieProd(user, movie);
    memSum[key] = userPref;
    double sumMax = 0;
    for (int i = 0; i < (int)userMovies.size(); ++i) {
      double val = max(memMax[i][prevKey], svdData.movieMovieProd(userMovies[i], movie));
      memMax[i][key] = val;
      sumMax += val;
    }
    return (1 - alpha) * userPref + alpha * sumMax;
  }
};