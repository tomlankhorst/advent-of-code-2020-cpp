#include <iostream>

static constexpr uint64_t subject_transform_step(uint64_t subject, uint64_t value) {
  value *= subject;
  return value % 20201227ull;
}

static constexpr uint64_t subject_transform(uint64_t subject, uint64_t loop_size) {
  auto value = uint64_t { 1 };
  for (size_t loop = 0; loop < loop_size; loop++)
    value = subject_transform_step(subject, value);
  return value;
}

static constexpr uint64_t subject_transform_until(uint64_t subject, uint64_t public_key) {
  auto value = uint64_t { 1 };
  size_t loop;
  for (loop = 0; value != public_key; loop++)
    value = subject_transform_step(subject, value);
  return loop;
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " {card-public-key} {door-public-key}\n";
    return 1;
  }

  uint64_t card_public_key = std::stoull(argv[1]),
    door_public_key = std::stoull(argv[2]);

  auto card_loop_size = subject_transform_until(7, card_public_key),
    door_loop_size = subject_transform_until(7, door_public_key);

  auto a1 = subject_transform(card_public_key, door_loop_size);

  if (a1 != subject_transform(door_public_key, card_loop_size)) {
    std::cerr << "Encryption keys do not match\n";
    return 1;
  }

  std::cout << "Part 1: " << a1 << "\n";

  std::cout << a1 << "\n";
}