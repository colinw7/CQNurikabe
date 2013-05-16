#ifndef CNurikabe_H
#define CNurikabe_H

#include <cstdlib>

#include <vector>
#include <set>
#include <map>
#include <iostream>

#define BLACK_REGION_CONSTRAINT ((CNurikabe::Region *) 0x1)

class CNurikabe {
 public:
  // coordinate (sorted by row then col)
  struct Coord {
    Coord(int row1=-1, int col1=-1) :
     row(row1), col(col1) {
    }

    friend bool operator==(const Coord &c1, const Coord &c2) {
      return (c1.row == c2.row && c1.col == c2.col);
    }

    friend bool operator!=(const Coord &c1, const Coord &c2) {
      return ! (c1 == c2);
    }

    friend bool operator<(const Coord &c1, const Coord &c2) {
      return (c1.row < c2.row || (c1.row == c2.row && c1.col < c2.col));
    }

    friend bool operator>(const Coord &c1, const Coord &c2) {
      return (c1.row > c2.row || (c1.row == c2.row && c1.col > c2.col));
    }

    friend bool operator>=(const Coord &c1, const Coord &c2) {
      return (c1.row > c2.row || (c1.row == c2.row && c1.col >= c2.col));
    }

    Coord getN() const { return Coord(row - 1, col    ); }
    Coord getS() const { return Coord(row + 1, col    ); }
    Coord getE() const { return Coord(row    , col + 1); }
    Coord getW() const { return Coord(row    , col - 1); }

    int dist(const Coord &coord) const {
      int dr = abs(row - coord.row);
      int dc = abs(col - coord.col);

      return dr + dc + 1;
    }

    bool touches(const Coord &coord) const {
      int dr = abs(row - coord.row);
      int dc = abs(col - coord.col);

      return (dr + dc == 1);
    }

    bool cornerTouches(const Coord &coord) const {
      int dr = abs(row - coord.row);
      int dc = abs(col - coord.col);

      return (dr == 1 && dc == 1);
    }

    friend std::ostream &operator<<(std::ostream &os, const Coord &c) {
      c.print(os);

      return os;
    }

    void print(std::ostream &os=std::cout, bool newline=false) const {
      os << "(" << row << "," << col << ")";
      if (newline) os << std::endl;
    }

    std::string str() const;

    int row;
    int col;
  };

  typedef std::set<Coord> Coords;

  typedef std::vector<Coord>  CoordArray;
  typedef std::vector<Coords> CoordsArray;

  typedef std::pair<Coords,Coords> CoordsPair;

  class Grid;
  class Region;
  class Pool;
  class Island;
  class Gap;

  struct RegionCmp {
    bool operator()(const Region *a, const Region *b) {
      if      (a->getValue() <  b->getValue())
        return true;
      else if (a->getValue() >  b->getValue())
        return false;
      else
        return (a->getCoord() < b->getCoord());
    }
  };

  typedef std::set<Pool *>               Pools;
  typedef std::set<Gap *>               Gaps;
  typedef std::set<Region *, RegionCmp> Regions;

  class Cell {
   public:
    enum Value {
      WHITE   = -1,
      BLACK   = -2,
      UNKNOWN = -3
    };

   public:
    Cell(Grid *grid, int value, const Coord &coord);

    static int charToValue(char c);

    Grid *getGrid() const { return grid_; }

    const Coord &getCoord() const { return coord_; }

    void init();

    void setValue(int value);
    void setSolution(int solution);

    bool isUnknown() const;
    bool isWhite  () const;
    bool isBlack  () const;
    bool isNumber () const;

    int getNumber() const;

    bool isNumber(int value) const;

    void setWhite();
    void setBlack();

    bool isUnknownOrWhite() const { return isUnknown() || isWhite(); }
    bool isUnknownOrBlack() const { return isUnknown() || isBlack(); }

    bool isNumberOrWhite() const { return isNumber() || isWhite(); }

    bool isSolvedWhite() const { return solution_ == WHITE; }

    bool isSolvedNumberOrWhite() const { return isNumber() || isSolvedWhite(); }

    Cell *getLShapeCornerCell() const;

    //------

    void setRegion(Region *region);

    Region *getRegion() const { return region_; }

    bool inRegion() const { return (region_ != NULL); }

    bool inOtherRegion(Region *region) const { return (region_ != NULL && region_ != region); }

    //------

    Pool *getPool() const;

    bool inPool() const { return (pool_ != NULL); }

    bool inOtherPool(const Pool *pool) const { return (pool_ != NULL && pool_ != pool); }

    //------

    Island *getIsland() const { return island_; }

    bool inIsland() const { return (island_ != NULL); }

    //------

    Gap *getGap() const { return gap_; }

    bool inGap() const { return (gap_ != NULL); }

    //------

    void setRegionConstraint(Region *region);

    bool isBlackRegionConstraint() const {
      return (region_constraint_ == BLACK_REGION_CONSTRAINT);
    }

    Region *getRegionConstraint() const {
      if (! isBlackRegionConstraint())
        return region_constraint_;
      else
        return NULL;
    }

    bool canBeInRegion(Region *region) {
      return region_constraint_ == NULL || region_constraint_ == region;
    }

    //------

    void reset();

    void resetPointers();

    void resetPoolPointer();

    //------

    Cell *getN (int count=1) const;
    Cell *getNE(int count=1) const;
    Cell *getE (int count=1) const;
    Cell *getSE(int count=1) const;
    Cell *getS (int count=1) const;
    Cell *getSW(int count=1) const;
    Cell *getW (int count=1) const;
    Cell *getNW(int count=1) const;

    //------

    void buildPool();
    void buildIsland();
    void buildGap();

    //------

    int dist(Cell *cell) const {
      return getCoord().dist(cell->getCoord());
    }

    bool touches(Cell *cell) const {
      return getCoord().touches(cell->getCoord());
    }

    bool cornerTouches(Cell *cell) const {
      return getCoord().cornerTouches(cell->getCoord());
    }

    //------

    void getRegions(Regions &regions) const;

    void getConnectedCoords(Coords &coords);

   private:
    Grid   *grid_;
    Coord   coord_;
    int     value_;
    int     solution_;
    Region *region_constraint_;
    Region *region_;
    Pool   *pool_;
    Island *island_;
    Gap    *gap_;
  };

  typedef std::set<Cell *>    Cells;
  typedef std::vector<Cell *> CellArray;

  struct Solution {
    Solution(const Coords &coords=Coords()) :
     id(0), icoords(coords), ocoords(), valid(true) {
      calcHash();
    }

    void checkValid(Region *region) const;
    void checkValid(Grid *grid) const;

    bool checkValid1(Grid *grid) const;

    friend bool operator==(const Solution &s1, const Solution &s2) {
      if (s1.icoords.size() != s2.icoords.size()) return false;

      if (s1.hash != s2.hash) return false;

      Coords::const_iterator p1, p2, pe;

      for (p1 = s1.icoords.begin(), p2 = s2.icoords.begin(), pe = s1.icoords.end();
             p1 != pe; ++p1, ++p2)
        if (*p1 != *p2) return false;

      return true;
    }

    friend bool operator<(const Solution &s1, const Solution &s2) {
      if (s1.icoords.size() < s2.icoords.size()) return true;
      if (s1.icoords.size() > s2.icoords.size()) return false;

      if (s1.hash < s2.hash) return true;
      if (s1.hash > s2.hash) return false;

      Coords::const_iterator p1, p2, pe;

      for (p1 = s1.icoords.begin(), p2 = s2.icoords.begin(), pe = s1.icoords.end();
             p1 != pe; ++p1, ++p2) {
        if (*p1 < *p2) return true;
        if (*p1 > *p2) return false;
      }

      return false;
    }

    void calcHash() {
      hash = 0;

      int n = 0;

      Coords::const_iterator p1, p2;

      for (p1 = icoords.begin(), p2 = icoords.end(); p1 != p2; ++p1, ++n) {
        const Coord &coord = *p1;

        hash ^= coord.row;
        hash ^= coord.col;

        if (n > 8) break;
      }
    }

    void print() const;

    void print(std::ostream &os) const {
      Coords::const_iterator p1, p2;

      for (p1 = icoords.begin(), p2 = icoords.end(); p1 != p2; ++p1)
        os << " " << *p1;
    }

    int    id;
    int    hash;
    Coords icoords;
    Coords ocoords;
    Coords blackCoords;
    Coords whiteCoords;
    bool   valid;
  };

  typedef std::set<Solution> Solutions;

  typedef std::map<int,Solutions> SolutionsMap;

  struct OneWhiteConstraint {
    OneWhiteConstraint(const Coords &coords1) :
     coords(coords1) {
    }

    Coords coords;

    void print(std::ostream &os) const {
      Coords::const_iterator p1, p2;

      for (p1 = coords.begin(), p2 = coords.end(); p1 != p2; ++p1)
        os << " " << *p1;
    }
  };

  typedef std::vector<OneWhiteConstraint> OneWhiteConstraints;

  struct OneBlackConstraint {
   public:
    OneBlackConstraint(const Coords &coords1) :
     coords(coords1) {
    }

    Coords coords;

    void print(std::ostream &os) const {
      Coords::const_iterator p1, p2;

      for (p1 = coords.begin(), p2 = coords.end(); p1 != p2; ++p1)
        os << " " << *p1;
    }
  };

  typedef std::vector<OneBlackConstraint> OneBlackConstraints;

  class Region {
   public:
    Region(Grid *grid, Cell *cell);

    Grid *getGrid() const { return grid_; }

    int getValue() const { return cell_->getNumber(); }

    Cell *getNumberCell() const { return cell_; }

    const Coord &getCoord() const { return cell_->getCoord(); }

    const Coords &getCoords() const { return coords_; }

    void getCells(Cells &cells) const;

    int size() const { return coords_.size(); }

    bool isComplete() const { return ! partial_; }

    void addCell(Cell *cell);

    const Solution &getSolution() const { return solution_; }

    void addRegionLocked();

    void setMaxDepth(int maxDepth) { maxDepth_ = maxDepth; }

    void checkConnectCoords(Coords &coord);

    Solutions &getSolutions();

    bool buildSolutionsWithAllCoords(Solutions &solutions, Coords &allCoords);

    bool buildSolutions(Solutions &solutions);

    bool buildSolutions(Coords &coords, Solutions &solutions);

    void checkSolutions(const Solutions &solutions);

    void build();

    void solve();

    bool getConstrainedWhites(const Coords &coords, Coords &unknownCoords);
    bool checkConstrainedBlacks(const Coords &coords);

    bool isValid() const;

    bool hasValidSolution() const;

    void addOneWhiteConstraint(const Coords &coords);
    void addOneBlackConstraint(const Coords &coords);

    void filterWhiteCoords(Coords &unknownCoords);

    void printConstraints();

    void setChanged();

    void print() const;

    void print(std::ostream &os) const;

   private:
    Grid         *grid_;
    Cell         *cell_;
    Coords        coords_;
    bool          partial_;
    Solutions     solutions_;
    bool          solutionsValid_;
    Solution      solution_;
    SolutionsMap  solutionsMap_;
    int           maxDepth_;

    OneWhiteConstraints oneWhiteConstraints_;
    OneBlackConstraints oneBlackConstraints_;
  };

  struct CellDistCmp {
    CellDistCmp(Cell *cell1) :
     cell(cell1) {
    }

    bool operator()(const Cell *a, const Cell *b) {
      if      (a->dist(cell) <  b->dist(cell))
        return true;
      else if (a->dist(cell) == b->dist(cell))
        return (a < b);
      else
        return false;
    }

    Cell *cell;
  };

  typedef std::set<Cell *, CellDistCmp> DistCells;

  class Pool {
   public:
    Pool(Grid *grid);

    void reset();

    bool empty() const { return coords_.empty(); }

    void addCoord(const Coord &coord);

    const Coords &getCoords() const { return coords_; }

    int size() const { return coords_.size(); }

    void solve();

    bool isValid() const;

    void print() const;

    void print(std::ostream &os) const;

   private:
    Grid   *grid_;
    Coords  coords_;
  };

  class Island {
   public:
    Island(Grid *grid);

    void reset();

    bool empty() const { return coords_.empty(); }

    void addCoord(const Coord &coord);

    const Coords &getCoords() const { return coords_; }

    void getCells(Cells &cells) const;

    Region *getRegionConstraint() const;

    void setRegionConstraint(Region *region);

    int size() const { return coords_.size(); }

    void setGaps();

    void solve();

    void checkSolutions(const Solutions &solutions);

    bool isValid() const;

    void print() const;

    void print(std::ostream &os) const;

   private:
    Grid   *grid_;
    Coords  coords_;
    Gaps    gaps_;
  };

  typedef std::set<Island *> Islands;

  class Gap {
   public:
    Gap(Grid *grid);

    void reset();

    bool empty() const { return coords_.empty(); }

    void addCoord(const Coord &coord);

    void addRegion(Region *region);
    void addIsland(Island *island);

    const Coords &getCoords() const { return coords_; }

    const Regions &getRegions() const { return regions_; }

    int size() const { return coords_.size(); }

    bool hasIslands() const { return ! islands_.empty(); }

    void solve();

    void checkSolutions(const Solutions &solutions);

    bool isValid() const;

    void print() const;

    void print(std::ostream &os) const;

   private:
    Grid    *grid_;
    Coords   coords_;
    Regions  regions_;
    Islands  islands_;
  };

  class Grid {
   public:
    Grid(CNurikabe *nurikabe, int num_rows, int num_cols);

    int getNumRows() const { return num_rows_; }
    int getNumCols() const { return num_cols_; }

    int getNumCells() const { return num_rows_*num_cols_; }

    const Cell *getCell(const Coord &coord) const;

    Cell *getCell(const Coord &coord);

    int getMaxValue() const { return max_value_; }

    void setMaxValue(int max_value) { max_value_ = max_value; }

    int getMaxRemaining() const { return maxRemaining_; }

    void updateMaxRemaining(int n) {
      if (nextMaxRemaining_ < 0)
        nextMaxRemaining_ = n;
      else
        nextMaxRemaining_ = std::min(nextMaxRemaining_, n);
    }

    int getMaxSolutions() const { return maxSolutions_; }

    void setMaxSolutions(int num) { maxSolutions_ = num; }

    void updateMaxSolutions() {
      nextMaxSolutions_ = true;
    }

    void setChanged(bool changed=true);
    bool isChanged() const { return changed_; }

    void pushCoords(const Coords &blackCoords, const Coords &whiteCoords);
    void popCoords();

    void resetCoords();

    void commit();

    bool isTop() const { return coordsStack_.empty(); }

    int getCoordDepth() const { return coordsStack_.size(); }

    void addBlackCoord(const Coord &coord);
    void addWhiteCoord(const Coord &coord);

    bool isBlackCoord(const Coord &coord) const {
      return blackCoords_.find(coord) != blackCoords_.end();
    }

    bool isWhiteCoord(const Coord &coord) const {
      return whiteCoords_.find(coord) != whiteCoords_.end();
    }

    const Regions &getRegions() const { return regions_; }

    const Pools &getPools() const { return pools_; }

    int getNumPools() const { return pools_.size(); }

    const Islands &getIslands() const { return islands_; }

    int getNumIslands() const { return islands_.size(); }

    const Gaps &getGaps() const { return gaps_; }

    int getNumGaps() const { return gaps_.size(); }

    void reset();

    void addRegions();

    Pool *createPool();
    void  deletePool(Pool *pool);

    Island *createIsland();
    void    deleteIsland(Island *island);

    Gap  *createGap();
    void  deleteGap(Gap *gap);

    void rebuild(bool force=false);
    void buildRegions();
    void buildPools();
    void buildIslands();
    void buildGaps();

    bool isSolved() const;

    void solveStep();
    void simpleSolveStep();
    void recurseSolveStep();

    bool checkSinglePool();
    bool checkSinglePool(const Coords &coords);

    void solveUnknown(Cell *cell);

    void validate();

    bool checkValid();

    void setConstraints();

    void addOneWhiteConstraint(const Cells &cells);
    void addOneWhiteConstraint(const Coords &coords);

    void addOneBlackConstraint(const Cells &cells);
    void addOneBlackConstraint(const Coords &coords);

    bool isOtherPoolReachable(Cell *cell, const Pool *pool, const Coords &coords);

    bool isBlackReachable(Cell *cell);
    bool isBlackReachable(Cell *cell, const Coords &coords);

    bool canConnectToRegion(Cell *cell, Region *region) const;
    bool canConnectToRegion(Cell *cell, Region *region, Cells &cells) const;

    void addConnectedWhite  (Cell *cell, Cells &cells);
    void addConnectedBlack  (Cell *cell, Cells &cells);
    void addConnectedUnknown(Cell *cell, Cells &cells);

    void addConnectedNumberOrWhite(Cell *cell, Coords &coords);

    void addSolvedNumberOrWhite(Cell *cell, Coords &coords);

    void getOutside(const Coords &icoords, Coords &ocoords);

    void getOutsideUnknown(const Coords &whiteCoords, Coords &unknownCoords);

    void getOutsideUnknownOrWhite(const Coords &coords, Coords &unknownCoords);

    bool checkNonBlack(Cell *cell, Coords &coords, int maxNum);

    Coords getCommonCoords(const CoordsArray &coordsArray);

    void getCommonCoords(const Solutions &solutions, Coords &icoords, Coords &ocoords);

    int getNumIncomplete() const { return numIncomplete_; }

    void startChange();
    void endChange  (const std::string &msg);
    void resetChange();
    bool inChange   () const { return changing_ != 0; }

    void addChange(const Coord &coord);

    void updateBreak() const;

    void generate();

    void getUnknownCells(Cells &cells);

    bool checkBlacksValid();

    void logicError(const std::string &msg);

    void print() const;

    void print(std::ostream &os) const;

    void printMap() const;
    void printMap(std::ostream &os) const;

   private:
    typedef std::vector<CoordsPair> CoordsStack;
    typedef std::vector<Pool *>     PoolArray;
    typedef std::vector<Island *>   IslandArray;
    typedef std::vector<Gap *>      GapArray;

    CNurikabe    *nurikabe_;
    int           num_rows_, num_cols_;
    CellArray     cells_;
    int           max_value_;
    Regions       regions_;
    Pools         pools_;
    PoolArray     poolsArray_;
    Islands       islands_;
    IslandArray   islandsArray_;
    Gaps          gaps_;
    GapArray      gapsArray_;
    bool          changed_;
    int           changing_;
    CoordArray    changes_;
    int           maxRemaining_;
    int           nextMaxRemaining_;
    int           maxSolutions_;
    bool          nextMaxSolutions_;
    int           numIncomplete_;
    Coords        blackCoords_, whiteCoords_;
    CoordsStack   coordsStack_;
  };

 public:
  CNurikabe();

  virtual ~CNurikabe() { }

  int getNumRows() const;
  int getNumCols() const;

  void setPuzzle(int num);

  bool init(const std::string &board_def, const std::string &solution_def);

  void reset();

  Grid *getGrid() const { return grid_; }

  const Cell *getCell(const Coord &coord) const;

  Cell *getCell(const Coord &coord);

  void solve();

  bool solveStep();

  bool isSolved() const;

  CNurikabe::Solutions getRegionSolutions(Region *region, int maxDepth) const;
  CNurikabe::Solutions getRegionSolutions(Region *region) const;

  void setCellBlack(Cell *cell);
  void setCellWhite(Cell *cell);

  void playSolution(const Solution &solution, bool validate=true);

  void unplaySolution();

  void commit();

  void updateBreak();

  const Regions &getRegions() const { return grid_->getRegions(); }

  void generate(int rows, int cols);

  void print() const;

  void print(std::ostream &os) const;

  virtual void setBusy(bool) const { }

  virtual void notifyChanged() { }

  virtual bool checkBreak() { return false; }

 private:
  bool parse(const std::string &board_def, const std::string &solution_def);

 private:
  Grid *grid_;
};

#endif
