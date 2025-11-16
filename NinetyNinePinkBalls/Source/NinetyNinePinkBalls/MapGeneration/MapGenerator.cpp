#include "MapGenerator.h"

#include <vector>
#include <array>
#include <unordered_set>
#include <random>
#include <memory>
#include <sstream>
#include <iomanip>
#include <string>
#include <set>
#include <numbers>
#include <fstream>
#include <iostream>

using vector_t = FVector;


struct point_t {
  int x, y;

  bool operator==(const point_t& oth) const {
    return x == oth.x && y == oth.y;
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

  [[nodiscard]] bool has_single_wall() const {
    int count = 0;
    if (n) count++;
    if (s) count++;
    if (e) count++;
    if (w) count++;
    return count == 1;
  }
};

enum class wall_orientation { H, V };

struct collider_t {
  vector_t centroid;
  wall_orientation orientation;
  double length;

  collider_t(const vector_t& c, wall_orientation o, double l)
      : centroid(c), orientation(o), length(l) {}

  [[nodiscard]] std::array<vector_t, 2> get_endpoints() const {
    if (orientation == wall_orientation::H) {
      return {{
        {centroid.X - length / 2, centroid.Y, centroid.Z},
        {centroid.X + length / 2, centroid.Y, centroid.Z}
      }};
    } else {
      return {{
        {centroid.X, centroid.Y - length / 2, centroid.Z},
        {centroid.X, centroid.Y + length / 2, centroid.Z}
      }};
    }
  }

  [[nodiscard]] double get_angle() const {
    return (orientation == wall_orientation::H) ? 0.0 : std::numbers::pi / 2;
  }
};

using wall_data = std::tuple<double, double, wall_orientation>;

namespace std {
  template<>
  struct hash<wall_data> {
    size_t operator()(const wall_data& key) const {
      size_t h1 = std::hash<double>()(std::get<0>(key));
      size_t h2 = std::hash<double>()(std::get<1>(key));
      size_t h3 = std::hash<int>()(static_cast<int>(std::get<2>(key)));
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  template<>
  struct equal_to<wall_data> {
    bool operator()(const wall_data& lhs, const wall_data& rhs) const {
      return std::get<0>(lhs) == std::get<0>(rhs) &&
             std::get<1>(lhs) == std::get<1>(rhs) &&
             std::get<2>(lhs) == std::get<2>(rhs);
    }
  };
}

struct map_config_t {
  int width;
  int height;
  int segment_length;
  int threshold;
};


class map_t {
private:
  int segment_length;
  int width;
  int height;
  int threshold = 30;
  std::vector<std::vector<cell_t>> grid;
  vector_t start;
  std::vector<std::shared_ptr<collider_t>> walls;

  std::mt19937 rng{ std::random_device{}() };

public:
  [[nodiscard]] vector_t centroid() const { return start; }

  vector_t retrieve_safe_point() {
    std::uniform_int_distribution<int> dist_x(0, width - 1);
    std::uniform_int_distribution<int> dist_y(0, height - 1);

    int rx = dist_x(rng);
    int ry = dist_y(rng);

    std::uniform_int_distribution<int> offset(
      -segment_length * 0.75 / 2, segment_length * 0.75 / 2);


    return vector_t{
      rx * segment_length + segment_length / 2.0 + offset(rng),
      ry * segment_length + segment_length / 2.0 + offset(rng),
      0.0
    };
  }

  std::vector<std::shared_ptr<collider_t>>& get_walls() { return walls; }

  map_t(const map_config_t& config)
      : width(config.width), height(config.height), grid(config.height, std::vector<cell_t>(config.height)), 
        segment_length(config.segment_length), threshold(config.threshold) {
    prim(
      {std::uniform_int_distribution<int>(0, width - 1)(rng),
      std::uniform_int_distribution<int>(0, height - 1)(rng)}
    );
    random_remove_wall();
    generate_colliders();
  }

  [[nodiscard]] std::string latex() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    oss << "\\documentclass[margin=5mm,tikz]{standalone}\n"
        << "\\usepackage{tikz}\n"
        << "\\begin{document}\n";

    oss << "\\begin{tikzpicture}[scale=0.5]\n";
    for (const auto& wall : walls) {
      const auto& c = wall->centroid;
      double half = wall->length / 2.0;

      if (wall->orientation == wall_orientation::H) {
        oss << "\\draw (" << c.X - half << "," << c.Y << ") -- ("
            << c.X + half << "," << c.Y << ");\n";
      } else {
        oss << "\\draw (" << c.X << "," << c.Y - half << ") -- ("
            << c.X << "," << c.Y + half << ");\n";
      }
    }

    oss << "\\end{tikzpicture}\n\\end{document}";

    return oss.str();
  }

private:
  void prim(point_t s) {
    std::vector<std::array<int, 5>> queue;
    std::unordered_set<point_t> visited;

    visited.insert(s);

    for (int i = 0; i < direction.size(); ++i) {
      const auto& dir = direction[i];
      queue.push_back({s.x + dir.x, s.y + dir.y, i, s.x, s.y});
    }

    while(!queue.empty()) {
      std::uniform_int_distribution<int> dist(0, queue.size() - 1);
      size_t idx = dist(rng);

      auto wall = queue[idx];
      queue.erase(queue.begin() + idx);

      int nx = wall[0];
      int ny = wall[1];
      int dir = wall[2];
      int px = wall[3];
      int py = wall[4];

      point_t next{nx, ny};

      if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
          visited.find(next) == visited.end()) {
        visited.insert(next);

        remove_wall(point_t{px, py}, next, dir);

        for (int i = 0; i < direction.size(); ++i) {
          const auto& dirc = direction[i];
          queue.push_back({nx + dirc.x, ny + dirc.y, i, nx, ny});
        }
      }
    }
  }

  void random_remove_wall() {
    std::vector<std::vector<int>> density;

    for (size_t si = 0; si < grid.size() - 4; ++si) {
      for (size_t sj = 0; sj < grid[si].size() - 4; ++sj) {
        int wall_count = 0;
        for (size_t i = si; i < si + 4; ++i) {
          for (size_t j = sj; j < sj + 4; ++j) {
            wall_count += grid[i][j].n + grid[i][j].s + grid[i][j].e + grid[i][j].w;
          }
        }
        density.push_back({static_cast<int>(si), static_cast<int>(sj), wall_count});
      }
    }

    for (const auto& d : density) {
      if (d[2] > threshold) {
        std::uniform_int_distribution<int> dist_i(d[0], d[0] + 3);
        std::uniform_int_distribution<int> dist_j(d[1], d[1] + 3);
        int ri = dist_i(rng);
        int rj = dist_j(rng);

        std::vector<int> possible_walls;
        if (grid[ri][rj].n) possible_walls.push_back(0);
        if (grid[ri][rj].s) possible_walls.push_back(1);
        if (grid[ri][rj].e) possible_walls.push_back(2);
        if (grid[ri][rj].w) possible_walls.push_back(3);

        if (!possible_walls.empty()) {
          std::uniform_int_distribution<int> dist_wall(0, possible_walls.size() - 1);
          int wall_dir = possible_walls[dist_wall(rng)];

          point_t p1{ri, rj};
          point_t p2{ri + direction[wall_dir].x, rj + direction[wall_dir].y};
          
          if (p2.x >= 0 && p2.x < width && p2.y >= 0 && p2.y < height)
            remove_wall(p1, p2, wall_dir);
        }
      }
    }
  }

  void remove_wall(point_t p1, point_t p2, int dir) {
    if (dir == 0) {
      grid[p1.x][p1.y].n = false;
      grid[p2.x][p2.y].s = false;
    } else if (dir == 1) {
      grid[p1.x][p1.y].s = false;
      grid[p2.x][p2.y].n = false;
    } else if (dir == 2) {
      grid[p1.x][p1.y].e = false;
      grid[p2.x][p2.y].w = false;
    } else if (dir == 3) {
      grid[p1.x][p1.y].w = false;
      grid[p2.x][p2.y].e = false;
    }
  }

  void generate_colliders() {
    std::unordered_set<wall_data> unique_walls;
    walls.clear();

    auto is_border = [this](size_t i, size_t j) {
      return i == 0 || j == 0 || i == height - 1 || j == width - 1;
    };

    for (size_t i = 0; i < grid.size(); ++i) {
      for (size_t j = 0; j < grid[i].size(); ++j) {
        const auto& cell = grid[i][j];
        if (cell.has_single_wall() && !is_border(i, j))
          continue;

        double iw = i * segment_length;
        double jw = j * segment_length;

        if (cell.n || i == 0) {
          unique_walls.insert({
            jw + segment_length / 2.0, iw, 
            wall_orientation::H});
        }
        if (cell.s || i == height - 1) {
          unique_walls.insert({
            jw + segment_length / 2.0, iw + segment_length, 
            wall_orientation::H});
        }
        if (cell.e || j == width - 1) {
          unique_walls.insert({
            jw + segment_length, iw + segment_length / 2.0, 
            wall_orientation::V});
        }
        if (cell.w || j == 0) {
          unique_walls.insert({
            jw, iw + segment_length / 2.0, 
            wall_orientation::V});
        }
      }
    }

    for (const auto& wall : unique_walls) {
      double x = std::get<0>(wall);
      double y = std::get<1>(wall);
      wall_orientation o = std::get<2>(wall);

      walls.push_back(std::make_shared<collider_t>(
        vector_t{x, y, 0.0},
        o,
        static_cast<double>(segment_length)));
    }
  }
};

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

    vector_t retrieve_safe_point(int segment_length) {
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
    placement_t placer;

    map_t(int w, int h, int segment_length, std::vector<piece_t> pieces)
      : width(w), height(h), cell_size(segment_length), placer(w, h, pieces) {
      placer.solve();
      walls = placer.retrieve_walls(cell_size);
    }

    vector_t retrieve_safe_point() { return placer.retrieve_safe_point(cell_size); }
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
  
  std::vector<piece_t> pieces = {
        { 
          { 
            {0,5},
            {0,4},
            {0,3},
            {0,2},
            {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1},
            {0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {5,0},
          },1, 10 
        },
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
    },
    {
      {
        {0,2}, {1,2}, {2,2},
        {0,1}, {1,1}, {2,1},
        {0,0}, {1,0}, {2,0}
      }, 1, 4
    },
    {
        {
          {0,2},        {2,2},
          {0,1}, {1,1}, {2,1},
          {0,0},        {2,0}
        }, 2, 10
      },
      {
        {
          {0, 0}
        }, 3, 10 
      },
      {
        {
          {0,0}, {1,0}, {2,0}
        }, 4, 10
      },
      {
        {
          {0,2},
          {0,1},
          {0,0}, {1,0}, {2,0}
        }, 5, 10
      }
  };
};

namespace
{
   const FVector MAP_OFFSET = {200.f, 200.f, 0.f};

}

bool AMapGenerator::IsMapReady() const
{
	return _isMapReady;
}

FVector AMapGenerator::GetPlayerStartPosition() const
{
	return _playerStartPosition + FVector::UpVector * 200.f;
}

void AMapGenerator::BeginPlay()
{
  Super::BeginPlay();
  GenerateMap();
}

map_config_t AMapGenerator::GetConfig() const
{
  map_config_t config;
  config.height = MapHeight;
  config.width = MapWidth;
  config.segment_length = TileSize;
  config.threshold = Threshold;
  return config;
}

std::vector<std::shared_ptr<collider_t>> AMapGenerator::CalculatePositionsWithMap()
{
	map_config_t config = GetConfig();
	
	map_t map {config};
	auto walls = map.get_walls();
  _playerStartPosition = map.retrieve_safe_point();

  _ghostPosition = map.retrieve_safe_point();
  int32 tries = 0;
  while (FVector::PointsAreNear(_ghostPosition, _playerStartPosition, 5000.f) && tries < 5000)
  {
    _ghostPosition = map.retrieve_safe_point();
    ++tries;
  }
	
	return walls;
}

std::vector<std::shared_ptr<pavage::collider_t>> AMapGenerator::CalculatePositionsWithPavage()
{
  map_config_t config = GetConfig();

  pavage::map_t map {config.width, config.height, config.segment_length, pavage::pieces};
  _playerStartPosition = map.retrieve_safe_point();

  _ghostPosition = map.retrieve_safe_point();
  int32 tries = 0;
  while (FVector::PointsAreNear(_ghostPosition, _playerStartPosition, 5000.f) && tries < 5000)
  {
    _ghostPosition = map.retrieve_safe_point();
    ++tries;
  }
  
  _ghostPosition += MAP_OFFSET.X * FVector::UpVector;
  
  return map.get_walls();
}

void AMapGenerator::SetMapReady()
{
	_isMapReady = true;
	OnMapReady.Broadcast();
}

void AMapGenerator::GenerateMap()
{
	SpawnFloor();
  if (UsesPavage)
  {
    SpawnWallsAndDoors();
  }
  else
  {
    SpawnWalls();
  }
	PlaceObstacle();
  SpawnBalls();
	
	SetMapReady();
}

void AMapGenerator::SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation)
{
	ComponentToSpawn->SetRelativeLocation(Position);
	ComponentToSpawn->SetRelativeRotation(Rotation);
  ComponentToSpawn->SetRelativeScale3D(FVector(Scale, Scale, 1.f));
			
	ComponentToSpawn->RegisterComponent();
	_spawnedMapElements.Add(ComponentToSpawn);
}

void AMapGenerator::PlaceObstacle()
{
    // TODO: Implement
}

void AMapGenerator::SpawnFloor()
{
	for (int i = 0; i< MapWidth; i++)
	{
		for (int j = 0; j< MapHeight; j++)
		{
			UStaticMeshComponent* spawnedFloorTile = NewObject<UStaticMeshComponent>(this);
			spawnedFloorTile->SetStaticMesh(FloorMeshes[0]);
			const auto position = FVector(TileSize * i, TileSize * j, 0.f) + MAP_OFFSET * Scale;
			SpawnMapElement(spawnedFloorTile, position);
		}
	}
}

void AMapGenerator::SpawnWalls()
{
	std::vector<std::shared_ptr<collider_t>> wallPositions = CalculatePositionsWithMap();
	
	for (const auto& wallPosition : wallPositions)
	{
		UStaticMeshComponent* wallToSpawn = NewObject<UStaticMeshComponent>(this);
	
		wallToSpawn->SetStaticMesh(WallMeshes[0]);
		
		FRotator rotation = wallPosition->orientation == wall_orientation::V
			? FRotator{} 
			: FRotator(0, 90.f, 0.f);
		
		SpawnMapElement(wallToSpawn, wallPosition->centroid, rotation);
	}
}

void AMapGenerator::SpawnWallsAndDoors()
{
  std::vector<std::shared_ptr<pavage::collider_t>> wallPositions = CalculatePositionsWithPavage();
	
  for (const auto& wallPosition : wallPositions)
  {
    UStaticMeshComponent* wallToSpawn = NewObject<UStaticMeshComponent>(this);
	
    UStaticMesh* mesh = wallPosition->is_door ? DoorMeshes[0] :WallMeshes[0];
    wallToSpawn->SetStaticMesh(mesh);
		
    FRotator rotation = wallPosition->orientation == wall_orientation::V
      ? FRotator{} 
    : FRotator(0, 90.f, 0.f);
		
    SpawnMapElement(wallToSpawn, wallPosition->centroid, rotation);
  }
}

void AMapGenerator::SpawnBalls()
{
  for (int i = 0; i < _ballCount; ++i)
  {
      float posx = FMath::RandRange(0.f, MapWidth * TileSize);
      float posy = FMath::RandRange(0.f, MapHeight * TileSize);
      FVector ballPosition = FVector(posx, posy, 150.f);
      GetWorld()->SpawnActor(_ballActorClass, &ballPosition);
  }
  GetWorld()->SpawnActor(_hauntedBallActorClass, &_ghostPosition);
}
