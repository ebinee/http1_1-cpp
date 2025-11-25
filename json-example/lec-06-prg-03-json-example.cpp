#include <iostream>
#include <string>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

int main() {
	std::ifstream json_in_file("lec-06-prg-03-json-example.json");
	json superHeroes;
	json_in_file >> superHeroes;

	std::cout << superHeroes["homeTown"].get<std::string>() << std::endl;
	std::cout << superHeroes["active"].get<bool>() << std::endl;
	std::cout << superHeroes["members"][1]["powers"][2].get<std::string>() << std::endl;

	return 0;
}