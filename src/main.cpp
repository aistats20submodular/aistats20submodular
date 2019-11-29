#include <iostream>
#include "utils.h"
#include <sstream>
#include "movie_data.h"
#include <fstream>
#include <ctime>
#include <locale>


//#include "inputs/SvdData.h"
//#include "inputs/MoviesKnapsack.h"
//#include "inputs/AverageRatingKnapsack.h"
#include "inputs/SubtractAverageKnapsack.h"
//#include <inputs/DelegateKnapsack.h>

#include "algorithms/MarginalRatioThresholding.h"
#include "algorithms/Greedy.h"
#include "algorithms/GreedyPP.h"
#include "algorithms/SievePP.h"
#include "algorithms/Distributed.h"
#include "algorithms/BranchingMRT.h"
#include "algorithms/MofiedSimple.h"
#include "algorithms/randomAlg.h"

//#include "algorithms/robust/AlgMult.h"
//#include "algorithms/MultiDimensional.h"

#include "Graph.h"
#include "inputs/CoverageKnapsack.h"
#include "inputs/LinearKnapsack.h"
#include "Dataset.h"
//#include "debugUtils.h"
//Erfan:

//:Erfan

using namespace std;

void readMoviesData(vector<Movie> &movies, vector<Rating> &ratings, int &userCount, const string &folder) {
  movies = readCsv<Movie>(folder + "/movies.csv", Movie::parse);
//  links = readCsv<Link>(folder + "/links.csv", Link::parse);
//  tags = readCsv<Tag>(folder + "/tags.csv", Tag::parse);
  ratings = readCsv<Rating>(folder + "/ratings.csv", Rating::parse);
  sort(begin(movies), end(movies), [](const Movie &a, const Movie &b) { return a.movie < b.movie; });
  int maxUser = 0;
  map<int, int> movieMap;
  for (int i = 0; i < (int) movies.size(); ++i) {
    movieMap[movies[i].movie] = i;
    movies[i].movie = i;
  }
  for (Rating &rating : ratings) {
    rating.user--;
    rating.movie = movieMap[rating.movie];
    maxUser = max(maxUser, rating.user);
  }
  userCount = maxUser + 1;
  cout << "Movie count = " << movies.size()
       << "; User count = " << userCount
       << "; Ratings count = " << ratings.size()
       << ";\n";
}

Knapsack<int> *createSubtractAverageKnapsack(
    const vector<Movie> &movies,
    const vector<Rating> &ratings,
    double k,
    const vector<int> &toCover,
    int d
) {
//  vector<Genres> genres = { Genres({"Adventure", "Action"}, {"Comedy", "Horror"}),
//                            Genres({"Drama", "Crime"}, {"Thriller", "Documentary"})};
//  vector<double> K = {10, 10};

//  vector<Genres> genres = { Genres({"Adventure", "Action"}, {"Comedy", "Horror"})};
//  vector<Genres> genres = { Genres({"Drama", "Romance"}, {"Sci-Fy", "Fantasy"})};
//  vector<Genres> genres = { Genres({"Thriller", "Western"}, {"Children", "Animation"})};
  vector<double> K(d, k);
  return new SubtractAverageKnapsack(movies, ratings, K, toCover);
}

void prepareKnapsack(
    const Dataset dataset,
    const string &inputFile,
    Knapsack<int> *&knapsack,
    double &kStep,
    double &knapsackOffset,
    int &size,
    vector<int> &toCover
) {
  const int d = 1; // always one in this project
  switch (dataset) {
    case COVERAGE: {
      Graph g = Graph::read(inputFile);
      double k = 1; // Just a default minimum value. Later the for loop will iterate over a range values
      vector<double> K(d, k);
      measureTime("Prepare knapsack problem", [&]() { knapsack = new CoverageKnapsack(K, g); });
      size = (int) g.vertices.size();
      kStep = 1;
      knapsackOffset = 0.6;
      break;
    }
    case MOVIES: {
      vector<Movie> movies;
      vector<Rating> ratings;
      int userCount = 0;

      // Datasets can be found here: https://grouplens.org/datasets/movielens/
      measureTime("Read data", [&]() { readMoviesData(movies, ratings, userCount, inputFile); });
//      measureTime("Read data", [&]() { readMoviesData(movies, ratings, userCount, "../data/ml-20m"); });

      int user = Rand::next(userCount);
      cout << "User: " << user << endl;
      cout << "User movies: ";
      int moviesCount = 0;
      for (const Rating &rating : ratings) {
        if (rating.user == user) {
          cout << rating.movie << " ";
          moviesCount++;
          toCover.push_back(rating.movie);
        }
      }
      cout << endl << "User movies count: " << moviesCount << endl;

      removeDuplicates(toCover);
      double k = 10;
//  measureTime("Prepare knapsack problem", [&]() { knapsack = createMoviesKnapsack(movies, ratings); });
//  measureTime("Prepare knapsack problem", [&]() { knapsack = createAverageRatingKnapsack(movies, ratings); });
      measureTime("Prepare knapsack problem",
                  [&]() { knapsack = createSubtractAverageKnapsack(movies, ratings, k, toCover, d); });
//      lowerBound = upperBound = 6;
      size = (int) movies.size();
      debugPrint(size);


      kStep = 1;
      break;
    }
//    case NONROBUST_KILLER: {
      // size = 1000;
      // //lowerBound = upperBound = 100;
      // vector<double> K = { 1 };
      // knapsack = new NonrobustKiller(K, size);
//      break;
//    }
    case LINEAR: {
      double k = 10;
      vector<double> K(d, k);
      LinearKnapsack *linearKnapsack = LinearKnapsack::readLinearKnapsack(K, inputFile);
      // LinearKnapsack* linearKnapsack = LinearKnapsack::readLinearKnapsack(K, "../data/linears/simpleLinear");
      size = linearKnapsack->n;
      knapsack = linearKnapsack;
      kStep = 0.00001;
      break;
    }
    default:
      return;
  }
}

void run(const string &inputFile, Dataset dataset, const double maxKnapsack, double timeout, const string &outputDir) {
  createDir(outputDir);

  Knapsack<int> *knapsack;
  double kStep;
  double knapsackOffset = 0;
  int size;
  vector<int> toCover;
  prepareKnapsack(dataset, inputFile, knapsack, kStep, knapsackOffset, size, toCover);

  vector<int> input = range(size);
  debugPrint(size);
  double lowerBound = 1e-8, upperBound = 1, eps = 0.01;
  vector<const Algorithm<int>*> algs = {
      // new MarginalRatioThresholding<int>(0.01, 1, 0.1),
      // new RandomAlg<int>(),
      new Distributed<int>(lowerBound, upperBound, eps),
      new GreedyPP<int>(0, NONE),
      new GreedyPP<int>(0, PLUS_MAX),
      new GreedyPP<int>(0, OR_MAX),
      new GreedyPP<int>(1, NONE),
      new SievePP<int>(lowerBound, upperBound, eps, NONE),
      new SievePP<int>(lowerBound, upperBound, eps, PLUS_MAX),
      new SievePP<int>(lowerBound, upperBound, eps, OR_MAX),
      new BranchingMRT<int>(lowerBound, upperBound, eps),
      // new GreedyPP<int>(1, false)
      // new GreedyPP<int>(1, true)
      // new GreedyPP<int>(2, false),
      // new GreedyPP<int>(0,true),
      // new GreedyPP<int>(0,false)
  };
  time_t now = time(nullptr);
  tm *ltm = localtime(&now);

  ostringstream fileName;

  fileName << "output_" << algs.size() << "_"
           << ltm->tm_mday << "_"
           << ltm->tm_hour << "_"
           << ltm->tm_min << "_"
           << ltm->tm_sec << "_";


  ostringstream fullFileName;
  fullFileName << outputDir << "/" << fileName.str() << ".csv";
  ofstream fileStream(fullFileName.str());
  fileStream << "algName,condID,nItems,capacity,oracleCalls,obj,cost,nItems,runTime" << endl;
  fileStream.precision(10);


  int condID = 0;
  const int nRepeat = 1;
  vector<bool> tl(algs.size(), false);
  int ki = 0;
  for (double k = knapsackOffset + 1; k < maxKnapsack + 1e-9; k += kStep) { // NOLINT(cert-flp30-c)
    knapsack->K[0] = k;
    ki++;
    cout << "ki=" << ki << "=>" << k << endl;
    for (int repeatInd = 0; repeatInd < nRepeat; repeatInd++) {
      vector<double> objectives;
      int algi = -1;
      for (const auto& alg: algs) {
        algi++;
        if (tl[algi]) {
          cout << "-" << endl;
          fileStream << "-" << endl;
          continue;
        }
        knapsack->reset();// remove cache if any
        auto startTime = std::chrono::system_clock::now();
        auto res = alg->solve(input, *knapsack, timeout);
        double duration = (std::chrono::duration<double>(std::chrono::system_clock::now() - startTime)).count();
        //cout << "duration =" << duration << endl;
        // cout<<res.set.size()<<endl;
        res.runTime = duration;
        res.nItems = size;
        res.capacity = k;
        res.algName = alg->name();
        res.condID = condID;
        if (res.objective == -1) {
          cout << "-" << endl;
          fileStream << "-" << endl;
          tl[algi] = true;
        } else {
          cout << res.algName << ","
               << res.condID << ","
               << res.nItems << ","
               << res.capacity << ","
               << res.oracleCalls << ","
               << res.objective << ","
               << res.c[0] << ","
               << res.set.size() << ","
               << res.runTime
               << endl;
          fileStream << res.algName << ","
                     << res.condID << ","
                     << res.nItems << ","
                     << res.capacity << ","
                     << res.oracleCalls << ","
                     << res.objective << ","
                     << res.c[0] << ","
                     << res.set.size() << ","
                     << res.runTime << endl;
          fileStream.flush();
        }
        objectives.push_back(res.objective);
      }
      condID++;
    }
  }


  fileStream.close();
  cout << "\n ** To see the result csv:" << endl;
  cout << "cat " << fullFileName.str() << " | column -t -s, | less -S" << endl;

  cout << "\n ** To draw paired comparison plots:" << endl;
  cout << "python ../pythonCodes/pairwiseCSV.py --fileName " << fileName.str() << endl;
}

int runWithCommandLineArgs(const vector<string> &args) {
  if (args.size() != 5) {
    cout
        << "Command line format: submodular-knapsack <input file> <dataset type> <max knapsack size> <timeout> <output directory>"
        << endl;
    cout << R"(<dataset type> can take values "movies", "coverage", "linear" (without quotes))" << endl;
    return -1;
  }
  Dataset ds;
  string inputFile = args[0];
  string datasetName = args[1];
  transform(datasetName.begin(), datasetName.end(), datasetName.begin(), ::tolower);
  if (datasetName == "movies") {
    ds = MOVIES;
  } else if (datasetName == "linear") {
    ds = LINEAR;
  } else if (datasetName == "coverage") {
    ds = COVERAGE;
  } else {
    cout << "The dataset type " << datasetName << " is not supported" << endl;
    cout << R"(Dataset type can take values "movies", "coverage", "linear" (without quotes))" << endl;
    return -1;
  }
  double nK;
  try {
    nK = stod(args[2]);
  } catch (...) {
    cout << "Max knapsack size should be a number, but " << args[2] << " was found";
    return -1;
  }
  double timeout;
  try {
    timeout = stod(args[3]);
  } catch (...) {
    cout << "Timeout should be a number, but " << args[3] << " was found";
    return -1;
  }
  string outputDir = args[4];
  run(inputFile, ds, nK, timeout, outputDir);
  return 0;
}

int main(int argc, char *argv[]) {
  vector<string> args(argc - 1);
  for (int i = 0; i < argc - 1; i++) {
    args[i] = argv[i + 1];
  }
  return runWithCommandLineArgs(args);
}
