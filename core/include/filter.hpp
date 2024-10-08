#ifndef FILTER_HPP_
#define FILTER_HPP_

#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <unordered_map>
#include <fstream>
#include "json.hpp"
#include "util.hpp"

/* Returns true of the integer vector contains the integer x. */
bool isIn(std::vector<int> &v, int x);

class Filter {
    int _id;
    std::vector<int> _whitelist;
    std::vector<int> _blacklist;
    std::vector<int> _correction_whitelist;
    std::vector<int> _correction_blacklist;
public:
    Filter(int id);
    Filter();
    ~Filter();

    // default copy assignment/construction are fine

    /* Push integers to Filter's blacklists and whitelists, for global or correction filtering. */
    Filter& pushWhitelist(int x);
    Filter& pushBlacklist(int x);
    Filter& pushCorrectionWhitelist(int x);
    Filter& pushCorrectionBlacklist(int x);

    void clearLists();

    std::vector<int> &getWhitelist();
    std::vector<int> &getBlacklist();
    std::vector<int> &getCorrectionWhitelist();
    std::vector<int> &getCorrectionBlacklist();

    int &getID();
};

class FilterState {
    Filter *_filter;
public:
    FilterState(Filter *filter);
    FilterState();
    ~FilterState();

    // default copy assignment/construction are (reference is read only)

    /* Sets up instance to preserve state of provided filter. */
    void setFilter(Filter *filter);

    /* Attempts to pass integer through contained filter. */
    bool pass(int x);
    bool passCorrection(int x);

    /* Returns whether this FilterState is set to a specific Filter. */
    bool hasFilter();

    int id();

};

/* Searches the provided directory for .json files, and parses them to load filter data. Returns
   an unordered map mapping .json file names (excluding the .json extension) to their defined
   Filter data.

   All .json files parsed are expected to have the following format:

   e.g.
   {
        "name" : "testname",
        "id" : 0,
        "whitelist" : [1, 2, 3, 4],
        "blacklist" : [3],
        "correctionWhitelist" : [1, 2],
        "correctionBlacklist" : [2]
   }
*/
std::unordered_map<std::string, Filter> loadFilters(std::string dir);

#endif