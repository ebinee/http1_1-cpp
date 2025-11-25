#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

int main() {
    json superHeroes_source = {
            {"squadName", "Super hero squad"},
            {"homeTown", "Metro City"},
            {"formed", 2016},
            {"secretBase", "Super tower"},
            {"active", true},
            {"members", json::array({
                json{
                    {"name", "Molecule Man"},
                    {"age", 29},
                    {"secretIdentity", "Dan Jukes"},
                    {"powers", json::array({
                        "Radiation resistance",
                        "Turning tiny",
                        "Radiation blast"
                    })}
                },
                json{
                    {"name", "Madame Uppercut"},
                    {"age", 39},
                    {"secretIdentity", "Jane Wilson"},
                    {"powers", json::array({
                        "Million tonne punch",
                        "Damage resistance",
                        "Superhuman reflexes"
                    })}
                },
                json{
                    {"name", "Eternal Flame"},
                    {"age", 1000000},
                    {"secretIdentity", "Unknown"},
                    {"powers", json::array({
                        "Immortality",
                        "Heat Immunity",
                        "Inferno",
                        "Teleportation",
                        "Interdimensional travel"
                    })}
                }
            })}
    };

    std::string superHeroes = superHeroes_source.dump(4);
    std::cout << superHeroes << std::endl;

    return 0;
}