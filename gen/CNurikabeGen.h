class CNurikabeGen {
 private:
  struct Cell {
  };

 public:
  CNurikabeGen() { }

  void generate(int rows, int cols);

  Cell &getCell(int row, int col);

 private:
  int               rows_, cols_;
  std::vector<Cell> cells_;
};
