#include <iostream>
#include "utils.h"
#include <sstream>
#include "movie_data.h"

//#include "inputs/SvdData.h"
//#include "inputs/MoviesKnapsack.h"
#include "inputs/AverageRatingKnapsack.h"
#include "inputs/SubtractAverageKnapsack.h"
#include <inputs/DelegateKnapsack.h>
#include <algorithms/robust/MakeRobust.h>

#include "algorithms/MarginalRatioThresholding.h"
#include "algorithms/Greedy.h"
#include "algorithms/robust/AlgMult.h"
#include "algorithms/MultiDimensional.h"

#include "Graph.h"
#include "inputs/CoverageKnapsack.h"
#include "testWithRemovals.h"
#include "Dataset.h"

//Erfan:
#define RandomLinearInput 13
//:Erfan


using namespace std;

void readData(vector<Movie>& movies, vector<Rating>& ratings, int& userCount, const string& folder) {
  movies = readCsv<Movie>(folder + "/movies.csv", Movie::parse);
//  links = readCsv<Link>(folder + "/links.csv", Link::parse);
//  tags = readCsv<Tag>(folder + "/tags.csv", Tag::parse);
  ratings = readCsv<Rating>(folder + "/ratings.csv", Rating::parse);
  sort(begin(movies), end(movies), [](const Movie& a, const Movie& b) { return a.movie < b.movie; });
  int maxUser = 0;
  map<int, int> movieMap;
  for (int i = 0; i < (int)movies.size(); ++i) {
    movieMap[movies[i].movie] = i;
    movies[i].movie = i;
  }
  for (Rating &rating : ratings) {
    rating.user--;
    rating.movie = movieMap[rating.movie];
    maxUser = max(maxUser, rating.user);
  }
  userCount = maxUser + 1;
  cout << "Movie count = " << movies.size() << "; User count = " << userCount << "; Ratings count = " << ratings.size() << ";\n";
}

/*
void checkData(const vector<Rating>& ratings, const SvdData& svdData) {
  createDir("../out");
  ofstream checkFile;
  checkFile.open("../out/check.txt", ofstream::out);
  for (int t = 0; t < 1000; t++) {
    int i = Rand::next(ratings.size());
    const Rating &rating = ratings[i];
    checkFile << rating.rating << " " << svdData.get(rating.user, rating.movie) << endl;
  }
  checkFile.close();
}
*/

Result<int> runAlgorithm(const vector<int>& input, const Knapsack<int>& knapsack, const Algorithm<int>& algorithm) {
  auto res = measureTime<Result<int>>("Algorithm " + algorithm.name(), [&]() {
    auto res = algorithm.solveBeforeRemovals(input, knapsack);
    cout << "Oracle calls: " << res.oracleCalls << endl;
    cout << "Objective before removals: " << res.objective << endl;
    cout << "Costs: " << vectorToStr(res.c, " ") << endl;
    cout << endl;
    cout << "Elements before removals: " << res.set.size() << endl;
    cout << vectorToStr(res.set, " ") << endl;
    return res;
  });
  cout << endl;
  return res;
}

/*
Knapsack<int>* createMoviesKnapsack(const vector<Movie> &movies, const vector<Rating> &ratings) {
  auto svdData = measureTime<SvdData>("Run SVD", [&]() { return SvdData(ratings, 500); });
  measureTime("Check data", [&]() { checkData(ratings, svdData); });
  vector<vector<string>> genres = {{"Comedy", "Horror"}};
  vector<double> K = {100};
  vector<int> userMovies = {10, 20, 30};
  int user = 1;
  double alpha = 0.5;
  return new MoviesKnapsack(movies, svdData, genres, K, user, userMovies, alpha);
}
*/

Knapsack<int>* createAverageRatingKnapsack(const vector<Movie> &movies, const vector<Rating> &ratings) {
  vector<vector<string>> genres = {{"Comedy", "Horror"}};
  vector<double> K = {10};
  return new AverageRatingKnapsack(movies, ratings, genres, K);
}

Knapsack<int>* createSubtractAverageKnapsack(const vector<Movie> &movies, const vector<Rating> &ratings, double k, vector<int> toCover, int d) {
//  vector<Genres> genres = { Genres({"Adventure", "Action"}, {"Comedy", "Horror"}),
//                            Genres({"Drama", "Crime"}, {"Thriller", "Documentary"})};
//  vector<double> K = {10, 10};

//  vector<Genres> genres = { Genres({"Adventure", "Action"}, {"Comedy", "Horror"})};
//  vector<Genres> genres = { Genres({"Drama", "Romance"}, {"Sci-Fy", "Fantasy"})};
//  vector<Genres> genres = { Genres({"Thriller", "Western"}, {"Children", "Animation"})};
  vector<Genres> genres = { Genres({"Comedy", "Horror"}, {"Adventure", "Action"})};
  if (d == 2) {
    genres.push_back(Genres({"Drama", "Crime"}, {"Thriller", "Documentary"}));
  }
  vector<double> K(d, k);
  for (int i = 0; i < (int) genres.size(); ++i) {
    cout << "K = " << K[i] << "; bad = (" << vectorToStr(genres[i].bad, ", ");
    cout << "); good = (" << vectorToStr(genres[i].good, ", ") << ")\n";
  }
  return new SubtractAverageKnapsack(movies, ratings, genres, K, toCover);
}

/*
void removeWithProbability() {
  cout << "Objective/Total oracle calls after removals (each element from the initial set is removed with probability p)" << endl;
  cout << "    " << vectorToStr(vector_map<string>(algs, [](auto alg) { return alg->name(); }), ",    ") << endl;
  for (double p = 0; p < 1.0 + 0.000001; p += (p < 0.9) ? 0.01 : 0.001) {
    set<int> exclude;
    for (int i = 0; i < size; i ++) {
      if (Rand::check(p)) {
        exclude.insert(i);
      }
    }
    cout.precision(3);
    cout << "p = " << p << " | Objective:";
    vector<int> oracleCalls;
    cout.precision(5);
    for (int i = 0; i < (int) algs.size(); ++i) {
      const auto alg = algs[i];
      auto res = alg->finish(results[i], exclude, *knapsack, lowerBound, upperBound, eps);
      oracleCalls.emplace_back(res.oracleCalls);
      cout << "    " << res.objective;
    }
    cout << "    | Oracle calls:    " << vectorToStr(oracleCalls, "    ") << endl;
  }
}
*/

// CUrrently is not used
struct Parameters {
  int size;
  Knapsack<int>* knapsack;
  int d;
  double k;
  double m;
  double lowerBound;
  double upperBound;
  double eps;
  double mult;
};

void run() {
  int d = 1;
  double k = 2;
  double eps = 0.5;
//  omp_set_num_threads(8);
//  Eigen::setNbThreads(8);
  Knapsack<int>* knapsack;
  int size;
  double lowerBound, upperBound;
  vector<int> toCover;
  Dataset dataset = COVERAGE;
  //Dataset dataset = MOVIES;
  switch (dataset) {
    // case RandomLinearInput: {
      // cout<<"Not yet implemented"<<endl;
      // break;
    // }
    case COVERAGE: {
//      Graph g = Graph::read("../data/graphs/facebook.txt");
//    Graph g = Graph::read("../data/graphs/wiki-vote.txt");
//    Graph g = Graph::read("../data/graphs/twitter.txt");
      Graph g = Graph::read("../data/graphs/completeGraph.txt");
      vector<double> K(d, k);
      cout<<"here:"<<d<<" "<<k<<" "<<endl;
      measureTime("Prepare knapsack problem", [&]() { knapsack = new CoverageKnapsack(K, g); });
      cout<<"after preparing:"<< (*knapsack).constraintNum <<endl;
      size = (int) g.vertices.size();
      lowerBound = upperBound = 0.5;
//    lowerBound = upperBound = 0.35;
//    lowerBound = upperBound = 0.02;
      break;
    }
    case MOVIES: {
      vector<Movie> movies;
      vector<Rating> ratings;
      int userCount = 0;

      // Datasets can be found here: https://grouplens.org/datasets/movielens/
      measureTime("Read data", [&]() { readData(movies, ratings, userCount, "../data/ml-latest-small"); });
//      measureTime("Read data", [&]() { readData(movies, ratings, userCount, "../data/ml-20m"); });
/*
      for (int i = 0; i < 50; ++i) {
        int n = (int)movies.size();
        movies.push_back(Movie(n, "SomeMovie", {"Comedy", "Horror"}));
        for (int u = 0; u < userCount; ++u) {
//          ratings.emplace_back(u, n, Rand::check(0.5) ? 1 : 5);
          ratings.emplace_back(u, n, 5);
        }
      }
*/
      int user = Rand::next(userCount);
      cout << "User: " << user << endl;
      cout << "User movies: ";
      int moviesCount = 0;
      for (const Rating& rating : ratings) {
        if (rating.user == user) {
          cout << rating.movie << " ";
          moviesCount++;
          toCover.push_back(rating.movie);
        }
      }
      cout << endl << "User movies count: " << moviesCount << endl;
/*
      for (int i = 0; i < 100; ++i) {
        toCover.push_back(Rand::next(movies.size()));
      }
*/
      removeDuplicates(toCover);

//  measureTime("Prepare knapsack problem", [&]() { knapsack = createMoviesKnapsack(movies, ratings); });
//  measureTime("Prepare knapsack problem", [&]() { knapsack = createAverageRatingKnapsack(movies, ratings); });
      measureTime("Prepare knapsack problem", [&]() { knapsack = createSubtractAverageKnapsack(movies, ratings, k, toCover, d); });
      lowerBound = upperBound = 6;
      size = (int) movies.size();
      break;
    }
    case NONROBUST_KILLER: {
      size = 1000;
      lowerBound = upperBound = 100;
      vector<double> K = { 1 };
      knapsack = new NonrobustKiller(K, size);
      break;
    }
  }
  vector<int> input = range(size);
  for (int i = 0; i < (int)toCover.size(); ++i) {
    swap(input[i], input[toCover[i]]);
  }

/*
  reverse(input.begin(), input.end());
  shuffle(next(input.begin()), input.end(), generator);
*/
  double mult = dataset == MOVIES ? 300 : 700;
  for (int i = 0; i < 3; ++i) {
    printf("Inside the for-loop\n");
    auto robustAlg = MakeRobust<int>(new MarginalRatioThresholding<int>(lowerBound, upperBound, eps), mult);
    auto curRes = robustAlg.solveBeforeRemovals(input, *knapsack);
    auto curObj = robustAlg.finish(curRes, {}, *knapsack, lowerBound, upperBound, eps).objective;
    cerr << "obj = " << curObj << endl;
    lowerBound = upperBound = curObj;
  }
  lowerBound /= 2;
//  upperBound *= 0.6;


  cout << "lower bound = " << lowerBound << "; upper bound = " << upperBound << "; eps = " << eps << ";\n\n";
  int m = 0;
//  int robustLB = 10, robustUB = 10;
  double robustLB = lowerBound, robustUB = upperBound;
  vector<const Algorithm<int>*> algs =
    {
//      new MarginalRatioThresholding<int>(lowerBound, upperBound, eps),
//      new MultiDimensional<int>(lowerBound, upperBound, eps),
      new Greedy<int>(),
//      new AlgMult<int>(lowerBound, upperBound, eps, m),
    };
  vector<Result<int>> results = { runAlgorithm(input, *knapsack, *algs[0]) };
  cout<<"ERER results:"<<results[0].objective<<endl;
  for (int i = 0; i < 3; ++i) {
    auto robustAlg = MakeRobust<int>(new MarginalRatioThresholding<int>(lowerBound, upperBound, eps), mult);
    auto curRes = robustAlg.solveBeforeRemovals(input, *knapsack);
    mult *= (double)results[0].set.size() / curRes.set.size();
    cerr << "mult = " << mult << endl;
  }
  addRange(algs,
           {
             new MakeRobust<int>(new MarginalRatioThresholding<int>(robustLB, robustUB, eps), mult),
             new MakeRobust<int>(new MultiDimensional<int>(robustLB, robustUB, eps), mult),
           });
  if (d == 1) {
    algs.push_back(new MakeRobust<int>(new Greedy<int>(), mult));
  }
  for (int i = 1; i < (int) algs.size(); ++i) {
    results.push_back(runAlgorithm(input, *knapsack, *algs[i]));
  }
  cout << endl;

//  removeWithProbability();
  cout << std::fixed;
  cout << "Elements count in all algorithms: ";
  for (const auto& res : results) {
    cout << res.set.size() << " ";
  }
  cout << endl;
  cout << "Objective after removal of union of returned elements" << endl;
  set<int> exclude;
  for (const auto alg : algs) {
    cout << "    " << alg->name() << ",";
  }
  cout << "    OPT" << endl;
  Result<int> rem(d);
  auto offline = AlgMult<int>(lowerBound, upperBound, eps, m);
  while (true) {
    cout.precision(5);
    vector<int> nextExclude;
    for (int i = 0; i < (int) algs.size(); ++i) {
      const auto alg = algs[i];
      auto res = alg->finish(results[i], exclude, *knapsack, lowerBound, upperBound, eps);
      cout << "    " << res.objective;
      addRange(nextExclude, res.set);
    }
    rem.set.clear();
    for (int x : input) {
      if (exclude.count(x) == 0) {
        rem.set.push_back(x);
      }
    }

    auto offlineSolution = offline.finish(rem, {}, * knapsack, lowerBound, upperBound, eps);
//        auto offlineSolution = greedy.solve(remaining, *knapsack, false);
    double approx = d == 1 ? 1 - pow(exp(1), - offlineSolution.c[0] / k) : 1. / (1 + 2. * d);
//    cerr << offlineSolution.objective << " " << approx << endl;
    cout << "    " << offlineSolution.objective / approx;
//    cout << "    " << offlineSolution.objective;
    cout << "   |del|=" << exclude.size();
    cout << endl;
    if (nextExclude.empty()) {
      break;
    }
    for (int x : nextExclude) {
      exclude.insert(x);
    }
  }
  // I don't delete pointers, but who cares
}

int main() {
  measureTime("Total", run);
//  measureTime("Total", testWithRemovals);
  return 0;
}