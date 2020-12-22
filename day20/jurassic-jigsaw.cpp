#include "day05/arg_input.hpp"
#include "day05/tokenize.hpp"

#include <array>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <set>
#include <cmath>

namespace ranges = std::ranges;

/**
 * A linear algebra library would really be helpful here.
 * Especially the matrix transformations, resizing and sea-monster matching took quite a few lines of code.
 */

template<size_t Size=10>
struct generic_tile_t {
  static constexpr size_t N = Size;
  size_t id = 0;
  enum class rotation_e { Rot0 = 0, Rot90, Rot180, Rot270, FlipRot0, FlipRot90, FlipRot180, FlipRot270 };
  using line_t = std::array<char, N>;
  enum class edge_e { Top = 0, Right, Bottom, Left, TopFlip, RightFlip, BottomFlip, LeftFlip };
  using edges_t = std::array<line_t, 8>;
  using corner_t = std::pair<line_t, line_t>;
  using corners_t = std::array<corner_t, 8>;
  using data_t = std::array<line_t, N>;
  data_t data = {};

  [[nodiscard]] edges_t make_edges() const {
    auto edgs = edges_t {};
    auto right = line_t {};
    ranges::transform(data, right.begin(), [](const auto& l){
      return l.back();
    });
    auto bottom = data.back();
    ranges::reverse(bottom);
    auto left = line_t {};
    ranges::transform(data, left.begin(), [](const auto& l){
      return l.front();
    });
    ranges::reverse(left);
    edgs[0] = data.front();
    edgs[1] = right;
    edgs[2] = bottom;
    edgs[3] = left;
    ranges::copy(edgs.cbegin(), edgs.cbegin() + 4, edgs.begin() + 4);
    std::swap(edgs[5], edgs[7]); // flip R-L
    ranges::for_each(edgs.begin()+4, edgs.end(), [](auto& e){
      ranges::reverse(e);
    });
    return edgs;
  }
  edges_t edges {};

  [[nodiscard]] corners_t make_corners() const {
    auto eds = edges.empty() ? make_edges() : edges;
    auto crns = corners_t {};

    for (size_t i = 0; i < 4; i++)
      crns[i] = {eds[i], eds[(i+1)%4]};
    for (size_t i = 4; i < 8; i++)
      crns[i] = {eds[i], eds[4+(i+1)%4]};

    return crns;
  }
  corners_t corners {};

  struct line_hash {
    constexpr std::size_t operator()(const line_t& line) const {
      std::size_t h = 0;
      for (auto e : line)
        h ^= std::hash<char>{}(e)  + 0x9e3779b9 + (h << 6u) + (h >> 2u);
      return h;
    }
  };
  struct corner_hash {
    constexpr std::size_t operator()(const corner_t& corner) const {
      return line_hash{}(corner.first) ^ line_hash{}(corner.second);
    }
  };
};
using tile_t = generic_tile_t<10>;
using tiles_t = std::unordered_map<size_t, tile_t>;

tiles_t read_tiles(const std::vector<std::string>& tokens) {
  auto tiles = tiles_t {};

  auto it = tokens.cbegin();
  auto tile = tile_t {};
  static constexpr auto id_prefix = std::string_view {"Tile "};
  auto row = 0;
  while (it != tokens.cend()) {
    if (it->empty()) { // make sure to end with an empty line
      if (row == 10) {
        tile.edges = tile.make_edges();
        tile.corners = tile.make_corners();
        tiles[tile.id] = tile;
      }
      row = 0;
      tile = {};
    } else if (it->starts_with(id_prefix)) {
      tile.id = std::stoull(it->substr(id_prefix.size(), it->size() - id_prefix.size()-1));
    } else {
      ranges::copy(*it, tile.data[row++].begin());
    }

    it++;
  }
  if (row != 0) {
    throw std::invalid_argument("Unprocessed data, did you end with an empty line?");
  }

  return tiles;
}

struct tile_orientation_t {
  size_t id;
  // the rotation to apply to get this to top-right / top
  tile_t::rotation_e orientation;
};

using edge_tile_catalog_t = std::unordered_multimap<tile_t::line_t, tile_orientation_t, tile_t::line_hash>;
using corner_tile_catalog_t = std::unordered_multimap<tile_t::corner_t, tile_orientation_t, tile_t::corner_hash>;

auto make_catalogs(const tiles_t& tiles) {
  auto edges = edge_tile_catalog_t {};
  auto corners = corner_tile_catalog_t {};

  ranges::for_each(tiles, [&](const auto& tile){
    auto es = tile.second.edges;
    auto cs = tile.second.corners;
    for (size_t e = 0; e < es.size(); e++) {
      auto r = static_cast<tile_t::rotation_e>(e);
      edges.insert({es.at(e), {tile.first, r}});
      corners.insert({cs.at(e), {tile.first, r}});
    }
  });

  return std::make_tuple(edges, corners);
}

using image_t = std::vector<std::vector<std::pair<size_t, tile_t::rotation_e>>>;

struct state_t {
  using tile_ids_t = std::set<size_t>;
  state_t(size_t width, size_t height, const tiles_t& tiles)
      : w{width}, h{height}, image(height, image_t::value_type(width))
  {
    for (const auto& t : tiles) {
      available.insert(t.first);
    }
  }
  void next_pos() {
    if (++col >= w) {
      col = 0;
      ++row;
    }
  }
  uint64_t magic_number() {
    auto m = uint64_t {1};
    m *= image.front().front().first;
    m *= image.front().back().first;
    m *= image.back().front().first;
    m *= image.back().back().first;
    return m;
  }
  image_t image;
  tile_ids_t available;
  size_t row = 0, col = 0;
  const size_t w = 0, h = 0;
};

constexpr auto next_edge(tile_t::rotation_e r, size_t step) {
  auto i = static_cast<int>(r);
  return 4 * ( i/4 ) + ( (i+step)%4 );
};
static_assert(next_edge(tile_t::rotation_e::FlipRot270, 3)==6);
static_assert(next_edge(tile_t::rotation_e::Rot0, 0)==0);
static_assert(next_edge(tile_t::rotation_e::Rot270, 1)==0);

template<typename CT, typename T>
auto solve_next(const tiles_t&, const edge_tile_catalog_t&,
                const corner_tile_catalog_t&, state_t,
                const CT&, const T&, tile_t::edge_e) -> std::vector<state_t>;

auto solve(const tiles_t& tiles, const edge_tile_catalog_t& edges, const corner_tile_catalog_t& corners, state_t state) -> std::vector<state_t> {
  if (state.available.empty())
    return {state};

  if (state.col == 0 && state.row == 0) {
    auto res = std::vector<state_t>{};
    ranges::for_each(state.available, [&](const auto &tile_id) {
      auto substate = state;
      substate.next_pos();
      substate.available.erase(tile_id);
      for (size_t rot = 0; rot < 8; rot++) {
        substate.image[state.row][state.col] = {tile_id, static_cast<tile_t::rotation_e>(rot)};
        auto subres = solve(tiles, edges, corners, substate);
        ranges::copy(subres, std::back_inserter(res));
      }
    });
    return res;
  } else if (state.row == 0 || state.col == 0) {
    // top- and left-most tiles
    bool is_top = state.row == 0;
    const auto &image_adjacent = state.image[is_top ? 0 : (state.row - 1)][is_top ? (state.col - 1) : 0];
    const auto &tile_adjacent = tiles.at(image_adjacent.first);
    size_t i_edge = next_edge(image_adjacent.second, is_top ? 1 : 2);
    auto adjecent_edge = tile_adjacent.edges[i_edge];
    ranges::reverse(adjecent_edge); // reverse edge to find target edge
    return solve_next(tiles, edges, corners, state, edges,
                      adjecent_edge, is_top ? tile_t::edge_e::Left : tile_t::edge_e::Top);
  } else {
    // cornered tiles
    const auto &image_top = state.image[state.row - 1][state.col];
    const auto &tile_top = tiles.at(image_top.first);
    const auto &image_left = state.image[state.row][state.col - 1];
    const auto &tile_left = tiles.at(image_left.first);

    size_t i_right_edge = next_edge(image_left.second, 1);
    size_t i_bottom_edge = next_edge(image_top.second, 2);
    auto left_top_corner = tile_t::corner_t{tile_left.edges[i_right_edge], tile_top.edges[i_bottom_edge]};
    ranges::reverse(left_top_corner.first);
    ranges::reverse(left_top_corner.second);
    return solve_next(tiles, edges, corners, state, corners, left_top_corner, tile_t::edge_e::Left);
  };
};

template<typename CT, typename T>
auto solve_next(const tiles_t& tiles, const edge_tile_catalog_t& edges,
                const corner_tile_catalog_t& corners, state_t state,
                const CT& options, const T& match, tile_t::edge_e target_edge) -> std::vector<state_t> {
  std::vector<state_t> res;
  auto[candidate_begin, candidate_end] = options.equal_range(match);
  if (candidate_begin == candidate_end)
    return res;

  ranges::for_each(candidate_begin, candidate_end, [&](const auto &next) {
    if (!state.available.contains(next.second.id))
      return;
    auto substate = state;
    substate.next_pos();
    // try to achieve the target rotation
    auto next_rotation = static_cast<tile_t::rotation_e>(next.second.orientation);
    auto rot = static_cast<tile_t::rotation_e>(next_edge(next_rotation, target_edge==tile_t::edge_e::Left ? 1 : 0));
    substate.image[state.row][state.col] = {next.second.id, rot};
    substate.available.erase(next.second.id);
    auto results = solve(tiles, edges, corners, substate);
    ranges::copy(results, std::back_inserter(res));
  });
  return res;
}

std::vector<std::vector<char>> cutout(const tile_t& tile, tile_t::rotation_e rotation) {
  auto len = tile_t::N-2;
  std::vector<std::vector<char>> cutout (len, std::vector<char>(len, 'x'));

  size_t i = 0;
  ranges::for_each(tile.data.cbegin() + 1, tile.data.cend() - 1, [&](const auto& row){
    ranges::copy(row.cbegin() + 1, row.cend() - 1, cutout.at(i++).begin());
  });

  auto steps = static_cast<int>(rotation);
  if (steps >= 4) {
    steps -= 4;
    ranges::for_each(cutout, [](auto& row){
      ranges::reverse(row);
    });
  }
  for (size_t rot = 0; rot < steps; rot++) {
    auto rotated = cutout;
    for (int i = 0; i < len; ++i) {
      for (int j = 0; j < len; ++j) {
        rotated[i][j] = cutout[j][len - i - 1];
      }
    }
    cutout = rotated;
  }

  return cutout;
}

auto sea_monster() {
  static const std::string sea_monster = R"(                  #
#    ##    ##    ###
 #  #  #  #  #  #   )";

  auto sea_monster_hashtags = ranges::count(sea_monster, '#');
  auto sea_monster_coords = std::vector<std::pair<size_t, size_t>> {};
  sea_monster_coords.reserve(sea_monster_hashtags);

  size_t x = 0, y = 0;
  ranges::for_each(sea_monster, [&x, &y, &sea_monster_coords](auto ch){
    if (ch == '\n') {
      x = 0;
      y++;
    } else {
      if (ch == '#') {
        sea_monster_coords.emplace_back(x,y);
      }
      x++;
    }
  });
  y++;

  return std::make_tuple(sea_monster_coords, sea_monster_hashtags, x, y);
}

std::optional<size_t> sea_roughness(const tiles_t& tiles, const state_t& state) {
  static constexpr auto tile_width = tile_t::N - 2;
  static auto image_size = std::make_pair(tile_width*state.w, tile_width*state.h);

  using composed_image_t = std::vector<std::vector<char>>;
  auto image = composed_image_t (image_size.second, composed_image_t::value_type (image_size.first, 'x'));

  size_t tile_y = 0;
  ranges::for_each(state.image, [&](const auto& row){
    size_t tile_x = 0;
    ranges::for_each(row, [&](const auto& tile){
      auto prepared = cutout(tiles.at(tile.first), tile.second);
      for (auto row = 0; row < prepared.size(); row++) {
        ranges::copy(prepared.at(row), image.at(tile_y * tile_width + row).begin()+tile_x*tile_width);
      }
      tile_x++;
    });
    tile_y++;
  });

  const auto& [sea_monster_coords, sea_monster_hashtags, x, y] = sea_monster();

  std::vector<std::pair<size_t, size_t>> sea_monster_locations;
  for (auto r_it = image.cbegin(); r_it < image.cend() - y; r_it++) {
    for (auto c_it = r_it->cbegin(); c_it < r_it->cend() - x; c_it++) {
      auto c_off = std::distance(r_it->cbegin(), c_it);
      if (ranges::all_of(sea_monster_coords, [&](const auto& coord){
        auto& [x,y] = coord;
        return (r_it+y)->at(c_off+x) == '#';
      })) {
        sea_monster_locations.emplace_back(c_off, std::distance(image.cbegin(), r_it));
      }
    }
  }

  if (sea_monster_locations.empty())
    return std::nullopt;

  auto image_hashtags = std::accumulate(image.cbegin(), image.cend(), 0, [](auto a, const auto& r){
    return a + ranges::count(r, '#');
  });

  return image_hashtags - sea_monster_locations.size() * sea_monster_hashtags;
}

auto main(int argc, char* argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " {path-to-file}" << std::endl;
    return 1;
  }

  auto file = get_input(argc, argv);

  if (std::holds_alternative<int>(file))
    return std::get<int>(file);

  auto& input = std::get<std::ifstream>(file);

  auto tokens = tokenize(input);

  auto tiles = read_tiles(tokens);

  // make LUTs
  auto [edges, corners] = make_catalogs(tiles);

  // try a logical image size: square N x N
  size_t x = std::sqrt(tiles.size());
  auto state = state_t { x, x, tiles };
  auto res = solve(tiles, edges, corners, state);

  auto a1 = res.front().magic_number();
  std::cout << "Part 1: " << a1 << "\n";

  size_t a2;
  for (const auto& r : res) {
    auto rough = sea_roughness(tiles, r);
    if (rough) {
      a2 = *rough;
      std::cout << "Part 2: " <<  a2 << "\n";
      break;
    }
  }

  std::cout << a1 << "\n" << a2 << "\n";
}