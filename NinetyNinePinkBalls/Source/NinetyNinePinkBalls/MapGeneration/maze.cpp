#include <vector>
#include <array>
#include <unordered_set>
#include <random>
#include <memory>
#include <sstream>
#include <iomanip>
#include <string>
#include <numbers>
#include <fstream>
#include <iostream>

// template<typename T>
// struct TVector{ T X; T Y; T Z; };
//
// using FVector = TVector<double>;

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
          const auto& dir2 = direction[i];
          queue.push_back({nx + dir2.x, ny + dir2.y, i, nx, ny});
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

// int main() {
//   map_t m{{10, 10, 10, 30}};
//   std::ofstream ofs("maze.tex");
//   ofs << m.latex();
//
//   return 0;
// }
