#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <random>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <numbers>
#include <set>
#include <memory>
#include <iomanip>

template<typename T>
struct TVector{ T X; T Y; T Z; };
using FVector = TVector<double>;
using vector_t = FVector;


struct point_t {
  int x, y;
  bool operator==(const point_t& oth) const {
    return x == oth.x && y == oth.y;
  }
  bool operator<(const point_t& oth) const {
    if (x != oth.x) return x < oth.x;
    return y < oth.y;
  }
};

namespace std {
  template<>
  struct hash<point_t> {
    size_t operator()(const point_t& p) const {
      return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
  };
}

inline constexpr std::array<point_t, 4> direction = {{
  {-1, 0}, {1, 0}, {0, 1}, {0, -1}
}};

struct cell_t {
  bool n = true, s = true, e = true, w = true;
  
  [[nodiscard]] bool is_wall(int pos) const {
    switch (pos) {
      case 0: return n;
      case 1: return s;
      case 2: return e;
      case 3: return w;
      default: return false;
    }
  }
};

enum class wall_orientation { H, V };

namespace pavage {
  using shape_t = std::vector<std::pair<int, int>>;
  using const_ref_shape_t = const shape_t&;

  struct piece_t {
    shape_t cells;
    int type;
    int max_count;
    int used_count = 0;

    auto get_variants() const {
      std::vector<shape_t> variants;
      auto current = cells;
      
      for (int flip = 0; flip < 2; ++flip) {
        for (int rot = 0; rot < 4; ++rot) {
          int minx = INT_MAX, miny = INT_MAX;
          for (auto [x, y] : current) {
            minx = std::min(minx, x);
            miny = std::min(miny, y);
          }

          shape_t normalized;
          for (auto [x, y] : current) {
            normalized.push_back({ x - minx, y - miny });
          }

          std::sort(normalized.begin(), normalized.end());
          if (std::find(variants.begin(), variants.end(), normalized) == variants.end()) {
            variants.push_back(normalized);
          }
          for (auto& [x, y] : current) { std::tie(x, y) = std::make_pair(-y, x); }
        }
        for (auto& [x, y] : current) { x = -x; }
      }
  
      return variants;
    }
  };

  struct collider_t {
    vector_t centroid;
    wall_orientation orientation;
    double length;
    bool is_door;

    collider_t(vector_t c, wall_orientation o, double l, bool d)
      : centroid(c), orientation(o), length(l), is_door(d) {}

    [[nodiscard]] std::array<vector_t, 2> get_endpoints() const {
      if (orientation == wall_orientation::H) {
        return {{
          { centroid.X - length / 2, centroid.Y, centroid.Z },
          { centroid.X + length / 2, centroid.Y, centroid.Z }
        }};
      } else {
        return {{
          { centroid.X, centroid.Y - length / 2, centroid.Z },
          { centroid.X, centroid.Y + length / 2, centroid.Z }
        }};
      }
    }

    [[nodiscard]] double get_angle() const {
      return (orientation == wall_orientation::H) ? 0.0 : std::numbers::pi / 2;
    }
  };

  class placement_t {
    int width, height, placement_id = 0;
    std::vector<piece_t> pieces;
    std::vector<std::vector<int>> grid;
    int placements = 0;
    std::mt19937 rng{std::random_device{}()};

    bool can_place(const_ref_shape_t shape, int x, int y) {
      for (auto [dx, dy] : shape) {
        int nx = x + dx, ny = y + dy;
        if (nx < 0 || nx >= width || ny < 0 || ny >= height || grid[ny][nx] != -1) { 
          return false; 
        }
      }
      return true;
    }

    bool touches_existing(const_ref_shape_t shape, int x, int y) {
      if (placements == 0) { return true; }

      for (auto [dx, dy] : shape) {
        int cx = x + dx, cy = y + dy;
        shape_t neighbors = {{cx-1,cy}, {cx+1,cy}, {cx,cy-1}, {cx,cy+1}};
        for (auto [nx, ny] : neighbors) {
          if (nx >= 0 && nx < width && ny >= 0 && ny < height && grid[ny][nx] != -1) 
            return true;
        }
      }
      return false;
    }

    void place(int piece_idx, const_ref_shape_t shape, int x, int y) {
      for (auto [dx, dy] : shape) {
        grid[y + dy][x + dx] = placement_id;
      }
      placement_id++;
      placements++;
      pieces[piece_idx].used_count++;
    }

    struct collider_gen_t {
      int width, height;
      double cell_size;

      struct segment_t {
        int x, y;
        bool is_horizontal;
        bool is_door = false;
        
        bool operator==(const segment_t& oth) const {
          return x == oth.x && y == oth.y && is_horizontal == oth.is_horizontal;
        }

        bool operator<(const segment_t& oth) const {
          if (is_horizontal != oth.is_horizontal) 
            return is_horizontal < oth.is_horizontal;
          if (is_horizontal) {
            if (y != oth.y) return y < oth.y;
            return x < oth.x;
          } else {
            if (x != oth.x) return x < oth.x;
            return y < oth.y;
          }
        } 
      };

      std::vector<segment_t> extract_wall_segments(std::vector<std::vector<int>> grid) {
        std::set<segment_t> segments;
        std::vector<std::tuple<int, int, bool>> doors;
        std::set<std::pair<int, int>> visited{};

        for (int y = 0; y < height; ++y) {
          for (int x = 0; x < width; ++x) {
            int current = grid[y][x];

            if (y == 0 || grid[y-1][x] != current) {
              if (y == 0 || visited.contains({grid[y-1][x], current}) || grid[y-1][x] == -1 || current == -1)
                segments.insert({x, y, true});
              else {
                segments.insert({x, y, true, true});
                visited.insert({grid[y-1][x], current});
                visited.insert({current, grid[y-1][x]});
              }
            }
              

            if (y == height - 1 || grid[y+1][x] != current) {
              if (y == height - 1 || visited.contains({grid[y+1][x], current}) || grid[y+1][x] == -1 || current == -1)
                segments.insert({x, y + 1, true});
              else {
                segments.insert({x, y + 1, true, true});
                visited.insert({grid[y+1][x], current});
                visited.insert({current, grid[y+1][x]});
              }
            }

            if (x == 0 || grid[y][x-1] != current) {
              if (x == 0 || visited.contains({grid[y][x-1], current}) || grid[y][x-1] == -1 || current == -1)
                segments.insert({x, y, false});
              else {
                segments.insert({x, y, false, true});
                visited.insert({grid[y][x-1], current});
                visited.insert({current, grid[y][x-1]});
              }
            }

            if (x == width - 1 || grid[y][x+1] != current) {
              if (x == width - 1 || visited.contains({grid[y][x+1], current}) || grid[y][x+1] == -1 || current == -1)
                segments.insert({x + 1, y, false});
              else {
                segments.insert({x + 1, y, false, true});
                visited.insert({grid[y][x+1], current});
                visited.insert({current,grid[y][x+1]});
              }
            }
          }
        }

        return std::vector<segment_t>(segments.begin(), segments.end());
      }

      std::vector<std::shared_ptr<collider_t>> generate_colliders(std::vector<segment_t> segments) {
        std::vector<std::shared_ptr<collider_t>> colliders;

        if (segments.empty()) return colliders;

        for (const auto& seg : segments) {
          double length = cell_size;
          vector_t centroid;

          if (seg.is_horizontal) {
            centroid = { (seg.x + 0.5) * cell_size, seg.y * cell_size, 0.0 };
          } else {
            centroid = { seg.x * cell_size, (seg.y + 0.5) * cell_size, 0.0 };
          }

          colliders.emplace_back(std::make_shared<collider_t>(
            centroid, 
            seg.is_horizontal ? wall_orientation::H : wall_orientation::V, length, 
            seg.is_door));
        }

        return colliders;
      }
    };

  public:
    placement_t(int w, int h, std::vector<piece_t> p) 
      : width(w), height(h), pieces(p), grid(h, std::vector<int>(w, -1)) {}

    vector_t retrieve_safe_point() {
      std::uniform_int_distribution<int> dist_x(0, width - 1);
      std::uniform_int_distribution<int> dist_y(0, height - 1);

      int rx, ry;

      do {
        rx = dist_x(rng);
        ry = dist_y(rng);
      } while (grid[ry][rx] == -1);

      std::uniform_int_distribution<int> offset(
      -segment_length * 0.75 / 2, segment_length * 0.75 / 2);

      return vector_t{
        rx * segment_length + segment_length / 2.0 + offset(rng),
        ry * segment_length + segment_length / 2.0 + offset(rng),
        0.0
      };
    }

    void solve() {
      bool placed = true;
      while (placed) {
        placed = false;

        std::vector<int> order(pieces.size());
        for (int i = 0; i < pieces.size(); ++i) { order[i] = i; }
        std::shuffle(order.begin(), order.end(), rng);
        
        for (int p_idx : order) {
          if (pieces[p_idx].used_count >= pieces[p_idx].max_count) { continue; }

          shape_t positions;
          for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
              positions.push_back({x, y});
            }
          }
          std::shuffle(positions.begin(), positions.end(), rng);

          auto variants = pieces[p_idx].get_variants();
          std::shuffle(variants.begin(), variants.end(), rng);

          for (auto [x, y] : positions) {
            if (placed) break;
            for (const auto& variant : variants) {
              if (can_place(variant, x, y) && touches_existing(variant, x, y)) {
                place(p_idx, variant, x, y);
                placed = true;
                break;
              }
            }
          }
          if (placed) break;
        }
      }
    }

    std::vector<std::shared_ptr<collider_t>> retrieve_walls(double cell_size) {
      collider_gen_t collider_gen{width, height, cell_size};
      auto segments = collider_gen.extract_wall_segments(grid);
      return collider_gen.generate_colliders(segments);
    }

    void display() {
      for (auto& row : grid) {
        for (int cell : row) {
          if (cell == -1) {
            std::cout << ". ";
          } else {
            std::cout << (char)('A' + cell % 26) << ' ';
          }
        }
        std::cout << "\n";
      }
    }
  };

  struct map_t {
    int width, height, cell_size;
    std::vector<std::shared_ptr<collider_t>> walls;
    pacement_t placer;

    map_t(int w, int h, int csize, std::vector<piece_t> pieces)
      : width(w), height(h), cell_size(csize), placer(w, h, pieces) {
      placer.solve();
      walls = placer.retrieve_walls(csize);
    }

    vector_t retrieve_safe_point() { return placer.retrieve_safe_point(); }
    std::vector<std::shared_ptr<collider_t>>& get_walls() { return walls; }

    [[nodiscard]] std::string latex() const {
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(2);

      oss << "\\documentclass[margin=5pt]{standalone}\n"
          << "\\usepackage{tikz}\n"
          << "\\begin{document}\n"
          << "\\begin{tikzpicture}[scale=0.5]\n";
      for (const auto& wall : walls) {
        const auto& c = wall->centroid;
        double half = wall->length / 2.0;

        std::string color = wall->is_door ? "red" : "black";

        if (wall->orientation == wall_orientation::H) {
          oss << "\\draw[" << color << "] (" << c.X - half << "," << c.Y << ") -- ("
              << c.X + half << "," << c.Y << ");\n";
        } else {
          oss << "\\draw[" << color << "] (" << c.X << "," << c.Y - half << ") -- ("
              << c.X << "," << c.Y + half << ");\n";
        }
      }
      oss << "\\end{tikzpicture}\n"
          << "\\end{document}\n";

      return oss.str();
    }
  };
};

int main() {
  using namespace pavage; 

  std::vector<piece_t> pieces = {
    // { 
    //   { 
    //     {0,5},
    //     {0,4},
    //     {0,3},
    //     {0,2},
    //     {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
    //     {0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {5,0},
    //   },1, 10 
    // },
    {
      {
        {0,5}, {1,5},               {4,5}, {5,5}, 
        {0,4}, {1,4},               {4,4}, {5,4}, 
        {0,3}, {1,3}, {2,3}, {3,3}, {4,3}, {5,3},
        {0,2}, {1,2}, {2,2}, {3,2}, {4,2}, {5,2},
        {0,1}, {1,1},               {4,1}, {5,1}, 
        {0,0}, {1,0},               {4,0}, {5,0}, 
      }, 2, 10
    },
    {
      {
                      {2,3}, {3,3}, {4,3}, {5,3},
                      {2,2}, {3,2}, {4,2}, {5,2},
        {0,1}, {1,1}, {2,1}, {3,1},
        {0,0}, {1,0}, {2,0}, {3,0},
      }, 3, 10
    }
  };

  map_t map(20, 20, 10, pieces);
  std::cout << map.latex();

  return 0;
}

