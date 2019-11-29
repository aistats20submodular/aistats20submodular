#pragma once

#include "movie_data.h"
#include "utils.h"

struct SvdData {
  REDSVD::RedSVD svd;
  int userCount, movieCount;
  int numFeatures;

  SvdData() {}

  SvdData(const vector<Rating>& ratings, int numFeatures) {
    this->numFeatures = numFeatures;
    userCount = ratings[0].user;
    movieCount = ratings[0].movie;
    for (const auto& rating : ratings) {
      userCount = max(userCount, rating.user);
      movieCount = max(movieCount, rating.movie);
    }
    userCount++;
    movieCount++;

    cout << "Users: " << userCount << "; Movies: " << movieCount << endl;

    auto triplets = vector_map<Eigen::Triplet<float>>(ratings, [](const Rating& rating) {
      return Eigen::Triplet(rating.user, rating.movie, (float)rating.rating);
    });

    REDSVD::SMatrixXf A(userCount, movieCount);
    A.setFromTriplets(begin(triplets), end(triplets));

    svd.run(A, numFeatures);

    for (int i = 0; i < numFeatures; ++i) {
      cout << svd.singularValues()[i] << " ";
    }
    cout << endl;
  }

  double getProd(int i, int j, const Eigen::MatrixXf& A, const Eigen::MatrixXf& B) const {
    assert(0 <= i && i < A.rows());
    assert(0 <= j && j < B.rows());
    double res = 0;
    for (int k = 0; k < numFeatures; ++k) {
      res += (double)A(i, k) * B(j, k) * svd.singularValues()[k];
    }
    return res;
  }

  double get(int u, int m) const {
    return getProd(u, m, svd.matrixU(), svd.matrixV());
  }

  double userMovieProd(int u, int m) const {
    return getProd(u, m, svd.matrixU(), svd.matrixV());
  }

  double movieMovieProd(int m1, int m2) const {
    return getProd(m1, m2, svd.matrixV(), svd.matrixV());
  }
};