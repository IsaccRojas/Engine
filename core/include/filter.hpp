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
bool isIn(std::vector<int>& v, int x);

/* Container for generic filtering information.

    Passed integer only passes if it is not contained in the blacklist, and then is contained in the whitelist.
    Blacklist is ignored if it is empty, and whitelist is ignored if it is empty.
*/
class Filter {
    std::string _name;
    int _id;
    std::vector<int> _whitelist;
    std::vector<int> _blacklist;
    std::vector<int> _correction_whitelist;
    std::vector<int> _correction_blacklist;
public:
    Filter(const char* name, int id, std::vector<int> whitelist, std::vector<int> blacklist, std::vector<int> correction_whitelist, std::vector<int> correction_blacklist);
    Filter();
    ~Filter();

    // default copy assignment/construction are fine

    /* Sets name of Filter. */
    Filter& setName(const char* name);

    /* Sets ID of Filter. */
    Filter& setID(int id);

    /* Push integers to Filter's blacklists and whitelists, for global or correction filtering. */
    Filter& pushWhitelist(int x);
    Filter& pushBlacklist(int x);
    Filter& pushCorrectionWhitelist(int x);
    Filter& pushCorrectionBlacklist(int x);

    void clearLists();

    std::string name();

    int id();

    std::vector<int>& getWhitelist();
    std::vector<int>& getBlacklist();
    std::vector<int>& getCorrectionWhitelist();
    std::vector<int>& getCorrectionBlacklist();
};

class FilterState {
    Filter* _filter;
public:
    FilterState(Filter* filter);
    FilterState();
    ~FilterState();

    // default copy assignment/construction are (reference is read only)

    /* Sets up instance to preserve state of provided filter. */
    void setFilter(Filter* filter);

    /* Attempts to pass integer through contained filter. */
    bool pass(int x);
    bool passCorrection(int x);

    /* Returns whether this FilterState is set to a specific Filter. */
    bool hasFilter();

    int id();

};

#endif