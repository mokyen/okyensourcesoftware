// https://godbolt.org/z/MaExvE38x
#include <set>
#include <stdexcept>
#include <vector>

#include <iostream>

using Options = std::set<int>;
using Grid = std::vector<std::vector<Options>>;

class Puzzle {
public:
  Puzzle(size_t size) {
    Options defaultData;
    for (auto i = 1; i <= size; i++) {
      defaultData.insert(i);
    }

    grid.resize(size);

    // std::cout << "Grid size: " << grid.size() << " Grid[0].size " <<
    // grid[0].size() << std::endl;

    for (int i = 0; i < size; i++) {
      grid[i].resize(size);
      for (int j = 0; j < size; j++) {
        grid[i][j] = defaultData;
      }
    }
    // std::cout << "Grid size: " << grid.size() << " Grid[0].size " <<
    // grid[0].size() << std::endl;
    // std::cout << "Grid[0][0][0]: " << *grid[0][0].end() << std::endl;
  }

  std::vector<Options> getRow(size_t n) { return std::vector<Options>(); }
  std::vector<Options> getColumn(size_t m) { return std::vector<Options>(); }
  std::vector<Options> getBox(size_t n) { return std::vector<Options>(); }

  void setValue(size_t row, size_t col, int value) {
    if ((value <= 0) || (value > grid.size())) {
      throw std::invalid_argument("Bad value " + std::to_string(value));
    }

    grid[row][col] = Options({value});

    for (auto &cell : grid[row]) {
      if ((cell.size() == 1) && (*cell.begin() == value)) {
        continue;
      } else {
        cell.erase(value);
      }
      //TODO erase value from column
    }
  }

  // private:
  Grid grid;
};

int main() {
  Puzzle a(9);
  size_t row = 1;
  a.setValue(row, 1, 5);

  std::cout << "val: " << *a.grid[1][1].begin() << std::endl;

  auto size = a.grid.size();
  for (auto cell : a.grid[row]) {
    for (auto it = cell.begin(); it != cell.end(); it++) {
      std::cout << *it << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "val: " << *a.grid[1][1].begin() << std::endl;
}