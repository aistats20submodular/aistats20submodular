#!/usr/bin/env bash
if [[ "$OSTYPE" == "darwin"* ]]; then
    g++ -Wall -Wextra -O2 --std=c++14 -pthread -I headers src/movie_data.cpp src/utils.cpp src/main.cpp -o submodular-knapsack
else
    g++ -Wall -Wextra -O2 --std=c++14 -fopenmp -pthread -I headers src/movie_data.cpp src/utils.cpp src/main.cpp -o submodular-knapsack
fi
