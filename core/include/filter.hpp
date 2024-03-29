#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <unordered_map>
#include <fstream>
#include "json.hpp"
#include "util.hpp"

#ifndef FILTER_HPP_
#define FILTER_HPP_

bool isIn(std::vector<int> &v, int x);

class Filter {
    int _id;
    std::vector<int> _whitelist;
    std::vector<int> _blacklist;
public:
    Filter(int id);
    Filter();

    Filter& pushWhitelist(int x);
    Filter& pushBlacklist(int x);

    void clearLists();

    std::vector<int> &getWhiteList();

    std::vector<int> &getBlackList();

    int &getID();
};

class FilterState {
    Filter *_filter;
public:
    FilterState(Filter *filter);
    FilterState();

    void setFilter(Filter *filter);

    bool pass(int x);

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
        "blacklist" : [3]
   }
*/
std::unordered_map<std::string, Filter> loadFilters(std::string dir);

#endif