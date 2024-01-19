#include "../include/filter.hpp"

bool isIn(std::vector<int> &v, int x) {
    for (unsigned i = 0; i < v.size(); i++)
        if (v[i] == x)
            return true;
    return false;
}

Filter::Filter(int id) : _id(id) {}
Filter::Filter() : _id(0) {}

Filter& Filter::pushWhitelist(int x) {
    _whitelist.push_back(x);
    return *this;
}
Filter& Filter::pushBlacklist(int x) {
    _blacklist.push_back(x);
    return *this;
}

void Filter::clearLists() {
    _whitelist.clear();
    _blacklist.clear();
}

std::vector<int> &Filter::getWhiteList() {
    return _whitelist;
}

std::vector<int> &Filter::getBlackList() {
    return _blacklist;
}

int &Filter::getID() {
    return _id;
}

FilterState::FilterState(Filter *filter) : _filter(filter) {}
FilterState::FilterState() : _filter(nullptr) {}

void FilterState::setFilter(Filter *filter) { 
    _filter = filter;
}

bool FilterState::pass(int x) {
    // check if blacklist exists; if it does, check if x is in it
    std::vector<int> &blacklist = _filter->getBlackList();
    if (blacklist.size() > 0)
        if (isIn(blacklist, x))
            return false;
    
    // check if whitelist exists; if it does, check if x is in it
    std::vector<int> &whitelist = _filter->getWhiteList();
    if (whitelist.size() > 0) {
        if (isIn(whitelist, x))
            return true;
        else
            return false;
    }
    
    // empty filter, or only blacklist exists and x is not in it
    return true;
}

int FilterState::id() { return _filter->getID(); }

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

            // push loaded filter data into map
            filters[name] = Filter(id);
            for (unsigned i = 0; i < whitelist.size(); i++)
                filters[name].pushWhitelist(whitelist[i]);
            for (unsigned i = 0; i < blacklist.size(); i++)
                filters[name].pushBlacklist(blacklist[i]);
        }

        dir_loop_end:
        continue;
    }

    return filters;
}