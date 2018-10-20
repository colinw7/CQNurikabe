#include <CNurikabeGen.h>

void
CNurikabeGen::
generate(int rows, int cols)
{
  rows_ = rows;
  cols_ = cols;

  cells_.resize(rows_*cols_);

  Cell *startCell = randEdge();

  startCell->setBlack();

  blackCells.push_back(startCell);

  while (true) {
    int n = blackCells.size();

    Cell *cell = blackCells[rand(n)];

    Cell *cellN = cell->getN();
    Cell *cellS = cell->getS();
    Cell *cellE = cell->getE();
    Cell *cellW = cell->getW();

    Cells openCells;

    if (cellN && ! cellN->isBlack()) openCells.insert(cellN);
    if (cellS && ! cellS->isBlack()) openCells.insert(cellS);
    if (cellE && ! cellE->isBlack()) openCells.insert(cellE);
    if (cellW && ! cellW->isBlack()) openCells.insert(cellW);

    bool found = false;

    while (true) {
      int n = openCells.size();

      if (n == 0) break;

      int i = rand(n);

      Cell *cell = openCells[i];

      openCells.erase(cell);

      cell->setBlack();

      bool rc = verify();

      if (rc) {
        found = true;
        break;
      }

      cell->setUnknown();
    }

    if (! found)
      break;
  }
}

CNurikabeGen::Cell *
CNurikabeGen::
getCell(int row, int col)
{
  return &cells_[row*cols_ + col];
}
