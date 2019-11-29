#!/usr/bin/env bash
if [[ "$OSTYPE" == "darwin"* ]]; then
   curl -O http://files.grouplens.org/datasets/movielens/ml-20m.zip
else
    wget http://files.grouplens.org/datasets/movielens/ml-20m.zip
fi
unzip ml-20m.zip -d data

