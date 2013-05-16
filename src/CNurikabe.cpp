#include <CNurikabe.h>

#include <Puzzles.h>

#include <string>
#include <map>
#include <climits>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

struct changedSignal : std::exception {
  changedSignal(const char *msg1=NULL) :
   msg(msg1) {
  }

  const char *msg;
};

struct breakSignal : std::exception {
  breakSignal(const char *msg1=NULL) :
   msg(msg1) {
  }

  const char *msg;
};

static void break_signal() {
  throw breakSignal();
}

static void changed_signal() {
  throw changedSignal();
}

static void logicAssert(CNurikabe::Grid *g, bool c, const std::string &m) {
  if (! c) g->logicError(m);
}

static bool isLogging() {
  static bool initialized;
  static bool logging;

  if (! initialized) {
    initialized = true;
    logging     = (getenv("CNURIKABE_LOG") != NULL);
  }

  return logging;
}

static void logNoNL(const std::string &str) {
  if (isLogging())
    std::cerr << str;
}

static void log(const std::string &str) {
  if (isLogging())
    std::cerr << str << std::endl;
}

template<typename T>
static std::string toString(const T &t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

std::string intToString(int number) {
  std::stringstream ss;
  ss << number;
  return ss.str();
}

static int randInt(int imax)
{
  return (rand() % imax);
}

template<typename T>
T set_index(const std::set<T> &s, int i) {
  typename std::set<T>::const_iterator p = s.begin();

  for (int i1 = 0; i1 < i; ++i1)
    ++p;

  return *p;
}

//-------------

CNurikabe::
CNurikabe() :
 grid_(NULL)
{
  setPuzzle(1);
}

void
CNurikabe::
setPuzzle(int num)
{
  switch (num) {
    case 1: init(board_def1, solution_def1); break;
    case 2: init(board_def2, solution_def2); break;
    case 3: init(board_def3, solution_def3); break;
    case 4: init(board_def4, solution_def4); break;
    case 5: init(board_def5, solution_def5); break;
    case 6: init(board_def6, solution_def6); break;
    case 7: init(board_def7, solution_def7); break;
    case 8: init(board_def8, solution_def8); break;
    case 9: init(board_def9, solution_def9); break;
    default:                                 break;
  }
}

bool
CNurikabe::
init(const std::string &board_def, const std::string &solution_def)
{
  if (! parse(board_def, solution_def))
    return false;

  grid_->addRegions();

  return true;
}

void
CNurikabe::
reset()
{
  grid_->reset();
}

// _       = empty/unknown
// .       = white
// *       = black
// 1-9,A-Z = numbered

bool
CNurikabe::
parse(const std::string &board_def, const std::string &solution_def)
{
  int num_rows = 0;
  int num_cols = 0;

  int num_cols1 = 0;

  int i            = 0;
  int board_len    = board_def.size();
  int solution_len = solution_def.size();

  // calculate size of grid
  while (i < board_len) {
    if      (board_def[i] == '\n') {
      if (num_cols == 0)
        num_cols = num_cols1;
      else {
        if (num_cols1 != num_cols)
          return false;
      }

      ++num_rows;

      num_cols1 = 0;
    }
    else
      ++num_cols1;

    ++i;
  }

  if (num_rows == 0 || num_cols == 0)
    return false;

  // create grid
  grid_ = new Grid(this, num_rows, num_cols);

  int max_value = 1;

  // populate grid
  i = 0;

  Coord coord(0, 0);

  while (i < board_len) {
    if      (board_def[i] == '\n') {
      ++coord.row;

      coord.col = 0;
    }
    else {
      Cell *cell = grid_->getCell(coord);

      int value = Cell::charToValue(board_def[i]);

      max_value = std::max(max_value, value);

      cell->setValue(value);

      int solution = Cell::UNKNOWN;

      if (i < solution_len)
        solution = Cell::charToValue(solution_def[i]);

      cell->setSolution(solution);

      ++coord.col;
    }

    ++i;
  }

  grid_->setMaxValue(max_value);

  return true;
}

void
CNurikabe::
solve()
{
  setBusy(true);

  while (true) {
    try {
      grid_->solveStep();

      break; // no change
    }
    catch (changedSignal &) {
      grid_->resetChange();
    }
    catch (breakSignal &) {
      grid_->resetCoords();
      break;
    }
    catch (std::exception &e) {
      grid_->resetCoords();
      log(e.what());
      break;
    }
  }

  setBusy(false);

  if (isSolved())
    log("Complete");
  else
    log("No change");
}

bool
CNurikabe::
solveStep()
{
  bool changed = false;

  setBusy(true);

  bool rc = true;

  try {
    grid_->solveStep();
  }
  catch (changedSignal &) {
    changed = true;
    grid_->resetChange();
  }
  catch (breakSignal &) {
    grid_->resetCoords();
    rc = false;
  }
  catch (std::exception &e) {
    grid_->resetCoords();
    log(e.what());
    rc = false;
  }

  setBusy(false);

  if      (isSolved())
    log("Complete");
  else if (rc && ! changed)
    log("No change");

  return rc;
}

bool
CNurikabe::
isSolved() const
{
  return grid_->isSolved();
}

CNurikabe::Solutions
CNurikabe::
getRegionSolutions(Region *region, int maxDepth) const
{
  region->setMaxDepth(maxDepth);

  Solutions solutions = getRegionSolutions(region);

  region->setMaxDepth(INT_MAX);

  return solutions;
}

CNurikabe::Solutions
CNurikabe::
getRegionSolutions(Region *region) const
{
  setBusy(true);

  const CNurikabe::Solutions &solutions = region->getSolutions();

  setBusy(false);

  return solutions;
}

void
CNurikabe::
setCellBlack(Cell *cell)
{
  try {
    getGrid()->startChange();

    cell->setBlack();

    getGrid()->endChange("set black");
  }
  catch (changedSignal &) {
  }
  catch (...) {
    throw;
  }
}

void
CNurikabe::
setCellWhite(Cell *cell)
{
  try {
    getGrid()->startChange();

    cell->setWhite();

    getGrid()->endChange("set white");
  }
  catch (changedSignal &) {
  }
  catch (...) {
    throw;
  }
}

void
CNurikabe::
playSolution(const Solution &solution, bool validate)
{
  grid_->pushCoords(solution.ocoords, solution.icoords);

  try {
    grid_->rebuild(true);

    if (validate)
      solution.checkValid1(grid_);
  }
  catch (...) {
  }
}

void
CNurikabe::
unplaySolution()
{
  grid_->popCoords();

  try {
    grid_->rebuild(true);
  }
  catch (...) {
  }
}

void
CNurikabe::
commit()
{
  grid_->commit();
}

void
CNurikabe::
updateBreak()
{
  if (checkBreak())
    break_signal();
}

int
CNurikabe::
getNumRows() const
{
  return grid_->getNumRows();
}

int
CNurikabe::
getNumCols() const
{
  return grid_->getNumCols();
}

const CNurikabe::Cell *
CNurikabe::
getCell(const Coord &coord) const
{
  return grid_->getCell(coord);
}

CNurikabe::Cell *
CNurikabe::
getCell(const Coord &coord)
{
  return grid_->getCell(coord);
}

void
CNurikabe::
print() const
{
  print(std::cout);
}

void
CNurikabe::
print(std::ostream &os) const
{
  grid_->print(os);
}

void
CNurikabe::
generate(int rows, int cols)
{
  delete grid_;

  grid_ = new Grid(this, rows, cols);

  grid_->generate();
}

//-------------

CNurikabe::Grid::
Grid(CNurikabe *nurikabe, int num_rows, int num_cols) :
 nurikabe_(nurikabe), num_rows_(num_rows), num_cols_(num_cols), max_value_(1),
 changed_(true), changing_(0), maxRemaining_(8), maxSolutions_(4096),
 numIncomplete_(INT_MAX)
{
  cells_.resize(num_rows_*num_cols_);

  for (int i = 0, r = 0; r < num_rows_; ++r)
    for (int c = 0; c < num_cols_; ++c, ++i)
      cells_[i] = new Cell(this, Cell::UNKNOWN, Coord(r, c));
}

void
CNurikabe::Grid::
reset()
{
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    cell->reset();
  }

  //------

  Pools::const_iterator pp1, pp2;

  for (pp1 = pools_.begin(), pp2 = pools_.end(); pp1 != pp2; ++pp1)
    deletePool(*pp1);

  pools_.clear();

  //------

  Islands::const_iterator pi1, pi2;

  for (pi1 = islands_.begin(), pi2 = islands_.end(); pi1 != pi2; ++pi1)
    deleteIsland(*pi1);

  islands_.clear();

  //------

  Gaps::const_iterator pg1, pg2;

  for (pg1 = gaps_.begin(), pg2 = gaps_.end(); pg1 != pg2; ++pg1)
    deleteGap(*pg1);

  gaps_.clear();

  //------

  blackCoords_.clear();
  whiteCoords_.clear();

  coordsStack_.clear();

  maxRemaining_ = 8;
  maxSolutions_ = 16;

  addRegions();

  rebuild();
}

void
CNurikabe::Grid::
addRegions()
{
  Regions::const_iterator pr1, pr2;

  for (pr1 = regions_.begin(), pr2 = regions_.end(); pr1 != pr2; ++pr1)
    delete *pr1;

  regions_.clear();

  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isNumber()) continue;

    Region *region = new Region(this, cell);

    regions_.insert(region);
  }

  setChanged();
}

CNurikabe::Pool *
CNurikabe::Grid::
createPool()
{
  Pool *pool;

  if (poolsArray_.empty())
    pool = new Pool(this);
  else {
    pool = poolsArray_.back();

    poolsArray_.pop_back();

    pool->reset();
  }

  pools_.insert(pool);

  return pool;
}

void
CNurikabe::Grid::
deletePool(Pool *pool)
{
  pool->reset();

  poolsArray_.push_back(pool);

  pools_.erase(pool);
}

CNurikabe::Island *
CNurikabe::Grid::
createIsland()
{
  Island *island;

  if (islandsArray_.empty())
    island = new Island(this);
  else {
    island = islandsArray_.back();

    islandsArray_.pop_back();

    island->reset();
  }

  islands_.insert(island);

  return island;
}

void
CNurikabe::Grid::
deleteIsland(Island *island)
{
  island->reset();

  if (islands_.find(island) != islands_.end()) {
    islandsArray_.push_back(island);

    islands_.erase(island);
  }
}

CNurikabe::Gap *
CNurikabe::Grid::
createGap()
{
  Gap *gap;

  if (gapsArray_.empty())
    gap = new Gap(this);
  else {
    gap = gapsArray_.back();

    gapsArray_.pop_back();

    gap->reset();
  }

  gaps_.insert(gap);

  return gap;
}

void
CNurikabe::Grid::
deleteGap(Gap *gap)
{
  gap->reset();

  if (gaps_.find(gap) != gaps_.end()) {
    gapsArray_.push_back(gap);

    gaps_.erase(gap);
  }
}

const CNurikabe::Cell *
CNurikabe::Grid::
getCell(const Coord &coord) const
{
  return cells_[coord.row*num_cols_ + coord.col];
}

CNurikabe::Cell *
CNurikabe::Grid::
getCell(const Coord &coord)
{
  return cells_[coord.row*num_cols_ + coord.col];
}

void
CNurikabe::Grid::
setChanged(bool changed)
{
  changed_ = changed;

  if (changed && getCoordDepth() == 0) {
    Regions::iterator pr1, pr2;

    for (pr1 = regions_.begin(), pr2 = regions_.end(); pr1 != pr2; ++pr1) {
      Region *region = *pr1;

      region->setChanged();
    }
  }
}

void
CNurikabe::Grid::
pushCoords(const Coords &blackCoords, const Coords &whiteCoords)
{
  coordsStack_.push_back(CoordsPair(blackCoords_, whiteCoords_));

  blackCoords_.insert(blackCoords.begin(), blackCoords.end());
  whiteCoords_.insert(whiteCoords.begin(), whiteCoords.end());
}

void
CNurikabe::Grid::
popCoords()
{
  if (! coordsStack_.empty()) {
    CoordsPair coordsPair = coordsStack_.back();

    coordsStack_.pop_back();

    blackCoords_ = coordsPair.first;
    whiteCoords_ = coordsPair.second;
  }
  else {
    blackCoords_.clear();
    whiteCoords_.clear();
  }
}

void
CNurikabe::Grid::
resetCoords()
{
  resetChange();

  blackCoords_.clear();
  whiteCoords_.clear();

  coordsStack_.clear();

  rebuild(true);
}

void
CNurikabe::Grid::
commit()
{
  try {
    Coords blackCoords = blackCoords_;
    Coords whiteCoords = whiteCoords_;

    resetCoords();

    startChange();

    Coords::const_iterator pc1, pc2;

    for (pc1 = blackCoords.begin(), pc2 = blackCoords.end(); pc1 != pc2; ++pc1) {
      Cell *cell = getCell(*pc1);

      cell->setBlack();
    }

    for (pc1 = whiteCoords.begin(), pc2 = whiteCoords.end(); pc1 != pc2; ++pc1) {
      Cell *cell = getCell(*pc1);

      cell->setWhite();
    }

    endChange("commit");
  }
  catch (...) {
  }
}

void
CNurikabe::Grid::
addBlackCoord(const Coord &coord)
{
  blackCoords_.insert(coord);

  setChanged();
}

void
CNurikabe::Grid::
addWhiteCoord(const Coord &coord)
{
  whiteCoords_.insert(coord);

  setChanged();
}

void
CNurikabe::Grid::
rebuild(bool force)
{
  if (changed_ || force) {
    CellArray::iterator pc1, pc2;

    for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
      Cell *cell = *pc1;

      cell->resetPointers();
    }

    buildRegions();
    buildPools();
    buildIslands();
    buildGaps();

    changed_ = false;
  }
}

void
CNurikabe::Grid::
buildRegions()
{
  numIncomplete_ = 0;

  // build regions for number cells
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isNumber()) continue;

    Region *region = cell->getRegion();

    region->build();

    if (region->isComplete())
      ++numIncomplete_;
  }
}

void
CNurikabe::Grid::
buildIslands()
{
  // delete existing islands
  Islands::iterator pi1, pi2;

  for (pi1 = islands_.begin(), pi2 = islands_.end(); pi1 != pi2; ++pi1) {
    Island *island = *pi1;

    island->reset();

    islandsArray_.push_back(island);
  }

  islands_.clear();

  // build islands for isolated white cells
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isWhite()) continue;

    if (cell->inRegion() || cell->inIsland()) continue;

    cell->buildIsland();
  }
}

void
CNurikabe::Grid::
buildPools()
{
  // delete existing pools
  Pools::iterator pp1, pp2;

  for (pp1 = pools_.begin(), pp2 = pools_.end(); pp1 != pp2; ++pp1) {
    Pool *pool = *pp1;

    pool->reset();

    poolsArray_.push_back(pool);
  }

  pools_.clear();

  // clear pool pointers
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    cell->resetPoolPointer();
  }

  // build pools for black cells
  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isBlack()) continue;

    if (cell->inPool()) continue;

    cell->buildPool();
  }
}

void
CNurikabe::Grid::
buildGaps()
{
  // delete existing gaps
  Gaps::iterator pg1, pg2;

  for (pg1 = gaps_.begin(), pg2 = gaps_.end(); pg1 != pg2; ++pg1) {
    Gap *gap = *pg1;

    gap->reset();

    gapsArray_.push_back(gap);
  }

  gaps_.clear();

  //------

  // build gaps for unknown cells
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isUnknown()) continue;

    if (cell->inGap()) continue;

    cell->buildGap();
  }
}

bool
CNurikabe::Grid::
isSolved() const
{
  // all regions must be complete
  const Regions &regions = getRegions();

  Regions::const_iterator pr1, pr2;

  for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    if (! region->isComplete())
      return false;
  }

  // single black pool
  if (getNumPools() != 1)
    return false;

  // no isolated whites
  if (getNumIslands() != 0)
    return false;

  // no gaps
  if (getNumGaps() != 0)
    return false;

  return true;
}

bool
CNurikabe::Grid::
checkValid()
{
  // do as many simple steps as we can
  for (;;) {
    try {
      simpleSolveStep();
      break; // no change so we are done
    }
    catch (changedSignal &) {
      resetChange();
    }
    catch (breakSignal &) {
      resetCoords();
      break_signal();
    }
    catch (...) {
      resetChange();
      return false;
    }
  }

  updateBreak();

  //------

  // validate new board

  rebuild();

  //------

  const Pools &pools = getPools();

  Pools::const_iterator pp1, pp2;

  for (pp1 = pools.begin(), pp2 = pools.end(); pp1 != pp2; ++pp1) {
    Pool *pool = *pp1;

    if (! pool->isValid())
      return false;
  }

  //------

  const Regions &regions = getRegions();

  Regions::const_iterator pr1, pr2;

  for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    if (! region->isValid())
      return false;
  }

  //------

  const Islands &islands = getIslands();

  Islands::const_iterator pi1, pi2;

  for (pi1 = islands.begin(), pi2 = islands.end(); pi1 != pi2; ++pi1) {
    Island *island = *pi1;

    if (! island->isValid())
      return false;
  }

  //------

  const Gaps &gaps = getGaps();

  Gaps::const_iterator pg1, pg2;

  for (pg1 = gaps.begin(), pg2 = gaps.end(); pg1 != pg2; ++pg1) {
    Gap *gap = *pg1;

    if (! gap->isValid())
      return false;
  }

  //------

  if (isSolved()) {
    //std::cout << "Final Solution" << std::endl;

    // ensure blacks make single pool
    if (getNumPools() > 1)
      return false;
  }

  //------

  // got here so valid

  return true;
}

void
CNurikabe::Grid::
solveStep()
{
  log("solveStep");

  nextMaxRemaining_ = -1;
  nextMaxSolutions_ = false;

  simpleSolveStep();

  recurseSolveStep();

  // if got here then no change

  // try uping max remaining and max solutions
  while (nextMaxRemaining_ > maxRemaining_ || nextMaxSolutions_) {
    if (nextMaxRemaining_ > maxRemaining_) {
      maxRemaining_     = nextMaxRemaining_;
      nextMaxRemaining_ = -1;

      log("Up max remaining limit to " + intToString(maxRemaining_));
    }

    if (nextMaxSolutions_) {
      maxSolutions_     *= 2;
      nextMaxSolutions_  = false;

      log("Up max solutions limit to " + intToString(maxSolutions_));
    }

    //------

    simpleSolveStep();

    recurseSolveStep();
  }
}

void
CNurikabe::Grid::
simpleSolveStep()
{
  log("simpleSolveStep");

  rebuild();

  // Solve Regions
  Regions::iterator pr1, pr2;

  for (pr1 = regions_.begin(), pr2 = regions_.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    region->solve();
  }

  // Solve Pools
  Pools::const_iterator pp1, pp2;

  for (pp1 = pools_.begin(), pp2 = pools_.end(); pp1 != pp2; ++pp1) {
    Pool *pool = *pp1;

    pool->solve();
  }

  // Solve Islands
  Islands::const_iterator pi1, pi2;

  for (pi1 = islands_.begin(), pi2 = islands_.end(); pi1 != pi2; ++pi1) {
    Island *island = *pi1;

    island->solve();
  }

  // Solve Gaps
  Gaps::const_iterator pg1, pg2;

  for (pg1 = gaps_.begin(), pg2 = gaps_.end(); pg1 != pg2; ++pg1) {
    Gap *gap = *pg1;

    gap->solve();
  }

  //----

  if (! checkSinglePool())
    logicAssert(this, false, "multiple pools");
}

bool
CNurikabe::Grid::
checkSinglePool()
{
  // Check if single pool if all unknowns are black
  Coords icoords, ocoords;

  Gaps::const_iterator pg1, pg2;

  for (pg1 = gaps_.begin(), pg2 = gaps_.end(); pg1 != pg2; ++pg1) {
    Gap *gap = *pg1;

    ocoords.insert(gap->getCoords().begin(), gap->getCoords().end());
  }

  pushCoords(ocoords, icoords);

  bool single = false;
  bool first  = true;

  Cells blackCells;

  CellArray::const_iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isBlack()) continue;

    if (first) {
      addConnectedBlack(cell, blackCells);
      first  = false;
      single = true;
    }
    else {
      if (blackCells.find(cell) == blackCells.end()) {
        single = false;
        break;
      }
    }
  }

  popCoords();

  return single;
}

bool
CNurikabe::Grid::
checkSinglePool(const Coords &icoords)
{
  Coords ocoords;

  CellArray::const_iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isUnknown()) continue;

    const Coord &coord = cell->getCoord();

    if (icoords.find(coord) == icoords.end())
      ocoords.insert(coord);
  }

  pushCoords(ocoords, icoords);

  bool single = false;
  bool first  = true;

  Cells blackCells;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isBlack()) continue;

    if (first) {
      addConnectedBlack(cell, blackCells);
      first  = false;
      single = true;
    }
    else {
      if (blackCells.find(cell) == blackCells.end()) {
        single = false;
        break;
      }
    }
  }

  popCoords();

  return single;
}

void
CNurikabe::Grid::
recurseSolveStep()
{
  log("recurseSolveStep");

  rebuild();

  // build region solutions (slow)
  bool allValid = true;

  Coords allCoords;

  Regions::iterator pr1, pr2;

  for (pr1 = regions_.begin(), pr2 = regions_.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    int remaining = region->getValue() - region->size();

    if (remaining > getMaxRemaining()) {
      log("Too many remaining " + intToString(remaining));
      updateMaxRemaining(remaining);
      allValid = false; continue;
    }

    Solutions solutions;

    if (! region->buildSolutionsWithAllCoords(solutions, allCoords)) {
      allValid = false;
      continue;
    }

    if (remaining > 0)
      region->checkSolutions(solutions);
  }

  //----

  // check any unused cells in all valid solutions
  if (allValid && int(allCoords.size()) < getNumCells()) {
    CellArray::iterator pc1, pc2;

    for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
      Cell *cell = *pc1;

      if (allCoords.find(cell->getCoord()) != allCoords.end()) continue;

      if (! cell->isUnknown()) continue;

      //std::cout << "Unused Cell: "; cell->getCoord().print(); std::cout << std::endl;

      startChange();

      cell->setBlack();

      endChange("unused cells");
    }
  }

  validate();
}

void
CNurikabe::Grid::
solveUnknown(Cell *cell)
{
  if (cell->isBlackRegionConstraint()) {
    startChange();

    cell->setBlack();

    endChange("black region constraint");
  }

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // if unknown surrounded by white, must be white
  int numw = 0;

  if (! cellN || cellN->isNumberOrWhite()) ++numw;
  if (! cellS || cellS->isNumberOrWhite()) ++numw;
  if (! cellE || cellE->isNumberOrWhite()) ++numw;
  if (! cellW || cellW->isNumberOrWhite()) ++numw;

  if (numw == 4) {
    startChange();

    cell->setWhite();

    endChange("unknown surrounded by white");
  }

  // if unknown surrounded by black, must be black
  int numb = 0;

  if (! cellN || cellN->isBlack()) ++numb;
  if (! cellS || cellS->isBlack()) ++numb;
  if (! cellE || cellE->isBlack()) ++numb;
  if (! cellW || cellW->isBlack()) ++numb;

  if (numb == 4) {
    startChange();

    cell->setBlack();

    endChange("unknown surrounded by black");
  }

  //------

  // check for unreachables
  if (! isBlackReachable(cell)) {
    startChange();

    cell->setWhite();

    endChange("black unreachable");
  }
}

void
CNurikabe::Grid::
setConstraints()
{
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (cell->isNumberOrWhite()) continue;

    Cell *cellS  = cell->getS ();
    Cell *cellE  = cell->getE ();
    Cell *cellSE = cell->getSE();

    if (cellS  && cellS ->isNumberOrWhite()) continue;
    if (cellE  && cellE ->isNumberOrWhite()) continue;
    if (cellSE && cellSE->isNumberOrWhite()) continue;

    Cells unknownCells;

    if (cell   && cell  ->isUnknown()) unknownCells.insert(cell);
    if (cellS  && cellS ->isUnknown()) unknownCells.insert(cellS);
    if (cellE  && cellE ->isUnknown()) unknownCells.insert(cellE);
    if (cellSE && cellSE->isUnknown()) unknownCells.insert(cellSE);

    addOneWhiteConstraint(unknownCells);
  }

  Pools::const_iterator pp1, pp2;

  for (pp1 = pools_.begin(), pp2 = pools_.end(); pp1 != pp2; ++pp1) {
    Pool *pool = *pp1;

    Coords ocoords;

    getOutsideUnknown(pool->getCoords(), ocoords);

    addOneBlackConstraint(ocoords);
  }

  Islands::const_iterator pi1, pi2;

  for (pi1 = islands_.begin(), pi2 = islands_.end(); pi1 != pi2; ++pi1) {
    Island *island = *pi1;

    Region *region = island->getRegionConstraint();

    if (region != NULL)
      region->addOneWhiteConstraint(island->getCoords());
  }
}

void
CNurikabe::Grid::
addOneWhiteConstraint(const Cells &cells)
{
  Coords coords;

  Cells::const_iterator pc1, pc2;

  for (pc1 = cells.begin(), pc2 = cells.end(); pc1 != pc2; ++pc1)
    coords.insert((*pc1)->getCoord());

  addOneWhiteConstraint(coords);
}

void
CNurikabe::Grid::
addOneWhiteConstraint(const Coords &coords)
{
  Region *region = NULL;
  bool    same   = true;

  Coords::const_iterator pc1, pc2;

  for (pc1 = coords.begin(), pc2 = coords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = getCell(*pc1);

    Region *region1 = cell->getRegionConstraint();

    if      (region == NULL)
      region = region1;
    else if (region != region1) {
      same = false;
      break;
    }
  }

  if (region && same)
    region->addOneWhiteConstraint(coords);
}

void
CNurikabe::Grid::
addOneBlackConstraint(const Cells &cells)
{
  Coords coords;

  Cells::const_iterator pc1, pc2;

  for (pc1 = cells.begin(), pc2 = cells.end(); pc1 != pc2; ++pc1)
    coords.insert((*pc1)->getCoord());

  addOneBlackConstraint(coords);
}

void
CNurikabe::Grid::
addOneBlackConstraint(const Coords &coords)
{
  Region *region = NULL;
  bool    same   = true;

  Coords::const_iterator pc1, pc2;

  for (pc1 = coords.begin(), pc2 = coords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = getCell(*pc1);

    Region *region1 = cell->getRegionConstraint();

    if      (region == NULL)
      region = region1;
    else if (region != region1) {
      same = false;
      break;
    }
  }

  if (region && same)
    region->addOneBlackConstraint(coords);
}

bool
CNurikabe::Grid::
isOtherPoolReachable(Cell *cell, const Pool *pool, const Coords &coords)
{
  if (coords.find(cell->getCoord()) != coords.end())
    return false;

  // too many coords (assume true)
  if (int(coords.size()) > getMaxValue()) return true;

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  if (cellN && cellN->inOtherPool(pool)) return true;
  if (cellS && cellS->inOtherPool(pool)) return true;
  if (cellE && cellE->inOtherPool(pool)) return true;
  if (cellW && cellW->inOtherPool(pool)) return true;

  Coords coords1 = coords;

  coords1.insert(cell->getCoord());

  if (cellN && cellN->isUnknown()) {
    if (isOtherPoolReachable(cellN, pool, coords1))
      return true;
  }
  if (cellS && cellS->isUnknown()) {
    if (isOtherPoolReachable(cellS, pool, coords1))
      return true;
  }
  if (cellE && cellE->isUnknown()) {
    if (isOtherPoolReachable(cellE, pool, coords1))
      return true;
  }
  if (cellW && cellW->isUnknown()) {
    if (isOtherPoolReachable(cellW, pool, coords1))
      return true;
  }

  return false;
}

bool
CNurikabe::Grid::
isBlackReachable(Cell *cell)
{
  Coords coords;

  return isBlackReachable(cell, coords);
}

bool
CNurikabe::Grid::
isBlackReachable(Cell *cell, const Coords &coords)
{
  if (coords.find(cell->getCoord()) != coords.end())
    return false;

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  if (cellN && cellN->isBlack()) return true;
  if (cellS && cellS->isBlack()) return true;
  if (cellE && cellE->isBlack()) return true;
  if (cellW && cellW->isBlack()) return true;

  Coords coords1 = coords;

  coords1.insert(cell->getCoord());

  if (cellN && cellN->isUnknown()) {
    if (isBlackReachable(cellN, coords1))
      return true;
  }
  if (cellS && cellS->isUnknown()) {
    if (isBlackReachable(cellS, coords1))
      return true;
  }
  if (cellE && cellE->isUnknown()) {
    if (isBlackReachable(cellE, coords1))
      return true;
  }
  if (cellW && cellW->isUnknown()) {
    if (isBlackReachable(cellW, coords1))
      return true;
  }

  return false;
}

bool
CNurikabe::Grid::
canConnectToRegion(Cell *cell, Region *region) const
{
  Cells cells;

  if (region->isComplete()) return false;

  return canConnectToRegion(cell, region, cells);
}

bool
CNurikabe::Grid::
canConnectToRegion(Cell *cell, Region *region, Cells &cells) const
{
  // already done
  if (cells.find(cell) != cells.end()) return false;

  // check if too far away
  int minDist = cell->dist(region->getNumberCell()) + cells.size();

  if (minDist > region->getValue())
    return false;

  // add to cells
  cells.insert(cell);

  // check if we have reached region
  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  Regions regions;

  if (cellN && cellN->inRegion()) regions.insert(cellN->getRegion());
  if (cellS && cellS->inRegion()) regions.insert(cellS->getRegion());
  if (cellE && cellE->inRegion()) regions.insert(cellE->getRegion());
  if (cellW && cellW->inRegion()) regions.insert(cellW->getRegion());

  if (regions.size() > 1) return false;

  if (regions.size() == 1) {
    if (*regions.begin() != region)
      return false;

    region->getCells(cells);

    if (cellN && cellN->inIsland()) cellN->getIsland()->getCells(cells);
    if (cellS && cellS->inIsland()) cellS->getIsland()->getCells(cells);
    if (cellE && cellE->inIsland()) cellE->getIsland()->getCells(cells);
    if (cellW && cellW->inIsland()) cellW->getIsland()->getCells(cells);

    if (int(cells.size()) > region->getValue())
      return false;

    return true;
  }

  // get next cell, sorted by distance
  DistCells distCells(region->getNumberCell());

  if (cellN && ! cellN->isBlack() && cellN->canBeInRegion(region)) distCells.insert(cellN);
  if (cellS && ! cellS->isBlack() && cellS->canBeInRegion(region)) distCells.insert(cellS);
  if (cellE && ! cellE->isBlack() && cellE->canBeInRegion(region)) distCells.insert(cellE);
  if (cellW && ! cellW->isBlack() && cellW->canBeInRegion(region)) distCells.insert(cellW);

  DistCells::const_iterator p1, p2;

  for (p1 = distCells.begin(), p2 = distCells.end(); p1 != p2; ++p1) {
    Cell *cell = *p1;

    Cells cells1 = cells;

    if (canConnectToRegion(cell, region, cells1))
      return true;
  }

  return false;
}

void
CNurikabe::Grid::
addConnectedWhite(Cell *cell, Cells &cells)
{
  // if already in cells then we are done
  if (cells.find(cell) != cells.end())
    return;

  // add coord
  cells.insert(cell);

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching white cell (skip number as can only be one and already added)
  if (cellN && cellN->isWhite()) addConnectedWhite(cellN, cells);
  if (cellS && cellS->isWhite()) addConnectedWhite(cellS, cells);
  if (cellE && cellE->isWhite()) addConnectedWhite(cellE, cells);
  if (cellW && cellW->isWhite()) addConnectedWhite(cellW, cells);
}

void
CNurikabe::Grid::
addConnectedNumberOrWhite(Cell *cell, Coords &coords)
{
  // if already in coords then we are done
  if (coords.find(cell->getCoord()) != coords.end())
    return;

  // add coord
  coords.insert(cell->getCoord());

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching white cell (skip number as can only be one and already added)
  if (cellN && cellN->isNumberOrWhite()) addConnectedNumberOrWhite(cellN, coords);
  if (cellS && cellS->isNumberOrWhite()) addConnectedNumberOrWhite(cellS, coords);
  if (cellE && cellE->isNumberOrWhite()) addConnectedNumberOrWhite(cellE, coords);
  if (cellW && cellW->isNumberOrWhite()) addConnectedNumberOrWhite(cellW, coords);
}

void
CNurikabe::Grid::
addConnectedBlack(Cell *cell, Cells &cells)
{
  // if already in coords then we are done
  if (cells.find(cell) != cells.end())
    return;

  // add cell
  cells.insert(cell);

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching black cell
  if (cellN && cellN->isBlack()) addConnectedBlack(cellN, cells);
  if (cellS && cellS->isBlack()) addConnectedBlack(cellS, cells);
  if (cellE && cellE->isBlack()) addConnectedBlack(cellE, cells);
  if (cellW && cellW->isBlack()) addConnectedBlack(cellW, cells);
}

void
CNurikabe::Grid::
addConnectedUnknown(Cell *cell, Cells &cells)
{
  // if already in cells then we are done
  if (cells.find(cell) != cells.end())
    return;

  // add coord
  cells.insert(cell);

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching white cell (skip number as can only be one and already added)
  if (cellN && cellN->isUnknown()) addConnectedUnknown(cellN, cells);
  if (cellS && cellS->isUnknown()) addConnectedUnknown(cellS, cells);
  if (cellE && cellE->isUnknown()) addConnectedUnknown(cellE, cells);
  if (cellW && cellW->isUnknown()) addConnectedUnknown(cellW, cells);
}

void
CNurikabe::Grid::
addSolvedNumberOrWhite(Cell *cell, Coords &coords)
{
  // if already in coords then we are done
  if (coords.find(cell->getCoord()) != coords.end())
    return;

  // add coord
  coords.insert(cell->getCoord());

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching white cell (skip number as can only be one and already added)
  if (cellN && cellN->isSolvedNumberOrWhite()) addSolvedNumberOrWhite(cellN, coords);
  if (cellS && cellS->isSolvedNumberOrWhite()) addSolvedNumberOrWhite(cellS, coords);
  if (cellE && cellE->isSolvedNumberOrWhite()) addSolvedNumberOrWhite(cellE, coords);
  if (cellW && cellW->isSolvedNumberOrWhite()) addSolvedNumberOrWhite(cellW, coords);
}

void
CNurikabe::Grid::
getOutside(const Coords &icoords, Coords &ocoords)
{
  Coords::const_iterator pc1, pc2;

  for (pc1 = icoords.begin(), pc2 = icoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = getCell(*pc1);

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    // add cells not inside set
    if (cellN && icoords.find(cellN->getCoord()) == icoords.end())
      ocoords.insert(cellN->getCoord());
    if (cellS && icoords.find(cellS->getCoord()) == icoords.end())
      ocoords.insert(cellS->getCoord());
    if (cellE && icoords.find(cellE->getCoord()) == icoords.end())
      ocoords.insert(cellE->getCoord());
    if (cellW && icoords.find(cellW->getCoord()) == icoords.end())
      ocoords.insert(cellW->getCoord());
  }
}

void
CNurikabe::Grid::
getOutsideUnknown(const Coords &icoords, Coords &ocoords)
{
  Coords::const_iterator pc1, pc2;

  for (pc1 = icoords.begin(), pc2 = icoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = getCell(*pc1);

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    // add touching unknowns not in inside set
    if (cellN && cellN->isUnknown() && icoords.find(cellN->getCoord()) == icoords.end())
      ocoords.insert(cellN->getCoord());
    if (cellS && cellS->isUnknown() && icoords.find(cellS->getCoord()) == icoords.end())
      ocoords.insert(cellS->getCoord());
    if (cellE && cellE->isUnknown() && icoords.find(cellE->getCoord()) == icoords.end())
      ocoords.insert(cellE->getCoord());
    if (cellW && cellW->isUnknown() && icoords.find(cellW->getCoord()) == icoords.end())
      ocoords.insert(cellW->getCoord());
  }
}

void
CNurikabe::Grid::
getOutsideUnknownOrWhite(const Coords &icoords, Coords &ocoords)
{
  Coords::const_iterator pc1, pc2;

  for (pc1 = icoords.begin(), pc2 = icoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = getCell(*pc1);

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    // add touching unknowns not in inside set
    if (cellN && cellN->isUnknownOrWhite() && icoords.find(cellN->getCoord()) == icoords.end())
      ocoords.insert(cellN->getCoord());
    if (cellS && cellS->isUnknownOrWhite() && icoords.find(cellS->getCoord()) == icoords.end())
      ocoords.insert(cellS->getCoord());
    if (cellE && cellE->isUnknownOrWhite() && icoords.find(cellE->getCoord()) == icoords.end())
      ocoords.insert(cellE->getCoord());
    if (cellW && cellW->isUnknownOrWhite() && icoords.find(cellW->getCoord()) == icoords.end())
      ocoords.insert(cellW->getCoord());
  }
}

void
CNurikabe::Grid::
validate()
{
  int num_unassigned = 0;

  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    if      (cell->isBlack()) {
      // no pools allowed (2x2 all black)
      Cell *cellSE = cell->getSE();

      int n = 1;

      if (cellE  && cellE ->isBlack()) ++n;
      if (cellS  && cellS ->isBlack()) ++n;
      if (cellSE && cellSE->isBlack()) ++n;

      logicAssert(this, n != 4, "black pool");

      // ensure not surrounded by white
      n = 0;

      if (! cellN || cellN->isWhite()) ++n;
      if (! cellS || cellS->isWhite()) ++n;
      if (! cellE || cellE->isWhite()) ++n;
      if (! cellW || cellW->isWhite()) ++n;

      logicAssert(this, n != 4, "black surrounded");
    }
    else if (cell->isWhite()) {
      // ensure not surrounded by black
      if (! cell->isNumber(1)) {
        int n = 0;

        if (! cellN || cellN->isBlack()) ++n;
        if (! cellS || cellS->isBlack()) ++n;
        if (! cellE || cellE->isBlack()) ++n;
        if (! cellW || cellW->isBlack()) ++n;

        logicAssert(this, n != 4, "white surrounded");
      }
    }
    else if (cell->isUnknown()) {
      ++num_unassigned;
    }
  }

  // if no unknowns then single pool

  // all regions are less than or equal to value and have single number square
}

bool
CNurikabe::Grid::
checkNonBlack(Cell *cell, Coords &coords, int maxNum)
{
  // if already in coords then we are done
  if (coords.find(cell->getCoord()) != coords.end())
    return false;

  // add coord
  coords.insert(cell->getCoord());

  if (int(coords.size()) >= maxNum) return true;

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  // expand if touching white cell (skip number as can only be one and already added)
  if (cellN && ! cellN->isBlack()) if (checkNonBlack(cellN, coords, maxNum)) return true;
  if (cellS && ! cellS->isBlack()) if (checkNonBlack(cellS, coords, maxNum)) return true;
  if (cellE && ! cellE->isBlack()) if (checkNonBlack(cellE, coords, maxNum)) return true;
  if (cellW && ! cellW->isBlack()) if (checkNonBlack(cellW, coords, maxNum)) return true;

  return false;
}

CNurikabe::Coords
CNurikabe::Grid::
getCommonCoords(const CoordsArray &coordsArray)
{
  int n = coordsArray.size();

  if (n == 0) return Coords();

  const Coords &coords1 = coordsArray[0];

  if (n == 1)
    return coords1;

  // check for common coords in array of coords
  Coords rcoords;

  Coords::const_iterator pc1, pc2;

  for (pc1 = coords1.begin(), pc2 = coords1.end(); pc1 != pc2; ++pc1) {
    const Coord &coord1 = *pc1;

    // check if this coords exists in all other coords
    bool found = true;

    for (int i = 1; i < n; ++i) {
      const Coords &coords2 = coordsArray[i];

      if (coords2.find(coord1) == coords2.end()) {
        found = false; // not in these coords so we are done
        break;
      }
    }

    // if coord in all coords add to return coords
    if (found)
      rcoords.insert(coord1);
  }

  return rcoords;
}

void
CNurikabe::Grid::
getCommonCoords(const Solutions &solutions, Coords &icoords, Coords &ocoords)
{
  int n = solutions.size();

  if (n == 0) return;

  if (n == 1) {
    const Solution &solution1 = *solutions.begin();

    icoords = solution1.icoords;
    ocoords = solution1.ocoords;

    return;
  }

  //-----

  Solutions::iterator ps1 = solutions.begin();
  Solutions::iterator ps2 = solutions.end  ();

  const Solution &solution1 = *ps1++;

  // check for common inside coord in all solutions
  Coords::const_iterator pc1, pc2;

  for (pc1 = solution1.icoords.begin(), pc2 = solution1.icoords.end(); pc1 != pc2; ++pc1) {
    const Coord &coord1 = *pc1;

    // check if this inside cell exists in all other solutions
    bool found = true;

    for (Solutions::const_iterator ps3 = ps1; ps3 != ps2; ++ps3) {
      const Solution &solution2 = *ps3;

      if (solution2.icoords.find(coord1) == solution2.icoords.end()) {
        found = false; // not in this solution so we are done
        break;
      }
    }

    // if coord in all solutions must be inside
    if (found)
      icoords.insert(coord1);
  }

  // check for common outside coord in all solutions
  for (pc1 = solution1.ocoords.begin(), pc2 = solution1.ocoords.end(); pc1 != pc2; ++pc1) {
    const Coord &coord1 = *pc1;

    // check if this outside cell exists in all other solutions
    bool found = true;

    for (Solutions::const_iterator ps3 = ps1; ps3 != ps2; ++ps3) {
      const Solution &solution2 = *ps3;

      if (solution2.ocoords.find(coord1) == solution2.ocoords.end()) {
        found = false; // not in this solution so we are done
        break;
      }
    }

    // if coord in all solutions must be outside
    if (found)
      ocoords.insert(coord1);
  }
}

void
CNurikabe::Grid::
startChange()
{
  assert(! changing_);

  setChanged(false);

  ++changing_;

  updateBreak();

  if (changing_ == 1)
    changes_.clear();
}

void
CNurikabe::Grid::
endChange(const std::string &msg)
{
  assert(changing_ > 0);

  --changing_;

  updateBreak();

  if (changing_ == 0 && changed_) {
    logNoNL(msg + " : ");

    int n = changes_.size();

    for (int i = 0; i < n; ++i) {
      if (i > 0) logNoNL(", ");

      logNoNL(toString(changes_[i]));
    }

    log("");

    changes_.clear();

    if (n > 0)
      nurikabe_->notifyChanged();

    changed_signal();
  }
}

void
CNurikabe::Grid::
resetChange()
{
  setChanged();

  changing_ = 0;
}

void
CNurikabe::Grid::
addChange(const Coord &coord)
{
  changes_.push_back(coord);
}

void
CNurikabe::Grid::
updateBreak() const
{
  nurikabe_->updateBreak();
}

void
CNurikabe::Grid::
generate()
{
  reset();

  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1)
    (*pc1)->init();

  int num_rows = getNumRows();
  int num_cols = getNumCols();

  int row = randInt(num_rows);
  int col = randInt(num_cols);

  Cell *startCell = getCell(Coord(row, col));

  startCell->setValue(Cell::BLACK);

  CellArray blackCells;

  blackCells.push_back(startCell);

  int num_fails = 0;

  while (true) {
    int n = blackCells.size();

    Cells openCells;

    // find next black expand cells
    while (true) {
      Cell *cell = blackCells[randInt(n)];

      Cell *cellN = cell->getN();
      Cell *cellS = cell->getS();
      Cell *cellE = cell->getE();
      Cell *cellW = cell->getW();

      openCells.clear();

      if (cellN && ! cellN->isBlack()) openCells.insert(cellN);
      if (cellS && ! cellS->isBlack()) openCells.insert(cellS);
      if (cellE && ! cellE->isBlack()) openCells.insert(cellE);
      if (cellW && ! cellW->isBlack()) openCells.insert(cellW);

      if (! openCells.empty())
        break;
    }

    // chose valid expand cell
    Cell *cell  = NULL;
    bool  found = false;

    while (true) {
      int n = openCells.size();

      if (n == 0) break;

      cell = set_index<Cell *>(openCells, randInt(n));

      openCells.erase(cell);

      cell->setValue(Cell::BLACK);

      bool rc = checkBlacksValid();

      if (rc) {
        found = true;
        break;
      }

      cell->setValue(Cell::UNKNOWN);

      break;
    }

    // no expand cell so done
    if (! found) {
      ++num_fails;

      if (num_fails > 100)
        break;

      continue;
    }

    blackCells.push_back(cell);

    num_fails = 0;
  }

  // fill in numbers
  Cells unknownCells;

  getUnknownCells(unknownCells);

  int n = unknownCells.size();

  while (n > 0) {
    Cell *cell = set_index<Cell *>(unknownCells, randInt(n));

    Cells cells;

    addConnectedUnknown(cell, cells);

    Cells::iterator pc1, pc2;

    for (pc1 = cells.begin(), pc2 = cells.end(); pc1 != pc2; ++pc1)
      (*pc1)->setValue(Cell::WHITE);

    cell->setValue(cells.size());

    unknownCells.clear();

    getUnknownCells(unknownCells);

    n = unknownCells.size();
  }

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isNumber())
      cell->setValue(Cell::UNKNOWN);
  }

  addRegions();

  rebuild(true);
}

void
CNurikabe::Grid::
getUnknownCells(Cells &cells)
{
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isUnknown())
      continue;

    cells.insert(cell);
  }
}

bool
CNurikabe::Grid::
checkBlacksValid()
{
  CellArray::iterator pc1, pc2;

  for (pc1 = cells_.begin(), pc2 = cells_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    if (! cell->isBlack()) continue;

    Cell *cellS  = cell->getS ();
    Cell *cellE  = cell->getE ();
    Cell *cellSE = cell->getSE();

    int n = 1;

    if (cellE  && cellE ->isBlack()) ++n;
    if (cellS  && cellS ->isBlack()) ++n;
    if (cellSE && cellSE->isBlack()) ++n;

    if (n == 4)
      return false;
  }

  return true;
}

void
CNurikabe::Grid::
print() const
{
  print(std::cout);
}

void
CNurikabe::Grid::
print(std::ostream &os) const
{
  Grid *th = const_cast<Grid *>(this);

  th->rebuild(true);

  printMap(os);

  //------

  {
  bool first = true;

  os << "Pools:" << std::endl;

  Pools::const_iterator p1, p2;

  for (p1 = pools_.begin(), p2 = pools_.end(); p1 != p2; ++p1) {
    if (! first) os << std::endl;

    os << " "; (*p1)->print(os);

    first = false;
  }

  os << std::endl;
  }

  //------

  {
  bool first = true;

  os << "Regions:" << std::endl;

  Regions::const_iterator p1, p2;

  for (p1 = regions_.begin(), p2 = regions_.end(); p1 != p2; ++p1) {
    if (! first) os << std::endl;

    os << " "; (*p1)->print(os);

    first = false;
  }

  os << std::endl;
  }

  //------

  {
  bool first = true;

  os << "Islands:" << std::endl;

  Islands::const_iterator p1, p2;

  for (p1 = islands_.begin(), p2 = islands_.end(); p1 != p2; ++p1) {
    if (! first) os << std::endl;

    os << " "; (*p1)->print(os);

    first = false;
  }

  os << std::endl;
  }

  //------

  {
  bool first = true;

  os << "Gaps:" << std::endl;

  Gaps::const_iterator p1, p2;

  for (p1 = gaps_.begin(), p2 = gaps_.end(); p1 != p2; ++p1) {
    if (! first) os << std::endl;

    os << " "; (*p1)->print(os);

    first = false;
  }

  os << std::endl;
  }
}

void
CNurikabe::Grid::
logicError(const std::string &msg)
{
  if (! isTop())
    throw std::logic_error(msg.c_str());
  else {
    log("Logic Error: " + msg);
    break_signal();
  }
}

void
CNurikabe::Grid::
printMap() const
{
  printMap(std::cout);
}

void
CNurikabe::Grid::
printMap(std::ostream &os) const
{
  int num_rows = getNumRows();
  int num_cols = getNumCols();

  for (int r = 0; r < num_rows; ++r) {
    for (int c = 0; c < num_cols; ++c) {
      const Cell *cell = getCell(Coord(r, c));

      if      (cell->isNumber()) {
        int num = cell->getNumber();

        if      (num < 10) os << num;
        else if (num < 36) os << char('A' + (num - 10));
        else               os << '?';
      }
      else if (cell->isWhite ()) os << ".";
      else if (cell->isBlack ()) os << "*";
      else                       os << "_";
    }

    os << std::endl;
  }
}

//-------------

CNurikabe::Region::
Region(Grid *grid, Cell *cell) :
 grid_(grid), cell_(cell), partial_(true), solutions_(),
 solutionsValid_(false), maxDepth_(INT_MAX)
{
  addCell(cell);

  Coords coords;

  grid->addSolvedNumberOrWhite(cell, coords);

  solution_ = Solution(coords);
}

void
CNurikabe::Region::
addCell(Cell *cell)
{
  coords_.insert(cell->getCoord());

  cell->setRegion(this);

  partial_ = (size() < getValue());

  logicAssert(grid_, size() <= getValue(), "region overflow");

  //-----

  Cell *cellN = cell->getN();
  Cell *cellS = cell->getS();
  Cell *cellE = cell->getE();
  Cell *cellW = cell->getW();

  if (cellN && cellN->isWhite() && ! cellN->inRegion()) addCell(cellN);
  if (cellS && cellS->isWhite() && ! cellS->inRegion()) addCell(cellS);
  if (cellE && cellE->isWhite() && ! cellE->inRegion()) addCell(cellE);
  if (cellW && cellW->isWhite() && ! cellW->inRegion()) addCell(cellW);
}

void
CNurikabe::Region::
getCells(Cells &cells) const
{
  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1)
    cells.insert(grid_->getCell(*p1));
}

void
CNurikabe::Region::
build()
{
  coords_.clear();

  coords_.insert(getCoord());

  Cells cells;

  grid_->addConnectedWhite(cell_, cells);

  Cells::const_iterator p1, p2;

  for (p1 = cells.begin(), p2 = cells.end(); p1 != p2; ++p1) {
    Cell *cell = *p1;

    coords_.insert(cell->getCoord());

    cell->setRegion(this);
  }

  partial_ = (size() < getValue());
}

void
CNurikabe::Region::
solve()
{
  log("region " + intToString(getValue()) + " " + cell_->getCoord().str() + " solve");

  if (! isComplete()) {
    grid_->startChange();

    Coords::const_iterator p1, p2;

    for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
      Cell *cell = grid_->getCell(*p1);

      Cell *cellN = cell->getN();
      Cell *cellS = cell->getS();
      Cell *cellE = cell->getE();
      Cell *cellW = cell->getW();

      // cells adjacent to two partial regions must be black
      Cell *cellN2 = cell->getN(2);
      Cell *cellS2 = cell->getS(2);
      Cell *cellE2 = cell->getE(2);
      Cell *cellW2 = cell->getW(2);

      if (cellN2 && cellN2->inOtherRegion(this)) cellN->setBlack();
      if (cellS2 && cellS2->inOtherRegion(this)) cellS->setBlack();
      if (cellE2 && cellE2->inOtherRegion(this)) cellE->setBlack();
      if (cellW2 && cellW2->inOtherRegion(this)) cellW->setBlack();

      // regions diagonally must have black at opposite corners
      Cell *cellNW = cell->getNW();
      Cell *cellNE = cell->getNE();
      Cell *cellSW = cell->getSW();
      Cell *cellSE = cell->getSE();

      if (cellNW && cellNW->inOtherRegion(this)) { cellN->setBlack(); cellW->setBlack(); }
      if (cellNE && cellNE->inOtherRegion(this)) { cellN->setBlack(); cellE->setBlack(); }
      if (cellSW && cellSW->inOtherRegion(this)) { cellS->setBlack(); cellW->setBlack(); }
      if (cellSE && cellSE->inOtherRegion(this)) { cellS->setBlack(); cellE->setBlack(); }

      // outside region must be black or belong to this region
      if (grid_->isTop()) {
        if (cellN && cellN->isUnknown()) cellN->setRegionConstraint(this);
        if (cellS && cellS->isUnknown()) cellS->setRegionConstraint(this);
        if (cellE && cellE->isUnknown()) cellE->setRegionConstraint(this);
        if (cellW && cellW->isUnknown()) cellW->setRegionConstraint(this);
      }
    }

    grid_->endChange("adjacent or diagonal");

    //----

    // get outside coords
    Coords ocoords;

    grid_->getOutsideUnknown(coords_, ocoords);

    logicAssert(grid_, ! partial_ || ! ocoords.empty(), "no expansion for partial region");

    // if partial and one exit, exit must be white
    grid_->startChange();

    while (! isComplete() && ocoords.size() == 1) {
      Cell *openCell = grid_->getCell(*ocoords.begin());

      openCell->setWhite();

      build();

      partial_ = (size() < getValue());

      ocoords.clear();

      grid_->getOutsideUnknown(coords_, ocoords);
    }

    grid_->endChange("partial and one exit");

    //----

    // if only two unknowns to finish region, and cells touch at corners then far side
    // cell must be black
    if (size() == getValue() - 1 && ocoords.size() == 2) {
      Coords::const_iterator p1 = ocoords.begin();

      Cell *openCell1 = grid_->getCell(*p1++);
      Cell *openCell2 = grid_->getCell(*p1);

      if (openCell1->cornerTouches(openCell2)) {
        if (openCell1->getCoord().col > openCell2->getCoord().col)
          std::swap(openCell1, openCell2);

        Cell *cell1, *cell2;

        if (openCell1->getCoord().row < openCell2->getCoord().row) {
          cell1 = openCell1->getE();
          cell2 = openCell1->getS();
        }
        else {
          cell1 = openCell1->getE();
          cell2 = openCell1->getN();
        }

        grid_->startChange();

        if      (cell1->isUnknown() && ! cell2->isUnknown())
          cell1->setBlack();
        else if (cell2->isUnknown() && ! cell1->isUnknown())
          cell2->setBlack();

        grid_->endChange("two unknowns and touch at corners");
      }
    }

    // add region locked (TODO cleanup)
    addRegionLocked();

    // get all possible connect cells
    Coords coords = coords_;

    checkConnectCoords(coords);
  }
  else {
    grid_->startChange();

    // cells surrounding a complete region are black
    Coords ocoords;

    grid_->getOutsideUnknown(coords_, ocoords);

    Coords::const_iterator p1, p2;

    for (p1 = ocoords.begin(), p2 = ocoords.end(); p1 != p2; ++p1) {
      Cell *cell = grid_->getCell(*p1);

      cell->setBlack();
    }

    grid_->endChange("region surrounding cells");
  }
}

void
CNurikabe::Region::
addRegionLocked()
{
  if (isComplete()) return;

  Coords ocoords;

  grid_->getOutsideUnknown(getCoords(), ocoords);

  // if only two unknowns to finish region, and cells touch at corners then far side
  // cell can't belong to another region
  if (ocoords.size() == 2) {
    Coords::const_iterator p2 = ocoords.begin();
    Coords::const_iterator p1 = p2++;

    Cell *openCell1 = grid_->getCell(*p1);
    Cell *openCell2 = grid_->getCell(*p2);

    if (openCell1->cornerTouches(openCell2)) {
      if (openCell1->getCoord().col > openCell2->getCoord().col)
        std::swap(openCell1, openCell2);

      Cell *cell1, *cell2;

      if (openCell1->getCoord().row < openCell2->getCoord().row) {
        cell1 = openCell1->getE();
        cell2 = openCell1->getS();
      }
      else {
        cell1 = openCell1->getE();
        cell2 = openCell1->getN();
      }

      if      (cell1->isUnknown() && ! cell2->isUnknown()) {
        //std::cerr << cell1->getCoord() << " is Black or Region " << getValue() << std::endl;
        if (grid_->isTop())
          cell1->setRegionConstraint(this);
      }
      else if (cell2->isUnknown() && ! cell1->isUnknown()) {
        //std::cerr << cell2->getCoord() << " is Black or Region " << getValue() << std::endl;
        if (grid_->isTop())
          cell2->setRegionConstraint(this);
      }
    }
  }
}

void
CNurikabe::Region::
checkConnectCoords(Coords &coords)
{
  bool found = true;

  while (found) {
    Coords ocoords;

    grid_->getOutsideUnknownOrWhite(coords, ocoords);

    found = false;

    Coords::const_iterator pc1, pc2;

    for (pc1 = ocoords.begin(), pc2 = ocoords.end(); pc1 != pc2; ++pc1) {
      const Coord &coord = *pc1;

      Cell *cell = grid_->getCell(coord);

      if (! cell->canBeInRegion(this)) continue;

      if (coords.find(coord) == coords.end()) {
        coords.insert(*pc1);

        found = true;
      }
    }
  }

  // ensure enough resources
  logicAssert(grid_, int(coords.size()) >= getValue(),
               "no room for region " + intToString(getValue()));

  // if only just enough then all must be white
  if (int(coords.size()) == getValue()) {
    grid_->startChange();

    Coords::const_iterator pc1, pc2;

    for (pc1 = coords.begin(), pc2 = coords.end(); pc1 != pc2; ++pc1) {
      Cell *cell = grid_->getCell(*pc1);

      cell->setWhite();
    }

    grid_->endChange("only just enough unknown");
  }
}

CNurikabe::Solutions &
CNurikabe::Region::
getSolutions()
{
  if (! solutionsValid_) {
    try {
      Region *region = const_cast<Region *>(this);

      grid_->rebuild(true);

      Coords allCoords;

      region->buildSolutionsWithAllCoords(solutions_, allCoords);

      solutionsValid_ = true;
    }
    catch (...) {
      std::cerr << "CNurikabe::Region::getSolutions" << std::endl;
    }
  }

  return solutions_;
}

bool
CNurikabe::Region::
buildSolutionsWithAllCoords(Solutions &solutions, Coords &allCoords)
{
  solutions.clear();

  solutionsMap_.clear();

  // build all possible solutions for this region
  if (! isComplete()) {
    log("buildSolutions");

    if (! buildSolutions(solutions))
      return false;

    if (hasValidSolution() && solutions.find(solution_) == solutions.end()) {
      logicAssert(grid_, false, "Build Solutions has no Valid Solution Match");
      //solutionsValid_ = false;
      return false;
    }

    logicAssert(grid_, ! solutions.empty(), "no solutions for " + intToString(getValue()));

    //------

    log(intToString(solutions.size()) + " solutions");

    // set outside coords
    Solutions::iterator ps1, ps2;

    for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
      Solution &solution = const_cast<Solution &>(*ps1);

      logicAssert(grid_, solution.valid, "Solution not valid");

      allCoords.insert(solution.icoords.begin(), solution.icoords.end());

      solution.checkValid(this);
    }

    //------

    // trim to valid solutions
    Solutions solutions1;

    for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
      const Solution &solution = *ps1;

      if (solution.valid)
        solutions1.insert(solution);
    }

    solutions = solutions1;

    log(intToString(solutions.size()) + " valid solutions");

    if (hasValidSolution() && solutions.find(solution_) == solutions.end()) {
      logicAssert(grid_, false, "Build Solutions has no Valid Solution Match");
      //solutionsValid_ = false;
      return false;
    }

    //------

    CoordsArray ioCoordsArray;

    for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
      const Solution &solution = *ps1;

      Coords ioCoords = solution.icoords;

      ioCoords.insert(solution.ocoords.begin(), solution.ocoords.end());

      ioCoordsArray.push_back(ioCoords);
    }

    Coords commonIOCoords = grid_->getCommonCoords(ioCoordsArray);

    log(intToString(commonIOCoords.size()) + " common coords");

    Coords::const_iterator pc1, pc2;

    for (pc1 = commonIOCoords.begin(), pc2 = commonIOCoords.end(); pc1 != pc2; ++pc1) {
      Cell *cell = grid_->getCell(*pc1);

      if (cell->isUnknown()) {
        //std::cerr << *pc1 << " is Black or Region " << getValue() << std::endl;
        if (grid_->isTop())
          cell->setRegionConstraint(this);
      }
    }
  }
  else
    solutions.insert(Solution(coords_));

  int id = 1;

  Solutions::iterator ps1, ps2;

  for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1)
    const_cast<Solution &>(*ps1).id = id++;

  return true;
}

bool
CNurikabe::Region::
hasValidSolution() const
{
  return int(solution_.icoords.size()) == getValue();
}

void
CNurikabe::Region::
checkSolutions(const Solutions &solutions)
{
  if (isComplete()) return;

  logicAssert(grid_, ! solutions.empty(), "no valid solutions for " + intToString(getValue()));

  log(intToString(solutions.size()) + " solutions");

  CoordsArray whiteCoordsArray;
  CoordsArray blackCoordsArray;

  Solutions::iterator ps1, ps2;

  for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
    const Solution &solution = *ps1;

    whiteCoordsArray.push_back(solution.whiteCoords);
    blackCoordsArray.push_back(solution.blackCoords);
  }

  grid_->startChange();

  Coords commonICoords = grid_->getCommonCoords(whiteCoordsArray);
  Coords commonOCoords = grid_->getCommonCoords(blackCoordsArray);

  log(intToString(commonICoords.size()) + " common white, " +
      intToString(commonOCoords.size()) + " common black");

  Coords::const_iterator pc1, pc2;

  for (pc1 = commonICoords.begin(), pc2 = commonICoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    cell->setWhite();
  }

  for (pc1 = commonOCoords.begin(), pc2 = commonOCoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    cell->setBlack();
  }

  grid_->endChange("region common coords");

  //------

  build();
}

bool
CNurikabe::Region::
buildSolutions(Solutions &solutions)
{
  solutions.clear();

  solutionsMap_.clear();

  Coords coords = coords_;

  if (getValue() > 4) {
    oneWhiteConstraints_.clear();
    oneBlackConstraints_.clear();

    grid_->setConstraints();
  }

  return buildSolutions(coords, solutions);
}

bool
CNurikabe::Region::
buildSolutions(Coords &coords, Solutions &solutions)
{
  grid_->updateBreak();

  int nc = coords.size();

  // too many coords so can't be a solution
  if (nc > getValue()) return true;

  //----

  if (! checkConstrainedBlacks(coords))
    return true;

  //----

  // check if grid, with these whites, will form a single pool
  bool check = (grid_->getNumIncomplete() < 2);

  if (check && ! grid_->checkSinglePool(coords)) return true;

  //----

  Coords unknownCoords;

  // get constrained unknowns first
  if (! getConstrainedWhites(coords, unknownCoords)) {
    // otherwise use bordering unknowns
    grid_->getOutsideUnknown(coords, unknownCoords);
  }

  //grid_->filterWhiteCoords(unknownCoords);

  //----

  Solution solution(coords);

  // ensure we don't have this solution
  Solutions &nSolutions = solutionsMap_[nc];

  if (nSolutions.find(solution) != nSolutions.end()) {
    log("Skip already used solution for " + intToString(nc));
    return true;
  }

  nSolutions.insert(solution);

  //----

  // reached required size so we have a possible solution
  if (nc == getValue() || nc >= maxDepth_) {
    Solution solution1(coords);

    assert(solutions.find(solution1) == solutions.end());

    solutions.insert(solution1);

    if (int(solutions.size()) > grid_->getMaxSolutions()) {
      log("Too many solutions for " + intToString(getValue()));
      grid_->updateMaxSolutions();
      return false;
    }

    return true;
  }

  // for each cell in current region or unknown expand solution using the coord
  Coords::const_iterator pc1, pc2;

  for (pc1 = unknownCoords.begin(), pc2 = unknownCoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    if (! cell->canBeInRegion(this))
      continue;

    Coords coords1 = coords;

    // add coord and any whites or numbers touching this
    grid_->addConnectedNumberOrWhite(cell, coords1);

    // recurse to next expand coord
    if (! buildSolutions(coords1, solutions))
      return false;
  }

  return true;
}

bool
CNurikabe::Region::
getConstrainedWhites(const Coords &coords, Coords &unknownCoords)
{
  if (oneWhiteConstraints_.empty()) return false;

  OneWhiteConstraints::const_iterator p1, p2;

  for (p1 = oneWhiteConstraints_.begin(), p2 = oneWhiteConstraints_.end(); p1 != p2; ++p1) {
    const OneWhiteConstraint &constraint = *p1;

    bool found = false;

    // check if constraint satisfied by current solution
    Coords::const_iterator pc1, pc2;

    for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1) {
      const Coord &coord = *pc1;

      if (coords.find(coord) != coords.end()) {
        found = true;
        break;
      }
    }

    // if constraint not satisfied check if all constraint coords touch
    if (! found) {
      bool touch = true;

      for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1) {
        const Coord &coord = *pc1;

        // check if constraint coord touches any solution coord
        bool touch1 = true;

        Coords::const_iterator pc1, pc2;

        for (pc1 = coords.begin(), pc2 = coords.end(); pc1 != pc2; ++pc1) {
          const Coord &coord1 = *pc1;

          if (! coord.touches(coord1)) {
            touch1 = false;
            break;
          }
        }

        // no touch so not usable
        if (! touch1) {
          touch = false;
          break;
        }
      }

      if (touch) {
#if 0
        std::cerr << "force next coords:";
        for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1)
          std::cerr << " " << *pc1;
        std::cerr << std::endl;
#endif
        unknownCoords = constraint.coords;
        return true;
      }
    }
  }

  return false;
}

bool
CNurikabe::Region::
checkConstrainedBlacks(const Coords &coords)
{
  if (oneBlackConstraints_.empty()) return true;

  OneBlackConstraints::const_iterator p1, p2;

  for (p1 = oneBlackConstraints_.begin(), p2 = oneBlackConstraints_.end(); p1 != p2; ++p1) {
    const OneBlackConstraint &constraint = *p1;

    bool found = true;

    Coords::const_iterator pc1, pc2;

    for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1) {
      const Coord &coord = *pc1;

      if (coords.find(coord) == coords.end()) {
        found = false;
        break;
      }
    }

    if (found)
      return false;
  }

  return true;
}

bool
CNurikabe::Region::
isValid() const
{
  if (! isComplete()) {
    if (int(coords_.size()) > getValue())
      return false;

    Coords ocoords;

    grid_->getOutsideUnknown(coords_, ocoords);

    if (ocoords.empty())
      return false;

    Coords coords;

    if (! grid_->checkNonBlack(cell_, coords, getValue())) {
      log("Not enough resources for region " + intToString(getValue()));
      return false;
    }
  }
  else {
    if (int(coords_.size()) != getValue())
      return false;
  }

  return true;
}

void
CNurikabe::Region::
addOneWhiteConstraint(const Coords &coords)
{
  oneWhiteConstraints_.push_back(OneWhiteConstraint(coords));
}

void
CNurikabe::Region::
addOneBlackConstraint(const Coords &coords)
{
  oneBlackConstraints_.push_back(OneBlackConstraint(coords));
}

void
CNurikabe::Region::
filterWhiteCoords(Coords &unknownCoords)
{
  if (oneWhiteConstraints_.empty()) return;

  OneWhiteConstraints::const_iterator p1, p2;

  for (p1 = oneWhiteConstraints_.begin(), p2 = oneWhiteConstraints_.end(); p1 != p2; ++p1) {
    const OneWhiteConstraint &constraint = *p1;

    bool found = true;

    Coords::const_iterator pc1, pc2;

    for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1) {
      if (unknownCoords.find(*pc1) == unknownCoords.end()) {
        found = false;
        break;
      }
    }

    if (found) {
      logNoNL("force next coords:");

      for (pc1 = constraint.coords.begin(), pc2 = constraint.coords.end(); pc1 != pc2; ++pc1)
        logNoNL(" " + toString(*pc1));

      log("");

      unknownCoords = constraint.coords;

      return;
    }
  }
}

void
CNurikabe::Region::
printConstraints()
{
  grid_->rebuild(true);

  oneWhiteConstraints_.clear();
  oneBlackConstraints_.clear();

  grid_->setConstraints();

  OneWhiteConstraints::const_iterator pw1, pw2;

  for (pw1 = oneWhiteConstraints_.begin(), pw2 = oneWhiteConstraints_.end(); pw1 != pw2; ++pw1) {
    const OneWhiteConstraint &constraint = *pw1;

    std::cout << "One White: "; constraint.print(std::cout); std::cout << std::endl;
  }

  OneBlackConstraints::const_iterator pb1, pb2;

  for (pb1 = oneBlackConstraints_.begin(), pb2 = oneBlackConstraints_.end(); pb1 != pb2; ++pb1) {
    const OneBlackConstraint &constraint = *pb1;

    std::cout << "One Black: "; constraint.print(std::cout); std::cout << std::endl;
  }
}

void
CNurikabe::Region::
setChanged()
{
  solutionsValid_ = false;
}

void
CNurikabe::Region::
print() const
{
  print(std::cout);
}

void
CNurikabe::Region::
print(std::ostream &os) const
{
  {
  bool first = true;

  os << getValue() << ") ";

  Coords::const_iterator pc1, pc2;

  for (pc1 = coords_.begin(), pc2 = coords_.end(); pc1 != pc2; ++pc1) {
    if (! first) os << " ";

    (*pc1).print(os);

    first = false;
  }

  if (isComplete()) os << " (Complete)";
  }

  //----

  {
  os << " [";

  bool first = true;

  Solutions::const_iterator ps1, ps2;

  for (ps1 = solutions_.begin(), ps2 = solutions_.end(); ps1 != ps2; ++ps1) {
    const Solution &solution = *ps1;

    if (! first)
      os << ", ";

    os << " [";

    bool first1 = true;

    Coords::const_iterator pc1, pc2;

    for (pc1 = solution.icoords.begin(), pc2 = solution.icoords.end(); pc1 != pc2; ++pc1) {
      if (! first1) os << " ";

      (*pc1).print(os);

      first1 = false;
    }

    first = false;

    os << "]";
  }

  os << "]";
  }
}

//-------------

void
CNurikabe::Solution::
checkValid(Region *region) const
{
  checkValid(region->getGrid());
}

void
CNurikabe::Solution::
checkValid(Grid *grid) const
{
  Solution *th = const_cast<Solution *>(this);

  th->valid = true;

  th->ocoords.clear();

  grid->getOutsideUnknown(icoords, th->ocoords);

  grid->pushCoords(ocoords, icoords);

  try {
    grid->rebuild(true);

    if (! checkValid1(grid))
      th->valid = false;

    grid->popCoords();
  }
  catch (...) {
    th->valid = false;
    grid->popCoords();
  }

  grid->rebuild(true);
}

bool
CNurikabe::Solution::
checkValid1(Grid *grid) const
{
  Solution *th = const_cast<Solution *>(this);

  th->blackCoords.clear();
  th->whiteCoords.clear();

  if (! grid->checkValid())
    return false;

  //------

  const Pools &pools = grid->getPools();

  Pools::const_iterator pp1, pp2;

  for (pp1 = pools.begin(), pp2 = pools.end(); pp1 != pp2; ++pp1) {
    Pool *pool = *pp1;

    th->blackCoords.insert(pool->getCoords().begin(), pool->getCoords().end());
  }

  //------

  const Regions &regions = grid->getRegions();

  Regions::const_iterator pr1, pr2;

  for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    th->whiteCoords.insert(region->getCoords().begin(), region->getCoords().end());
  }

  //------

  const Islands &islands = grid->getIslands();

  Islands::const_iterator pi1, pi2;

  for (pi1 = islands.begin(), pi2 = islands.end(); pi1 != pi2; ++pi1) {
    Island *island = *pi1;

    th->whiteCoords.insert(island->getCoords().begin(), island->getCoords().end());
  }

  return true;
}

//-------------

CNurikabe::Pool::
Pool(Grid *grid) :
 grid_(grid)
{
}

void
CNurikabe::Pool::
reset()
{
  coords_.clear();
}

void
CNurikabe::Pool::
addCoord(const Coord &coord)
{
  coords_.insert(coord);
}

void
CNurikabe::Pool::
solve()
{
  log("pool solve");

  // check for black L shapes
  grid_->startChange();

  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
    Cell *cell = grid_->getCell(*p1);

    // if L shape at this black cell then corner must be white
    Cell *lcell = cell->getLShapeCornerCell();

    if (lcell)
      lcell->setWhite();
  }

  grid_->endChange("black l shape");

  //------

  // if more than 1 pool then set single expand point for this pool to black
  if (grid_->getNumPools() > 1) {
    Coords icoords = coords_;
    Coords ocoords;

    grid_->getOutsideUnknown(icoords, ocoords);

    logicAssert(grid_, ! ocoords.empty(), "no expansion for pool");

    while (ocoords.size() == 1) {
      const Coord &coord = *ocoords.begin();

      Cell *cell = grid_->getCell(coord);

      grid_->startChange();

      cell->setBlack();

      grid_->endChange("single expand for pool");

      icoords.insert(coord);

      ocoords.clear();

      grid_->getOutsideUnknown(icoords, ocoords);
    }
  }
}

bool
CNurikabe::Pool::
isValid() const
{
  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
    Cell *cell = grid_->getCell(*p1);

    Cell *cellS  = cell->getS ();
    Cell *cellE  = cell->getE ();
    Cell *cellSE = cell->getSE();

    // ensure no black pool
    int n = 1;

    if (cellE  && cellE ->isBlack()) ++n;
    if (cellS  && cellS ->isBlack()) ++n;
    if (cellSE && cellSE->isBlack()) ++n;

    if (n == 4)
      return false;

    Cell *cellN = cell->getN();
    Cell *cellW = cell->getW();

    // ensure not surrounded by white
    n = 0;

    if (! cellN || cellN->isWhite()) ++n;
    if (! cellS || cellS->isWhite()) ++n;
    if (! cellE || cellE->isWhite()) ++n;
    if (! cellW || cellW->isWhite()) ++n;

    if (n == 4)
      return false;
  }

  bool otherPools = (grid_->getNumPools() > 1);

  if (otherPools) {
    Coords ocoords;

    grid_->getOutsideUnknown(coords_, ocoords);

    if (ocoords.empty())
      return false;

    // check if pool can expand to connected to another pool
    bool found = false;

    Coords::const_iterator pc1, pc2;

    for (pc1 = ocoords.begin(), pc2 = ocoords.end(); pc1 != pc2; ++pc1) {
      const Coord &coord = *pc1;

      Cell *cell = grid_->getCell(coord);

      Coords coords;

      if (grid_->isOtherPoolReachable(cell, this, coords)) {
        found = true;
        break;
      }
    }

    if (! found)
      return false;
  }

  return true;
}

void
CNurikabe::Pool::
print() const
{
  print(std::cout);
}

void
CNurikabe::Pool::
print(std::ostream &os) const
{
  bool first = true;

  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
    if (! first) os << " ";

    (*p1).print(os);

    first = false;
  }
}

//-------------

CNurikabe::Island::
Island(Grid *grid) :
 grid_(grid)
{
}

void
CNurikabe::Island::
reset()
{
  coords_.clear();
}

void
CNurikabe::Island::
addCoord(const Coord &coord)
{
  coords_.insert(coord);
}

void
CNurikabe::Island::
getCells(Cells &cells) const
{
  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1)
    cells.insert(grid_->getCell(*p1));
}

CNurikabe::Region *
CNurikabe::Island::
getRegionConstraint() const
{
  Cell *cell = grid_->getCell(*coords_.begin());

  return cell->getRegionConstraint();
}

void
CNurikabe::Island::
setRegionConstraint(Region *region)
{
  if (! grid_->isTop()) return;

  Coords::const_iterator pc1, pc2;

  for (pc1 = coords_.begin(), pc2 = coords_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    if (grid_->isTop())
      cell->setRegionConstraint(region);
  }
}

void
CNurikabe::Island::
setGaps()
{
  Coords ocoords;

  grid_->getOutsideUnknown(coords_, ocoords);

  Coords::const_iterator pc1, pc2;

  for (pc1 = ocoords.begin(), pc2 = ocoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    assert(cell->inGap());

    gaps_.insert(cell->getGap());
  }
}

void
CNurikabe::Island::
solve()
{
  log("island solve");

  Coords icoords = coords_;
  Coords ocoords;

  grid_->getOutsideUnknown(icoords, ocoords);

  logicAssert(grid_, ! ocoords.empty(), "no expansion for island region");

  // set single expand point for this island to white
  while (ocoords.size() == 1) {
    const Coord &coord = *ocoords.begin();

    grid_->startChange();

    Cell *openCell = grid_->getCell(coord);

    openCell->setWhite();

    grid_->endChange("single expand for island");

    icoords.insert(coord);

    ocoords.clear();

    grid_->getOutsideUnknown(icoords, ocoords);
  }

  //------

  // check if first cell can reach any incomplete regions
  Regions connectRegions;

  Cell *cell = grid_->getCell(*coords_.begin());

  const Regions &regions = grid_->getRegions();

  Regions::const_iterator pr1, pr2;

  for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    if (grid_->canConnectToRegion(cell, region))
      connectRegions.insert(region);
  }

  logicAssert(grid_, ! connectRegions.empty(), "island can't connect");

  if (connectRegions.size() == 1) {
    Region *region = *connectRegions.begin();

    setRegionConstraint(region);

    // TODO: get common coords of solutions
  }
}

void
CNurikabe::Island::
checkSolutions(const Solutions &solutions)
{
  logicAssert(grid_, ! solutions.empty(), "no valid solutions for island");

  CoordsArray icoordsArray;

  Solutions::iterator ps1, ps2;

  for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
    const Solution &solution = *ps1;

    icoordsArray.push_back(solution.icoords);
  }

  grid_->startChange();

  Coords commonICoords = grid_->getCommonCoords(icoordsArray);

  Coords::const_iterator pc1, pc2;

  for (pc1 = commonICoords.begin(), pc2 = commonICoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    cell->setWhite();
  }

  grid_->endChange("common island solution");
}

bool
CNurikabe::Island::
isValid() const
{
  Coords ocoords;

  grid_->getOutsideUnknown(coords_, ocoords);

  if (ocoords.empty())
    return false;

  return true;
}

void
CNurikabe::Island::
print() const
{
  print(std::cout);
}

void
CNurikabe::Island::
print(std::ostream &os) const
{
  bool first = true;

  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
    if (! first) os << " ";

    (*p1).print(os);

    first = false;
  }
}

//-------------

CNurikabe::Gap::
Gap(Grid *grid) :
 grid_(grid)
{
}

void
CNurikabe::Gap::
reset()
{
  coords_ .clear();
  regions_.clear();
  islands_.clear();
}

void
CNurikabe::Gap::
addCoord(const Coord &coord)
{
  coords_.insert(coord);
}

void
CNurikabe::Gap::
addRegion(Region *region)
{
  regions_.insert(region);
}

void
CNurikabe::Gap::
addIsland(Island *island)
{
  Region *region = island->getRegionConstraint();

  if (region != NULL)
    regions_.insert(region);
  else
    islands_.insert(island);
}

void
CNurikabe::Gap::
solve()
{
  log("gap solve");

  Coords ocoords;

  grid_->getOutside(coords_, ocoords);

  // If all outside coords are black, must all be black
  bool all_black = true;

  Coords::iterator pc1, pc2;

  for (pc1 = ocoords.begin(), pc2 = ocoords.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    if (! cell->isBlack()) {
      all_black = false;
      break;
    }
  }

  if (all_black) {
    for (pc1 = coords_.begin(), pc2 = coords_.end(); pc1 != pc2; ++pc1) {
      Cell *cell = grid_->getCell(*pc1);

      grid_->startChange();

      cell->setBlack();

      grid_->endChange("all outside black");
    }
  }

  //------

  // Solve unknowns
  for (pc1 = coords_.begin(), pc2 = coords_.end(); pc1 != pc2; ++pc1) {
    Cell *cell = grid_->getCell(*pc1);

    grid_->solveUnknown(cell);
  }

  //------

  if (islands_.empty()) {
    for (pc1 = coords_.begin(), pc2 = coords_.end(); pc1 != pc2; ++pc1) {
      Cell *cell = grid_->getCell(*pc1);

      bool    found  = false;
      Region *region = NULL;

      Regions::const_iterator pr1, pr2;

      for (pr1 = regions_.begin(), pr2 = regions_.end(); pr1 != pr2; ++pr1) {
        Region *region1 = *pr1;

        if (grid_->canConnectToRegion(cell, region1)) {
          if (found) {
            region = NULL;
            break;
          }

          found  = true;
          region = region1;
        }
      }

      if      (! found) {
        //std::cerr << cell->getCoord() << " " << "can't connect" << std::endl;

        grid_->startChange();

        cell->setBlack();

        grid_->endChange("can't connect");
      }
      else if (region) {
        if (grid_->isTop())
          cell->setRegionConstraint(region);
      }
    }
  }
}

bool
CNurikabe::Gap::
isValid() const
{
  return true;
}

void
CNurikabe::Gap::
print() const
{
  print(std::cout);
}

void
CNurikabe::Gap::
print(std::ostream &os) const
{
  bool first = true;

  Coords::const_iterator p1, p2;

  for (p1 = coords_.begin(), p2 = coords_.end(); p1 != p2; ++p1) {
    if (! first) os << " ";

    (*p1).print(os);

    first = false;
  }
}

//-------------

CNurikabe::Cell::
Cell(Grid *grid, int value, const Coord &coord) :
 grid_(grid), coord_(coord), value_(value), solution_(UNKNOWN), region_constraint_(NULL),
 region_(NULL), pool_(NULL), island_(NULL), gap_(NULL)
{
}

void
CNurikabe::Cell::
init()
{
  value_             = UNKNOWN;
  solution_          = UNKNOWN;
  region_constraint_ = NULL;
  region_            = NULL;
  pool_              = NULL;
  island_            = NULL;
  gap_               = NULL;
}

int
CNurikabe::Cell::
charToValue(char c)
{
  if (isdigit(c))
    return c - '0';
  else if (c == '_')
    return Cell::UNKNOWN;
  else if (c == '.')
    return Cell::WHITE;
  else if (c == '*')
    return Cell::BLACK;
  else if (c >= 'A' && c <= 'Z')
    return c - 'A' + 10;
  else {
    std::cerr << "Bad board character '" << c << "'" << std::endl;
    return Cell::UNKNOWN;
  }
}

// only called on board setup
void
CNurikabe::Cell::
setValue(int value)
{
  value_ = value;
}

void
CNurikabe::Cell::
setSolution(int solution)
{
  solution_ = solution;
}

void
CNurikabe::Cell::
setRegion(Region *region)
{
  if (getRegion() == region) return;

  logicAssert(grid_, getRegion() == NULL, "cell already has region");

  region_ = region;
}

bool
CNurikabe::Cell::
isUnknown() const
{
  if (value_ != UNKNOWN) return false;

  if (! grid_->isTop())
    return ! grid_->isWhiteCoord(coord_) && ! grid_->isBlackCoord(coord_);
  else
    return true;
}

bool
CNurikabe::Cell::
isWhite() const
{
  if (value_ == WHITE) return true;

  if (value_ != UNKNOWN) return false;

  if (! grid_->isTop())
    return grid_->isWhiteCoord(coord_);
  else
    return false;
}

bool
CNurikabe::Cell::
isBlack() const
{
  if (value_ == BLACK) return true;

  if (value_ != UNKNOWN) return false;

  if (! grid_->isTop())
    return grid_->isBlackCoord(coord_);
  else
    return false;
}

bool
CNurikabe::Cell::
isNumber() const
{
  return value_ > 0;
}

bool
CNurikabe::Cell::
isNumber(int value) const
{
  return (value_ == value);
}

int
CNurikabe::Cell::
getNumber() const
{
  logicAssert(grid_, isNumber(), "cell not a number");

  return value_;
}

void
CNurikabe::Cell::
setWhite()
{
  // invalidates outside coords
  // invalidates regions, islands and gaps

  if (isNumberOrWhite()) return;

  logicAssert(grid_, isUnknown(), "cell must be unknown");

  if (grid_->isTop()) {
    logicAssert(grid_, solution_ == UNKNOWN || solution_ == WHITE, "no solution match");

    value_ = WHITE;

    assert(grid_->inChange());

    grid_->setChanged();

    grid_->addChange(coord_);
  }
  else
    grid_->addWhiteCoord(coord_);

  // TODO: move logic check ??
  Cell *cellN = getN();
  Cell *cellS = getS();
  Cell *cellE = getE();
  Cell *cellW = getW();

  Regions regions;

  if (cellN && cellN->inRegion()) regions.insert(cellN->getRegion());
  if (cellS && cellS->inRegion()) regions.insert(cellS->getRegion());
  if (cellE && cellE->inRegion()) regions.insert(cellE->getRegion());
  if (cellW && cellW->inRegion()) regions.insert(cellW->getRegion());

  if (! regions.empty()) {
    if (regions.size() > 1)
      logicAssert(grid_, isUnknown(), "white next to multiple regions");
  }
}

void
CNurikabe::Cell::
setBlack()
{
  // invalidates outside coords
  // invalidates pools

  if (isBlack()) return;

  logicAssert(grid_, isUnknown(), "cell must be unknown");

  logicAssert(grid_, ! pool_, "cell already in pool");

  if (grid_->isTop()) {
    logicAssert(grid_, solution_ == UNKNOWN || solution_ == BLACK, "no solution match");

    value_ = BLACK;

    assert(grid_->inChange());

    grid_->setChanged();

    grid_->addChange(coord_);
  }
  else
    grid_->addBlackCoord(coord_);
}

void
CNurikabe::Cell::
setRegionConstraint(Region *region)
{
  int number = region->getValue();

  logicAssert(grid_, grid_->isTop(), "not at top level");

  // validate value
  const Regions &regions = grid_->getRegions();

  Regions::const_iterator pr1, pr2;

  for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
    Region *region = *pr1;

    if (! region->hasValidSolution()) continue;

    const Solution &solution = region->getSolution();

    if (solution.icoords.find(coord_) != solution.icoords.end())
      logicAssert(grid_, region->getValue() == number, "no solution match");
  }

  if      (region_constraint_ == NULL)
    region_constraint_ = region;
  else if (region_constraint_ != region)
    region_constraint_ = BLACK_REGION_CONSTRAINT;
}

void
CNurikabe::Cell::
reset()
{
  logicAssert(grid_, grid_->isTop(), "not at top level");

  if (! isNumber())
    value_ = UNKNOWN;

  region_constraint_ = NULL;

  resetPointers();

  region_ = NULL;
}

void
CNurikabe::Cell::
resetPointers()
{
  if (! isNumber())
    region_ = NULL;

  pool_   = NULL;
  island_ = NULL;
  gap_    = NULL;
}

void
CNurikabe::Cell::
resetPoolPointer()
{
  pool_ = NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getLShapeCornerCell() const
{
  // check for black L shape
  logicAssert(grid_, isBlack(), "l shape cell not black");

  Cell *cellN = getN();
  Cell *cellE = getE();

  if (cellN && cellN->isBlack() && cellE && cellE->isBlack()) {
    Cell *lcell = getNE();

    logicAssert(grid_, ! lcell->isBlack(), "invalid black pool");

    if (lcell->isUnknown())
      return lcell;
  }

  Cell *cellW = getW();

  if (cellN && cellN->isBlack() && cellW && cellW->isBlack()) {
    Cell *lcell = getNW();

    logicAssert(grid_, ! lcell->isBlack(), "invalid black pool");

    if (lcell->isUnknown())
      return lcell;
  }

  Cell *cellS = getS();

  if (cellS && cellS->isBlack() && cellE && cellE->isBlack()) {
    Cell *lcell = getSE();

    logicAssert(grid_, ! lcell->isBlack(), "invalid black pool");

    if (lcell->isUnknown())
      return lcell;
  }

  if (cellS && cellS->isBlack() && cellW && cellW->isBlack()) {
    Cell *lcell = getSW();

    logicAssert(grid_, ! lcell->isBlack(), "invalid black pool");

    if (lcell->isUnknown())
      return lcell;
  }

  return NULL;
}

CNurikabe::Pool *
CNurikabe::Cell::
getPool() const
{
  if (! isBlack()) return NULL;

  assert(pool_);

  return pool_;
}

void
CNurikabe::Cell::
buildPool()
{
  logicAssert(grid_, ! pool_, "No pool");

  Cells cells;

  grid_->addConnectedBlack(this, cells);

  pool_ = grid_->createPool();

  Cells::const_iterator p1, p2;

  for (p1 = cells.begin(), p2 = cells.end(); p1 != p2; ++p1) {
    Cell *cell = *p1;

    logicAssert(grid_, cell == this || ! cell->pool_, "cell already in pool");

    const Coord &coord = cell->getCoord();

    pool_->addCoord(coord);

    cell->pool_ = pool_;
  }
}

void
CNurikabe::Cell::
buildIsland()
{
  Cells cells;

  grid_->addConnectedWhite(this, cells);

  logicAssert(grid_, ! island_, "no island");

  island_ = grid_->createIsland();

  Cells::const_iterator p1, p2;

  for (p1 = cells.begin(), p2 = cells.end(); p1 != p2; ++p1) {
    Cell *cell = *p1;

    logicAssert(grid_, cell == this || ! cell->island_, "cell already in island");

    island_->addCoord(cell->getCoord());

    cell->island_ = island_;
  }
}

void
CNurikabe::Cell::
buildGap()
{
  gap_ = grid_->createGap();

  Cells cells;

  grid_->addConnectedUnknown(this, cells);

  // get bordering regions

  Cells::const_iterator pc1, pc2;

  for (pc1 = cells.begin(), pc2 = cells.end(); pc1 != pc2; ++pc1) {
    Cell *cell = *pc1;

    gap_->addCoord(cell->getCoord());

    cell->gap_ = gap_;

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    if (cellN && cellN->inRegion()) gap_->addRegion(cellN->getRegion());
    if (cellS && cellS->inRegion()) gap_->addRegion(cellS->getRegion());
    if (cellE && cellE->inRegion()) gap_->addRegion(cellE->getRegion());
    if (cellW && cellW->inRegion()) gap_->addRegion(cellW->getRegion());

    if (cellN && cellN->inIsland()) gap_->addIsland(cellN->getIsland());
    if (cellS && cellS->inIsland()) gap_->addIsland(cellS->getIsland());
    if (cellE && cellE->inIsland()) gap_->addIsland(cellE->getIsland());
    if (cellW && cellW->inIsland()) gap_->addIsland(cellW->getIsland());
  }
}

void
CNurikabe::Cell::
getRegions(Regions &regions) const
{
  Cell *th = const_cast<Cell *>(this);

  if (isUnknown()) {
    Gap *gap = getGap();

    const Regions &regions1 = gap->getRegions();

    Regions::const_iterator p1, p2;

    for (p1 = regions1.begin(), p2 = regions1.end(); p1 != p2; ++p1)
      if (grid_->canConnectToRegion(th, *p1))
        regions.insert(*p1);
  }
}

void
CNurikabe::Cell::
getConnectedCoords(Coords &coords)
{
  if      (getPool())
    coords = getPool()->getCoords();
  else if (getRegion())
    coords = getRegion()->getCoords();
  else if (getIsland())
    coords = getIsland()->getCoords();
  else if (getGap())
    coords = getGap()->getCoords();
}

CNurikabe::Cell *
CNurikabe::Cell::
getN(int count) const
{
  if (count > 1) {
    Cell *cell = getN(count - 1);
    if (! cell) return NULL;

    return cell->getN(1);
  }

  if (coord_.row > 0)
    return grid_->getCell(Coord(coord_.row - 1, coord_.col));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getS(int count) const
{
  if (count > 1) {
    Cell *cell = getS(count - 1);
    if (! cell) return NULL;

    return cell->getS(1);
  }

  if (coord_.row < grid_->getNumRows() - 1)
    return grid_->getCell(Coord(coord_.row + 1, coord_.col));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getW(int count) const
{
  if (count > 1) {
    Cell *cell = getW(count - 1);
    if (! cell) return NULL;

    return cell->getW(1);
  }

  if (coord_.col > 0)
    return grid_->getCell(Coord(coord_.row, coord_.col - 1));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getE(int count) const
{
  if (count > 1) {
    Cell *cell = getE(count - 1);
    if (! cell) return NULL;

    return cell->getE(1);
  }

  if (coord_.col < grid_->getNumCols() - 1)
    return grid_->getCell(Coord(coord_.row, coord_.col + 1));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getNE(int count) const
{
  if (count > 1) {
    Cell *cell = getNE(count - 1);
    if (! cell) return NULL;

    return cell->getNE(1);
  }

  if (coord_.row > 0 && coord_.col < grid_->getNumCols() - 1)
    return grid_->getCell(Coord(coord_.row - 1, coord_.col + 1));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getSE(int count) const
{
  if (count > 1) {
    Cell *cell = getSE(count - 1);
    if (! cell) return NULL;

    return cell->getSE(1);
  }

  if (coord_.row < grid_->getNumRows() - 1 && coord_.col < grid_->getNumCols() - 1)
    return grid_->getCell(Coord(coord_.row + 1, coord_.col + 1));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getSW(int count) const
{
  if (count > 1) {
    Cell *cell = getSW(count - 1);
    if (! cell) return NULL;

    return cell->getSW(1);
  }

  if (coord_.row < grid_->getNumRows() - 1 && coord_.col > 0)
    return grid_->getCell(Coord(coord_.row + 1, coord_.col - 1));
  else
    return NULL;
}

CNurikabe::Cell *
CNurikabe::Cell::
getNW(int count) const
{
  if (count > 1) {
    Cell *cell = getNW(count - 1);
    if (! cell) return NULL;

    return cell->getNW(1);
  }

  if (coord_.row > 0 && coord_.col > 0)
    return grid_->getCell(Coord(coord_.row - 1, coord_.col - 1));
  else
    return NULL;
}

std::string
CNurikabe::Coord::
str() const
{
  return "(" + intToString(row) + "," + intToString(col) + ")";
}
