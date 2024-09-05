#include "../include/filter.hpp"

bool isIn(std::vector<int> &v, int x) {
    for (unsigned i = 0; i < v.size(); i++)
        if (v[i] == x)
            return true;
    return false;
}

Filter::Filter(int id) : _id(id) {}
Filter::Filter() : _id(-1) {}
Filter::~Filter() { /* automatic destruction is fine */ }

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

std::vector<int> &Filter::getWhitelist() {
    return _whitelist;
}
std::vector<int> &Filter::getBlacklist() {
    return _blacklist;
}
std::vector<int> &Filter::getCorrectionWhitelist() {
    return _correction_whitelist;
}
std::vector<int> &Filter::getCorrectionBlacklist() {
    return _correction_blacklist;
}

int &Filter::getID() {
    return _id;
}

FilterState::FilterState(Filter *filter) : _filter(filter) {}
FilterState::FilterState() : _filter(nullptr) {}
FilterState::~FilterState() { /* automatic destruction is fine */ }

void FilterState::setFilter(Filter *filter) {
    _filter = filter;
}

bool FilterState::pass(int x) {
    if (!_filter) {
        std::cerr << "WARN: FilterState::pass: attempt to pass value in FilterState instance " << this << " without a Filter reference" << std::endl;
        return false;
    }

    // check if blacklist exists; if it does, check if x is in it
    std::vector<int> &blacklist = _filter->getBlacklist();
    if (blacklist.size() > 0)
        if (isIn(blacklist, x))
            return false;
    
    // check if whitelist exists; if it does, check if x is in it
    std::vector<int> &whitelist = _filter->getWhitelist();
    if (whitelist.size() > 0) {
        if (isIn(whitelist, x))
            return true;
        else
            return false;
    }
    
    // only blacklist exists and x is not in it
    return true;
}

bool FilterState::passCorrection(int x) {
    if (!_filter) {
        std::cerr << "WARN: FilterState::passCorrection: attempt to pass correction value in FilterState instance " << this << " without a Filter reference" << std::endl;
        return false;
    }
    
    // check if blacklist exists; if it does, check if x is in it
    std::vector<int> &blacklist = _filter->getCorrectionBlacklist();
    if (blacklist.size() > 0)
        if (isIn(blacklist, x))
            return false;
    
    // check if whitelist exists; if it does, check if x is in it
    std::vector<int> &whitelist = _filter->getCorrectionWhitelist();
    if (whitelist.size() > 0) {
        if (isIn(whitelist, x))
            return true;
        else
            return false;
    }

    // only blacklist exists and x is not in it
    return true;
}

int FilterState::id() { return _filter->getID(); }

std::unordered_map<std::string, Filter> loadFilters(std::string dir) {
    std::unordered_map<std::string, Filter> filters;
    
    // iterate on provided directory
    nlohmann::json data;
    std::string filename;
    for (auto iter = std::filesystem::directory_iterator(dir); iter != std::filesystem::directory_iterator(); iter++) {
        filename = iter->path().string();

        // check if file ends in .json
        if (endsWith(filename, ".json")) {
            
            // try to parse .json file
            try {
                data = nlohmann::json::parse(std::ifstream(filename));
            } catch (const std::exception& e) {
                std::cerr << "Error loading .json file '" << filename << "': " << e.what() << std::endl;
                goto dir_loop_end;
            }

            // variables to store all retrieved fields
            std::string name;
            int id;
            std::vector<int> whitelist;
            std::vector<int> blacklist;
            std::vector<int> correction_whitelist;
            std::vector<int> correction_blacklist;

            // retrieve name
            if (data.contains("name") && data["name"].is_string())
                name = data["name"];
            else {
                std::cerr << "Error loading .json file '" << filename << "': field 'name' is missing or is not a string" << std::endl;
                goto dir_loop_end;
            }

            // retrieve ID
            if (data.contains("id") && data["id"].is_number_integer())
                id = data["id"];
            else {
                std::cerr << "Error loading .json file '" << filename << "': field 'id' is missing or is not an integer" << std::endl;
                goto dir_loop_end;
            }

            // retrieve whitelist
            if (data.contains("whitelist") && data["whitelist"].is_array()) {

                // iterate on whitelist
                for (unsigned i = 0; i < data["whitelist"].size(); i++) {

                    // retrieve value
                    if (data["whitelist"][i].is_number_integer())
                        whitelist.push_back(data["whitelist"][i]);
                    else {
                        std::cerr << "Error loading .json file '" << filename << "': field 'whitelist', element " << i << " is not an integer" << std::endl;
                        goto dir_loop_end;
                    }
                }
            } else {
                std::cerr << "Error loading .json file '" << filename << "': field 'whitelist' is missing or is not an array" << std::endl;
                goto dir_loop_end;
            }

            // retrieve blacklist
            if (data.contains("blacklist") && data["blacklist"].is_array()) {

                // iterate on blacklist
                for (unsigned i = 0; i < data["blacklist"].size(); i++) {
                
                    // retrieve value
                    if (data["blacklist"][i].is_number_integer())
                        blacklist.push_back(data["blacklist"][i]);
                    else {
                        std::cerr << "Error loading .json file '" << filename << "': field 'blacklist', element " << i << " is not an integer" << std::endl;
                        goto dir_loop_end;
                    }
                }
            } else {
                    std::cerr << "Error loading .json file '" << filename << "': field 'blacklist' is missing or is not an array" << std::endl;
                    goto dir_loop_end;
            }

            // retrieve correction whitelist
            if (data.contains("correctionWhitelist") && data["correctionWhitelist"].is_array()) {

                // iterate on correction whitelist
                for (unsigned i = 0; i < data["correctionWhitelist"].size(); i++) {

                    // retrieve value
                    if (data["correctionWhitelist"][i].is_number_integer())
                        correction_whitelist.push_back(data["correctionWhitelist"][i]);
                    else {
                        std::cerr << "Error loading .json file '" << filename << "': field 'correctionWhitelist', element " << i << " is not an integer" << std::endl;
                        goto dir_loop_end;
                    }
                }
            } else {
                std::cerr << "Error loading .json file '" << filename << "': field 'correctionWhitelist' is missing or is not an array" << std::endl;
                goto dir_loop_end;
            }

            // retrieve correction blacklist
            if (data.contains("correctionBlacklist") && data["correctionBlacklist"].is_array()) {

                // iterate on correction whitelist
                for (unsigned i = 0; i < data["correctionBlacklist"].size(); i++) {

                    // retrieve value
                    if (data["correctionBlacklist"][i].is_number_integer())
                        correction_blacklist.push_back(data["correctionBlacklist"][i]);
                    else {
                        std::cerr << "Error loading .json file '" << filename << "': field 'correctionBlacklist', element " << i << " is not an integer" << std::endl;
                        goto dir_loop_end;
                    }
                }
            } else {
                std::cerr << "Error loading .json file '" << filename << "': field 'correctionBlacklist' is missing or is not an array" << std::endl;
                goto dir_loop_end;
            }

            // push loaded filter data into map
            filters[name] = Filter(id);
            for (unsigned i = 0; i < whitelist.size(); i++)
                filters[name].pushWhitelist(whitelist[i]);
            for (unsigned i = 0; i < blacklist.size(); i++)
                filters[name].pushBlacklist(blacklist[i]);
            for (unsigned i = 0; i < correction_whitelist.size(); i++)
                filters[name].pushCorrectionWhitelist(correction_whitelist[i]);
            for (unsigned i = 0; i < correction_blacklist.size(); i++)
                filters[name].pushCorrectionBlacklist(correction_blacklist[i]);
        }

        dir_loop_end:
        continue;
    }
    
    return filters;
}