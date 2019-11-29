#include "movie_data.h"
#include "utils.h"
#include <sstream>

Rating Rating::parse(const vector <string>& input) {
  assert(input.size() == 4u);
  return Rating(stoi(input[0]), stoi(input[1]), stod(input[2]));
}

// There could be commas in title, have to split it
Movie Movie::parse(const vector <string>& input) {
  assert(input.size() >= 3u);
  string title;
  if (input.size() == 3u) {
    title = input[1];
  } else {
    stringstream ss;
    ss << input[1];
    for (int i = 2; i < (int)input.size() - 1; ++i) {
      ss << "," << input[i];
    }
    title = ss.str();
  }
  return Movie(stoi(input[0]), title, split(input[input.size() - 1], '|'));
}

Link Link::parse(const vector <string>& input) {
  assert(input.size() == 3u);
  return Link(stoi(input[0]), stoi(input[1]), stoi(input[2]));
}

Tag Tag::parse(const vector <string>& input) {
  assert(input.size() == 4u);
  return Tag(stoi(input[0]), stoi(input[1]), input[2]);
}

