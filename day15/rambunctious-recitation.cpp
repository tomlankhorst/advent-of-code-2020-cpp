#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

auto game(size_t to, std::vector<int> numbers) {
  numbers.reserve(to);
  std::unordered_map<int, int> birth;

  int i = 0;
  for (auto n : numbers)
    birth[n] = i++;

  birth.erase(numbers.back());

  for (auto turn = numbers.size(); turn < to; turn++) {
    auto spoken = numbers.back();
    auto born = birth.contains(spoken) ? birth.at(spoken) + 1 : turn;
    numbers.push_back(turn - born);
    birth[spoken] = turn - 1;
  }

  return numbers.back();
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "Provide input file\n";
    return 0;
  }

  auto file = std::ifstream (argv[1]);

  if (!file.good()) {
    std::cerr << "Could not open file\n";
    return 1;
  }

  std::string input;
  std::getline(file, input);

  std::vector<int> numbers;

  size_t pos = 0;
  for(auto to = input.find(','); to != std::string::npos; to = input.find(',', pos)) {
    numbers.push_back(std::stoi(input.substr(pos, to - pos)));
    pos = to + 1;
  }
  numbers.push_back(std::stoi(input.substr(pos)));

  auto a1 = game(2020, numbers);
  std::cout << "Part 1: " << a1 << "\n";

  auto a2 = game(30000000, numbers);
  std::cout << "Part 2: " << a2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
}