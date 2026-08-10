#pragma once

#include "gamepad-types.hpp"
#include <vector>

namespace ogh {

class ConfigStore {
public:
    static bool exists();
    static std::vector<Mapping> defaults();
    static std::vector<Mapping> load();
    static bool save(const std::vector<Mapping> &mappings);
};

} // namespace ogh
