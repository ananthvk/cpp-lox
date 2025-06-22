#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "hashmap.hpp"

#define MAP_TYPE HashMap<std::string, std::string>
#define GET(mp, word) mp.get_ref(word)

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    std::ifstream file(argv[1]);
    if (!file)
        return 1;

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string text = ss.str();
    file.close();

    MAP_TYPE mp;
    auto start = std::chrono::high_resolution_clock::now();

    std::istringstream iss(text);
    std::string word;
    while (iss >> word)
    {
        GET(mp, word)++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "Time elapsed: " << duration << " ns\n";
    std::cout << "\"the\": " << GET(mp, "the") << "\n";
    std::cout << "\"and\": " << GET(mp, "and") << "\n";
    std::cout << "\"a\": " << GET(mp, "a") << "\n";
    std::cout << "\"is\": " << GET(mp, "is") << "\n";

    return 0;
}
