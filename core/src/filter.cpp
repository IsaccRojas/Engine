#include "../include/filter.hpp"

bool isIn(std::vector<int>& v, int x) {
    for (unsigned i = 0; i < v.size(); i++)
        if (v[i] == x)
            return true;
    return false;
}

Filter::Filter(const char* name, int id, std::vector<int> whitelist, std::vector<int> blacklist, std::vector<int> correction_whitelist, std::vector<int> correction_blacklist) : 
    _name(name), _id(id), _whitelist(whitelist), _blacklist(blacklist), _correction_whitelist(correction_whitelist), _correction_blacklist(correction_blacklist)
{}
Filter::Filter() : _id(-1) {}
Filter::~Filter() { /* automatic destruction is fine */ }

Filter& Filter::setName(const char* name) {
    _name = name;
    return *this;
}

Filter& Filter::setID(int id) {
    _id = id;
    return *this;
}

Filter& Filter::pushWhitelist(int x) {
    _whitelist.push_back(x);
    return *this;
}
Filter& Filter::pushBlacklist(int x) {
    _blacklist.push_back(x);
    return *this;
}
Filter& Filter::pushCorrectionWhitelist(int x) {
    _correction_whitelist.push_back(x);
    return *this;
};
Filter& Filter::pushCorrectionBlacklist(int x) {
    _correction_blacklist.push_back(x);
    return *this;
}

void Filter::clearLists() {
    _whitelist.clear();
    _blacklist.clear();
    _correction_whitelist.clear();
    _correction_blacklist.clear();
}

std::string Filter::name() {
    return _name;
}

int Filter::id() {
    return _id;
}

std::vector<int>& Filter::getWhitelist() {
    return _whitelist;
}
std::vector<int>& Filter::getBlacklist() {
    return _blacklist;
}
std::vector<int>& Filter::getCorrectionWhitelist() {
    return _correction_whitelist;
}
std::vector<int>& Filter::getCorrectionBlacklist() {
    return _correction_blacklist;
}

FilterState::FilterState(Filter* filter) : _filter(filter) {}
FilterState::FilterState() : _filter(nullptr) {}
FilterState::~FilterState() { /* automatic destruction is fine */ }

void FilterState::setFilter(Filter* filter) {
    _filter = filter;
}

bool FilterState::pass(int x) {
    if (!_filter)
        throw std::runtime_error("Attempt to pass value with null Filter reference");

    // check if blacklist exists; if it does, check if x is in it
    std::vector<int>& blacklist = _filter->getBlacklist();
    if (blacklist.size() > 0)
        if (isIn(blacklist, x))
            return false;
    
    // check if whitelist exists; if it does, check if x is in it
    std::vector<int>& whitelist = _filter->getWhitelist();
    if (whitelist.size() > 0) {
        if (isIn(whitelist, x))
            return true;
        else
            return false;
    }
    
    // filter is empty, or only blacklist exists and x is not in it
    return true;
}

bool FilterState::passCorrection(int x) {
    if (!_filter)
        throw std::runtime_error("Attempt to pass correction value with null Filter reference");
    
    // check if blacklist exists; if it does, check if x is in it
    std::vector<int>& blacklist = _filter->getCorrectionBlacklist();
    if (blacklist.size() > 0)
        if (isIn(blacklist, x))
            return false;
    
    // check if whitelist exists; if it does, check if x is in it
    std::vector<int>& whitelist = _filter->getCorrectionWhitelist();
    if (whitelist.size() > 0) {
        if (isIn(whitelist, x))
            return true;
        else
            return false;
    }

    // only blacklist exists and x is not in it
    return true;
}

bool FilterState::hasFilter() { return _filter != nullptr; }

int FilterState::id() {
    if (!_filter)
        throw std::runtime_error("Attempt to get Filter ID with null Filter reference");
    
    return _filter->id();
}