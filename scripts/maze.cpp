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

template<typename T>
struct TVector{ T X; T Y; T Z; };

using FVector = TVector<double>;

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


class map_t {
private:
  int size;
  int width;
  int height;
  std::vector<std::vector<cell_t>> grid;
  vector_t start;
  std::vector<std::unique_ptr<collider_t>> walls;

  std::mt19937 rng{ std::random_device{}() };

public:
  [[nodiscard]] vector_t centroid() const { return start; }

  map_t(int w, int h)
      : width(w), height(h), grid(h, std::vector<cell_t>(w)),
        start{static_cast<double>(w) / 2.0, static_cast<double>(h) / 2.0, 0.0} {
    prim(point_t{0, 0});
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
          const auto& dir = direction[i];
          queue.push_back({nx + dir.x, ny + dir.y, i, nx, ny});
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
    walls.clear();

    for (size_t i = 0; i < grid.size(); ++i) {
      for (size_t j = 0; j < grid[i].size(); ++j) {
        const auto& cell = grid[i][j];
        double iw = i * height;
        double jw = j * width;


        if (cell.n) {
          walls.push_back(std::make_unique<collider_t>(
            vector_t{jw + width / 2.0, iw, 0.0},
            wall_orientation::H,
            static_cast<double>(width)));
        }
        if (cell.s) {
          walls.push_back(std::make_unique<collider_t>(
            vector_t{jw + width / 2.0, iw + height, 0.0},
            wall_orientation::H,
            static_cast<double>(width)));
        }
        if (cell.e) {
          walls.push_back(std::make_unique<collider_t>(
            vector_t{jw + width, iw + height / 2.0, 0.0},
            wall_orientation::V,
            static_cast<double>(height)));
        }
        if (cell.w) {
          walls.push_back(std::make_unique<collider_t>(
            vector_t{jw, iw + height / 2.0, 0.0},
            wall_orientation::V,
            static_cast<double>(height)));
        }
      }
    }
  }
};


int main() {
  map_t m{20, 20};
  std::ofstream ofs("maze.tex");
  ofs << m.latex();

  return 0;
}
