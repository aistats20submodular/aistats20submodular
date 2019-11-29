#pragma once

#include "utils.h"

struct Rating {
  int user, movie;
  double rating;

  Rating(int user, int movie, double rating) :
    user(user), movie(movie), rating(rating) { }

  static Rating parse(const vector<string>& input);
};

struct Movie {
  int movie;
  string title;
  set<string> genres;

  Movie(int movie, string title, const vector<string>& genres) :
    movie(movie), title(std::move(title)), genres(genres.begin(), genres.end()) {}

  static Movie parse(const vector<string>& input);
};

struct Link {
  int movie, imdb, tmdb;

  Link(int movie, int imdb, int tmdb) :
    movie(movie), imdb(imdb), tmdb(tmdb) {}

  static Link parse(const vector<string>& input);
};

struct Tag {
  int user, movie;
  string tag;

  Tag(int user, int movie, string tag) : user(user), movie(movie), tag(std::move(tag)) {}

  static Tag parse(const vector<string>& input);
};
