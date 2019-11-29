This repository contains source files for paper `"BRING YOUR OWN GREEDY"+MAX: Near-Optimal 1/2-Approximations for Submodular Knapsack`.

### Build

    ./build.sh

This command should build the project. Your PATH environment variable should contain `g++` with `C++14` support. The program was tested on Ubuntu (`g++ 7.4.0`), Windows (cygwin, `g++ 7.4.0`), macOS (`clang 11.0.0`).

Note that for macOS the generated code will be slower, since clang by default doesn't support OpenMP. If you use macOS and your compiler supports OpenMP, please edit `build.sh` accordingly.

### Run experiments on datasets from the paper

To run the experiments on `ego-Facebook`: `./run_fb.sh`

To run the experiments on `com-dblp`: `./run_dblp.sh`

Since `ml-20` dataset is too large for github, it should be downloaded separately:

    ./download_ml.sh
    
It should work in Linux and macOS, but Windows by default lacks necessary tools. One solution is to install `wget` and `unzip` with cygwin. Another one is to manually download http://files.grouplens.org/datasets/movielens/ml-20m.zip and unpack it to `data` folder.

    ./run_ml.sh

Please note that for `ml-20` the knapsack preparation time takes more than 10 minutes.

The logs are written to directory `results`: each run will create a new file with the corresponding timestamp. Folder `example_results` contain possible outputs after execution of these commands.

### Output description

Generated files have csv-format. Each line corresponds to execution of a particular algorithm on a certain knapsack size, and contains the following information:
* Algorithm name
* Experiment number (you can ignore this value)
* The total number of items to select from
* Knapsack capacity
* The number of oracle calls performed by the algorithm
* The objective of the solution returned by the algorithm
* The solution cost
* The number of collected items
* The running time

If some line consists of a single `-`, it means that the algorithm timed out.

### General command line syntax

    ./submodular-knapsack <input file/dir> <dataset type> <max knapsack size> <timeout> <output directory>
    
* `dataset type` can be `coverage` and `movies`.
* The program experiments on knapsacks of sizes `1.6, 2.6, ..., max knapsack size + 0.6`
* Each algorithm can spent at most `timeout` seconds on each knapsack problem.
* The logs are written to a new file in `output directory`. The file contains a timestamps as a part of its name.

For `coverage` dataset the input file should contain an undirected graph. Each line of the file represents an edge, which is defined by two vertices.

For `movies` dataset the input directory should contain files `movies.csv` and `ratings.csv` in the same format as `ml-20` dataset.

### Datasets

In this project the following datasets are used:
* ego-Facebook: http://snap.stanford.edu/data/facebook_combined.txt.gz
* com-dblp: http://snap.stanford.edu/data/bigdata/communities/com-dblp.ungraph.txt.gz (first 4 lines should be removed from the unpacked file)
* ml-20: http://files.grouplens.org/datasets/movielens/ml-20m.zip
