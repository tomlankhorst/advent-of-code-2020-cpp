#include "day05/tokenize.hpp"

#include <iostream>
#include <map>
#include <cstdint>
#include <fstream>
#include <variant>
#include <algorithm>
#include <numeric>

using sparse_mem_t = std::map<size_t, uint64_t>;

struct update_bitmask {
  std::string mask;
  [[nodiscard]] uint64_t value_mask(uint64_t value) const {
    return (value | make_mask('1')) & ~make_mask('0');
  }
  [[nodiscard]] auto addr_mask(uint64_t value) const {
    return std::make_pair(value & make_mask('0') | make_mask('1'), make_mask('X'));
  }
  [[nodiscard]] uint64_t make_mask(char cmp) const {
    return std::accumulate(mask.cbegin(), mask.cend(), 0ull, [&cmp](uint64_t acc, char c){
      acc <<= 1ull;
      if (c == cmp)
        acc |= 1ull;
      return acc;
    });
  }
};

struct write_memory {
  size_t addr;
  size_t value;
};

using instruction_t = std::variant<std::monostate, update_bitmask, write_memory>;
std::ostream& operator<<(std::ostream& os, const instruction_t& i) {
  if (std::holds_alternative<update_bitmask>(i)) {
    auto ub = std::get<update_bitmask>(i);
    os << "mask = " << ub.mask;
  } else {
    auto wm = std::get<write_memory>(i);
    os << "mem[" << wm.addr << "] = " << wm.value;
  }
  return os;
}

using instructions_t = std::vector<instruction_t>;

namespace ranges = std::ranges;

auto read_instructions(std::vector<std::string>& tokens) {
  instructions_t instructions(tokens.size());

  ranges::transform(tokens, instructions.begin(), [](const std::string &line) -> instruction_t {
    if (line[1] == 'a') { // m[a]sk

      return update_bitmask {line.substr(7, 36)};
    } else if (line[1] == 'e') { // m[e]m
      int brack_pos = line.find(']', 4);
      return write_memory {
          .addr = std::stoull(line.substr(4, brack_pos-4)),
          .value = std::stoull(line.substr(brack_pos+4)),
      };
    }

    return std::monostate {};
  });

  return instructions;
}

template<typename write_strategy>
struct vm {
  sparse_mem_t mem;
  update_bitmask mask;

  void run(const std::monostate&) {}
  void run(const update_bitmask& ins) {
    mask = ins;
  }
  void run(const write_memory& ins) {
    write_strategy{}(*this, ins);
  }
  [[nodiscard]] uint64_t sum() const {
    return std::accumulate(mem.cbegin(), mem.cend(), uint64_t{0}, [](uint64_t acc, const auto& el) {
      return acc + el.second;
    });
  }
};

struct value_mask_strategy {
  void operator()(vm<value_mask_strategy> &vm, const write_memory &ins) {
    vm.mem[ins.addr] = vm.mask.value_mask(ins.value);
  }
};

struct address_decoder_strategy {
  void operator()(vm<address_decoder_strategy>& vm, const write_memory &ins) {
    const auto& [addr, floating] = vm.mask.addr_mask(ins.addr);
    write(vm, addr, floating, ins.value);
  }

  static void write(vm<address_decoder_strategy>& vm, uint64_t address, uint64_t floating, uint64_t value) {
    if (!floating) {
      vm.mem[address] = value;
    } else {
      auto mask = hibitmask(floating);
      floating &= ~mask;
      write(vm, address | mask, floating, value);
      write(vm, address & ~mask, floating, value);
    }
  }
  static constexpr size_t hibitmask(uint64_t value) {
    if (value == 0)
      return 0;
    size_t msb = 0;
    value /= 2;
    while (value) {
      value /= 2;
      msb++;
    }
    return 1ull << msb;
  }
};
static_assert(address_decoder_strategy::hibitmask(0b0) == 0);
static_assert(address_decoder_strategy::hibitmask(0b1) == 0b1);
static_assert(address_decoder_strategy::hibitmask(0b10) == 0b10);
static_assert(address_decoder_strategy::hibitmask(0b1010) == 0b1000);

template<typename S>
std::ostream& operator<<(std::ostream& os, const vm<S>& machine) {
  os << "VM (mask: " << machine.mask << ", memory: " << machine.mem.size() << " elements sum to " << machine.sum() << ")";
  return os;
}

template<typename S>
auto execute(const instructions_t& program, vm<S> machine) {
  ranges::for_each(program, [&machine](const auto& i){
    std::visit([&machine](auto &&i){
      machine.run(std::forward<decltype(i)>(i));
    }, i);
  });
  return machine;
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "usage " << argv[0] << " path-to-input" << std::endl;
    return 0;
  }

  std::string_view path = argv[1];

  auto file = std::ifstream{path.begin()};

  if (!file.good()) {
    std::cout << "Could not open " << path << std::endl;
    return 1;
  }

  auto tokens = tokenize(file);

  auto program = read_instructions(tokens);

  auto machine1 = execute(program, vm<value_mask_strategy>{});
  auto a1 = machine1.sum();

  std::cout << machine1 << "\n";

  auto machine2 = execute(program, vm<address_decoder_strategy>{});
  auto a2 = machine2.sum();

  std::cout << machine2 << "\n";

  std::cout << a1 << "\n" << a2 << "\n";
};