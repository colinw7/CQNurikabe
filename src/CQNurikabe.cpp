#include <CQNurikabe.h>

#include <CHRTimer.h>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QStatusBar>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>

#include <help.xpm>

#include <iostream>

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  CQNurikabeApp nurikabe;

  return app.exec();
}

CQNurikabeApp::
CQNurikabeApp()
{
  nurikabe_ = new CQNurikabe(this);

  QFrame *frame = new QFrame();

  QVBoxLayout *frame_layout   = new QVBoxLayout;
  QHBoxLayout *nframe_layout  = new QHBoxLayout;
  QVBoxLayout *neframe_layout = new QVBoxLayout;

  frame_layout  ->setMargin(0); frame_layout  ->setSpacing(0);
  nframe_layout ->setMargin(0); nframe_layout ->setSpacing(0);
  neframe_layout->setMargin(0); neframe_layout->setSpacing(0);

  frame->setLayout(frame_layout);

  //----

  status_bar_ = new QStatusBar();

  frame_layout->addLayout(nframe_layout, 1);
  frame_layout->addWidget(status_bar_);

  //----

  canvas_ = new CQNurikabeCanvas(this);

  nframe_layout->addWidget(canvas_, 1);
  nframe_layout->addLayout(neframe_layout);

  //----

  QToolButton *help_button = new QToolButton;

  help_button->setAutoRaise(true);
  help_button->setIcon(QIcon(QPixmap(help_xpm)));

  help_button->setFocusPolicy(Qt::NoFocus);

  help_button->setToolTip("Canvas shortcuts:\n"
                          " C   : Show connections\n"
                          " S   : Show solution\n"
                          " Tab : Next solution\n"
                          " B   : Set black cell\n"
                          " W   : Set white cell\n"
                          " F1  : Check reachable\n"
                          " F2  : Show solutions\n"
                          " F3  : Generate\n"
                          " F4  : Print constraints\n"
                          " F5  : Draw constraints\n"
                          " F8  : Commit\n"
                          " +   : Increase solution depth\n"
                          " -   : Decrease solution depth\n"
                          " Del : Reset cell\n"
                          " O   : Rebuild\n"
                          " P   : Play solution\n"
                          " U   : Unplay solution\n"
                          " R   : Get regions\n"
                          " V   : Check valid\n"
                          " 1-9 : Choose puzzle\n"
                          " .   : Print\n"
                          " Esc : Cancel solve");

  //----

  QPushButton *solve_button = new QPushButton("Solve");

  solve_button->setToolTip("Solve Puzzle");

  solve_button->setFocusPolicy(Qt::NoFocus);

  canvas_->connect(solve_button, SIGNAL(clicked()), SLOT(solve()));

  //----

  QPushButton *step_button = new QPushButton("Step");

  step_button->setToolTip("Solve Puzzle (Single Step)");

  step_button->setFocusPolicy(Qt::NoFocus);

  canvas_->connect(step_button, SIGNAL(clicked()), SLOT(step()));

  //----

  QPushButton *reset_button = new QPushButton("Reset");

  reset_button->setToolTip("Reset Puzzle");

  reset_button->setFocusPolicy(Qt::NoFocus);

  canvas_->connect(reset_button, SIGNAL(clicked()), SLOT(reset()));

  //----

  neframe_layout->addWidget (help_button);
  neframe_layout->addStretch(1);
  neframe_layout->addWidget (solve_button);
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (step_button);
  neframe_layout->addSpacing(8);
  neframe_layout->addWidget (reset_button);
  neframe_layout->addSpacing(8);

  //----

  frame->resize(600, 500);

  frame->show();

  CNurikabe *nurikabe = getNurikabe();

  showMessage(QString("Ready (%1)").arg(nurikabe->getGrid()->getCoordDepth()));
}

void
CQNurikabeApp::
solve()
{
  if (nurikabe_->isBusy()) return;

  CNurikabe *nurikabe = getNurikabe();

  nurikabe->solve();

  //nurikabe->print(std::cout);

  showMessage(QString("Ready (%1)").arg(nurikabe->getGrid()->getCoordDepth()));
}

void
CQNurikabeApp::
step()
{
  if (nurikabe_->isBusy()) return;

  CNurikabe *nurikabe = getNurikabe();

  nurikabe->solveStep();

  //nurikabe->print(std::cout);

  showMessage(QString("Ready (%1)").arg(nurikabe->getGrid()->getCoordDepth()));
}

void
CQNurikabeApp::
reset()
{
  if (nurikabe_->isBusy()) return;

  CNurikabe *nurikabe = getNurikabe();

  try {
    nurikabe_->reset();
  }
  catch (...) {
  }

  canvas_->redraw();

  showMessage(QString("Ready (%1)").arg(nurikabe->getGrid()->getCoordDepth()));
}

void
CQNurikabeApp::
showMessage(const QString &str)
{
  status_bar_->showMessage(str);
}

//-------------

CQNurikabeCanvas::
CQNurikabeCanvas(CQNurikabeApp *app, QWidget *parent) :
 QWidget(parent), app_(app), dx_(0), dy_(0), cell_size_(8), currentCell_(NULL),
 showConnection_(false), showSolution_(false), showSolutions_(false), drawConstraint_(false),
 solutionNum_(0), showSolutionsDepth_(8), escape_(false)
{
  setFocusPolicy(Qt::StrongFocus);

  setWindowTitle("Nurikabe");
}

void
CQNurikabeCanvas::
redraw()
{
  draw();

  update();
}

void
CQNurikabeCanvas::
mousePressEvent(QMouseEvent *e)
{
  CNurikabe::Cell *cell = xyToCell(e->pos().x(), e->pos().y());

  if (cell != currentCell_)
    currentCell_ = cell;
  else
    currentCell_ = NULL;

  solutions_.clear();

  if (showSolutions_) {
    showSolutions();
  }

  graph_.reset();

  redraw();
}

void
CQNurikabeCanvas::
mouseMoveEvent(QMouseEvent *)
{
}

void
CQNurikabeCanvas::
keyPressEvent(QKeyEvent *e)
{
  CNurikabe *nurikabe = app_->getNurikabe();

  if      (e->key() == Qt::Key_C) {
    showConnection_ = ! showConnection_;
    solutionNum_    = 0;

    if (showConnection_) { showSolution_ = false; showSolutions_ = false; }

    redraw();
  }
  else if (e->key() == Qt::Key_S) {
    showSolution_ = ! showSolution_;
    solutionNum_  = 0;

    if (showSolution_) { showConnection_ = false; showSolutions_ = false; }

    redraw();
  }
  else if (e->key() == Qt::Key_Tab) {
    if (showSolution_ || showConnection_ || showSolutions_) {
      ++solutionNum_;

      redraw();
    }
  }
  else if (e->key() == Qt::Key_B) {
    if (currentCell_ && currentCell_->isUnknown()) {
      try {
        nurikabe->setCellBlack(currentCell_);
      }
      catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
      }

      redraw();
    }
  }
  else if (e->key() == Qt::Key_W) {
    if (currentCell_ && currentCell_->isUnknown()) {
      try {
        nurikabe->setCellWhite(currentCell_);
      }
      catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
      }

      redraw();
    }
  }
  else if (e->key() == Qt::Key_F1) {
    if (currentCell_ && currentCell_->isUnknown()) {
      //bool rc = nurikabe->getGrid()->isNumberReachable(currentCell_);

      //std::cerr << (rc ? "Reachable" : "Unreachable") << std::endl;
    }
  }
  else if (e->key() == Qt::Key_F2) {
    if (currentCell_) {
      CNurikabe::Region *region = currentCell_->getRegion();

      region->setChanged();
    }

    showSolutions_ = ! showSolutions_;

    showSolutions();

    redraw();
  }
  else if (e->key() == Qt::Key_F3) {
    nurikabe->getGrid()->generate();

    redraw();
  }
  else if (e->key() == Qt::Key_F4) {
    if (currentCell_) {
      CNurikabe::Region *region = currentCell_->getRegion();

      if (region)
        region->printConstraints();
    }
  }
  else if (e->key() == Qt::Key_F5) {
    drawConstraint_ = ! drawConstraint_;

    redraw();
  }
  else if (e->key() == Qt::Key_F8) {
    nurikabe->commit();

    redraw();
  }
  else if (e->key() == Qt::Key_Plus) {
    if (currentCell_) {
      CNurikabe::Region *region = currentCell_->getRegion();

      region->setChanged();
    }

    ++showSolutionsDepth_;

    if (showSolutions_) {
      showSolutions();

      redraw();
    }
  }
  else if (e->key() == Qt::Key_Minus) {
    if (currentCell_) {
      CNurikabe::Region *region = currentCell_->getRegion();

      region->setChanged();
    }

    if (showSolutionsDepth_ > 1)
      --showSolutionsDepth_;

    if (showSolutions_) {
      showSolutions();

      redraw();
    }
  }
  else if (e->key() == Qt::Key_Delete) {
    if (currentCell_ && (currentCell_->isWhite() || currentCell_->isBlack())) {
      try {
        currentCell_->reset();
      }
      catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
      }

      redraw();
    }
  }
  else if (e->key() == Qt::Key_O) {
    if (currentCell_) {
      if       (currentCell_->isBlack())
        currentCell_->buildPool();
      else if (currentCell_->isWhite() || currentCell_->isNumber())
        currentCell_->getGrid()->buildRegions();

      buildGraph();

      redraw();
    }
  }
  else if (e->key() == Qt::Key_P) {
    const CNurikabe::Solution &solution = getCurrentSolution();

    nurikabe->playSolution(solution);

    redraw();
  }
  else if (e->key() == Qt::Key_U) {
    nurikabe->unplaySolution();

    redraw();
  }
  else if (e->key() == Qt::Key_R) {
    if (currentCell_) {
      CNurikabe::Regions regions;

      currentCell_->getRegions(regions);

      CNurikabe::Regions::const_iterator p1, p2;

      for (p1 = regions.begin(), p2 = regions.end(); p1 != p2; ++p1)
        std::cout << " " << (*p1)->getValue();
      std::cout << " " << std::endl;
    }
  }
  else if (e->key() == Qt::Key_V) {
    if (currentCell_) {
      CNurikabe::Region *region = currentCell_->getRegion();

      if (region) {
        escape_ = false;

        solutions_ = nurikabe->getRegionSolutions(region);

        const CNurikabe::Solution &solution = getCurrentSolution(solutions_);

        solution.checkValid(region);
      }
    }
  }
  else if (e->key() >= Qt::Key_1 && e->key() <= Qt::Key_9) {
    currentCell_ = NULL;

    solutions_.clear();

    nurikabe->setPuzzle(e->key() - Qt::Key_0);

    resizeEvent(NULL);

    redraw();
  }
  else if (e->key() == Qt::Key_Period) {
    nurikabe->print();
  }
  else if (e->key() == Qt::Key_Escape) {
    escape_ = true;
  }
}

void
CQNurikabeCanvas::
showSolutions()
{
  CNurikabe *nurikabe = app_->getNurikabe();

  solutions_.clear();

  if (currentCell_ && currentCell_->inRegion()) {
    escape_ = false;

    CNurikabe::Region *region = currentCell_->getRegion();

    int maxSolutions = nurikabe->getGrid()->getMaxSolutions();

    nurikabe->getGrid()->setMaxSolutions(999999);

    nurikabe->setBusy(true);

    try {
      region->setMaxDepth(showSolutionsDepth_);

      region->buildSolutions(solutions_);

      region->setMaxDepth(INT_MAX);
    }
    catch (...) {
    }

    nurikabe->getGrid()->setMaxSolutions(maxSolutions);

    nurikabe->setBusy(false);
  }
}

void
CQNurikabeCanvas::
buildGraph()
{
  CNurikabe::Coords coords;

  currentCell_->getConnectedCoords(coords);

  graph_.reset();

  CNurikabe::Coords::const_iterator p1, p2;

  for (p1 = coords.begin(), p2 = coords.end(); p1 != p2; ++p1) {
    const CNurikabe::Coord &c1 = *p1;

    auto node1 = graph_.addNode(c1);

    CNurikabe::Coords::const_iterator p3 = p1;

    ++p3;

    for ( ; p3 != p2; ++p3) {
      const CNurikabe::Coord &c2 = *p3;

      auto node2 = graph_.addNode(c2);

      int dr = abs(c1.row - c2.row);
      int dc = abs(c1.col - c2.col);

      if (dr > 1 || dc > 1 || dr + dc > 1) continue;

      graph_.addEdge(node1, node2, 1);
    }
  }

  //mst_ = graph_.minimumSpaningTree();
}

void
CQNurikabeCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter;

  painter.begin(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.end();
}

void
CQNurikabeCanvas::
resizeEvent(QResizeEvent *)
{
  int width  = this->width ();
  int height = this->height();

  pixmap_ = QPixmap(width, height);

  const CNurikabe *nurikabe = app_->getNurikabe();

  int num_rows = nurikabe->getNumRows();
  int num_cols = nurikabe->getNumCols();

  int rsize = (0.9*height)/num_rows;
  int csize = (0.9*width )/num_cols;

  cell_size_ = std::min(rsize, csize);

  int xsize = num_cols*cell_size_;
  int ysize = num_rows*cell_size_;

  dx_ = (width  - xsize)/2;
  dy_ = (height - ysize)/2;

  double font_size = std::max(cell_size_*0.45, 1.0);

  font_  = QFont("helvetica", font_size);
  sfont_ = QFont("helvetica", font_size/2.0);

  QFontMetrics fm (font_);
  QFontMetrics fm1(sfont_);

  char_height_  = fm.height ();
  char_descent_ = fm.descent();

  schar_height_  = fm1.height ();
  schar_descent_ = fm1.descent();

  draw();
}

void
CQNurikabeCanvas::
draw()
{
  static bool in_draw;

  if (in_draw) return;

  in_draw = true;

  pixmap_.fill(Qt::white);

  //-----

  QPainter painter;

  painter.begin(&pixmap_);

  //-----

  drawBoard(&painter);

  //-----

  if      (showConnection_) {
    const CNurikabe *nurikabe = app_->getNurikabe();

    if (currentCell_ != NULL && currentCell_->inRegion()) {
      CNurikabe::Region *region = currentCell_->getRegion();

      escape_ = false;

      solutions_ = nurikabe->getRegionSolutions(region);

      drawCurrentConnection(&painter, solutions_);
    }
    else {
      const CNurikabe::Regions &regions = nurikabe->getRegions();

      CNurikabe::Regions::const_iterator pr1, pr2;

      for (pr1 = regions.begin(), pr2 = regions.end(); pr1 != pr2; ++pr1) {
        CNurikabe::Region *region = *pr1;

        escape_ = false;

        solutions_ = nurikabe->getRegionSolutions(region);

        drawConnections(&painter, solutions_);
      }
    }
  }
  else if (showSolution_) {
    const CNurikabe *nurikabe = app_->getNurikabe();

    if (currentCell_ != NULL && currentCell_->inRegion()) {
      CNurikabe::Region *region = currentCell_->getRegion();

      escape_ = false;

      solutions_ = nurikabe->getRegionSolutions(region, showSolutionsDepth_);

      drawCurrentSolution(&painter, solutions_);
    }
  }
  else if (showSolutions_) {
    drawCurrentSolution(&painter, solutions_);
  }

  //-----

  const Graph::EdgeList &edges = graph_.getEdges();

  Graph::EdgeList::const_iterator pedge1 = edges.begin();
  Graph::EdgeList::const_iterator pedge2 = edges.end  ();

  for ( ; pedge1 != pedge2; ++pedge1) {
    auto node1 = (*pedge1)->fromNode();
    auto node2 = (*pedge1)->toNode  ();

    const CNurikabe::Coord &c1 = node1->getData();
    const CNurikabe::Coord &c2 = node2->getData();

    int x1, y1, x2, y2;

    cellToXY(c1.row, c1.col, &x1, &y1);
    cellToXY(c2.row, c2.col, &x2, &y2);

    painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 2.0));

    painter.drawLine(x1 + cell_size_/2, y1 + cell_size_/2, x2 + cell_size_/2, y2 + cell_size_/2);
  }

  //-----

  painter.end();

  in_draw = false;
}

void
CQNurikabeCanvas::
drawBoard(QPainter *painter)
{
  const CNurikabe *nurikabe = app_->getNurikabe();

  int num_rows = nurikabe->getNumRows();
  int num_cols = nurikabe->getNumCols();

  QFontMetrics fm(font_);

  int y = dy_;

  for (int i = 0; i < num_rows; ++i) {
    int x = dx_;

    for (int j = 0; j < num_cols; ++j) {
       CNurikabe::Coord coord(i, j);

       const CNurikabe::Cell *cell = nurikabe->getCell(coord);

       if      (cell->isNumber()) {
         painter->setFont(font_);

         QString str = QString("%1").arg(cell->getNumber());

         int char_width = fm.horizontalAdvance(str);

         painter->setPen(QPen(Qt::black));

         painter->drawText(x + cell_size_/2 - char_width/2,
                           y + cell_size_/2 + char_height_/2 - char_descent_,
                           str);
       }
       else if (cell->isWhite()) {
         QRect rect(x, y, cell_size_, cell_size_);

         painter->fillRect(rect, QBrush(QColor(255, 255, 255)));

         if (drawConstraint_)
           drawRegionConstraint(painter, cell, x, y);
       }
       else if (cell->isBlack()) {
         QRect rect(x, y, cell_size_, cell_size_);

         painter->fillRect(rect, QBrush(QColor(0, 0, 0)));
       }
       else { // unknown
         QRect rect(x, y, cell_size_, cell_size_);

         painter->fillRect(rect, QBrush(QColor(200, 200, 200)));

         if (drawConstraint_)
           drawRegionConstraint(painter, cell, x, y);
       }

       if (cell == currentCell_) {
         QRect rect(x, y, cell_size_, cell_size_);

         painter->setPen(QColor(200, 100, 100));

         painter->drawRect(rect);

         painter->fillRect(rect, QBrush(QColor(0, 255, 0, 80)));
       }

       x += cell_size_;
     }

     y += cell_size_;
  }

  //-----

  int x1 = dx_, x2 = x1 + num_cols*cell_size_;
  int y1 = dy_, y2 = y1 + num_rows*cell_size_;

  painter->setPen(QPen(QColor(128, 128, 128)));

  x1 = dx_;
  y1 = dy_;

  for (int i = 0; i <= num_cols; ++i) {
    painter->drawLine(x1, y1, x1, y2);

    x1 += cell_size_;
  }

  x1 = dx_;
  y1 = dy_;

  for (int i = 0; i <= num_rows; ++i) {
    painter->drawLine(x1, y1, x2, y1);

    y1 += cell_size_;
  }
}

void
CQNurikabeCanvas::
drawRegionConstraint(QPainter *painter, const CNurikabe::Cell *cell, int x, int y)
{
  CNurikabe::Region *region = cell->getRegionConstraint();

  if (region == NULL) return;

  QFontMetrics fm1(sfont_);

  painter->setFont(sfont_);

  QString str = QString("%1").arg(region->getValue());

  int char_width = fm1.horizontalAdvance(str);

  painter->setPen(QPen(Qt::black));

  painter->drawText(x + cell_size_/4 - char_width/2,
                    y + cell_size_/4 + schar_height_/2 - schar_descent_,
                    str);
}

void
CQNurikabeCanvas::
drawConnections(QPainter *painter, const CNurikabe::Solutions &solutions)
{
  CNurikabe::Solutions::const_iterator ps1, ps2;

  for (ps1 = solutions.begin(), ps2 = solutions.end(); ps1 != ps2; ++ps1) {
    const CNurikabe::Solution &solution = *ps1;

    CNurikabe::Coords::const_iterator pc1, pc2;

    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    for (pc1 = solution.icoords.begin(), pc2 = solution.icoords.end(); pc1 != pc2; ++pc1) {
      const CNurikabe::Coord &coord = *pc1;

      x1 = x2;
      y1 = y2;
      x2 = dx_ + coord.col*cell_size_ + cell_size_/2;
      y2 = dy_ + coord.row*cell_size_ + cell_size_/2;

      if (x1 != 0) {
        painter->setPen(QColor(100, 100, 200));

        painter->drawLine(x1, y1, x2, y2);
      }
    }
  }
}

void
CQNurikabeCanvas::
drawCurrentConnection(QPainter *painter, const CNurikabe::Solutions &solutions)
{
  if (solutions.empty()) return;

  const CNurikabe::Solution &solution = getCurrentSolution(solutions);

  int numSolutions = solutions.size();

  app_->showMessage(QString("Solution %1 of %2").arg(solutionNum_ + 1).arg(numSolutions));

  CNurikabe::Coords::const_iterator pc1, pc2;

  int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

  for (pc1 = solution.icoords.begin(), pc2 = solution.icoords.end(); pc1 != pc2; ++pc1) {
    const CNurikabe::Coord &coord = *pc1;

    x1 = x2;
    y1 = y2;
    x2 = dx_ + coord.col*cell_size_ + cell_size_/2;
    y2 = dy_ + coord.row*cell_size_ + cell_size_/2;

    if (x1 != 0) {
      painter->setPen(QColor(100, 100, 200));

      painter->drawLine(x1, y1, x2, y2);
    }
  }
}

void
CQNurikabeCanvas::
drawCurrentSolution(QPainter *painter, const CNurikabe::Solutions &solutions)
{
  if (solutions.empty()) return;

  const CNurikabe::Solution &solution = getCurrentSolution(solutions);

  int numSolutions = solutions.size();

  app_->showMessage(QString("Solution %1 of %2").arg(solutionNum_ + 1).arg(numSolutions));

  CNurikabe *nurikabe = app_->getNurikabe();

  // TODO: prevent changed signal here
  if (showSolutions_)
    nurikabe->playSolution(solution, false);
  else
    nurikabe->playSolution(solution);

  drawBoard(painter);

  nurikabe->unplaySolution();
}

CNurikabe::Solution
CQNurikabeCanvas::
getCurrentSolution()
{
  static CNurikabe::Solution no_solution;

  if (currentCell_ && currentCell_->inRegion()) {
    CNurikabe::Region *region = currentCell_->getRegion();

    CNurikabe *nurikabe = app_->getNurikabe();

    escape_ = false;

    solutions_ = nurikabe->getRegionSolutions(region);

    return getCurrentSolution(solutions_);
  }
  else
    return no_solution;
}

CNurikabe::Solution
CQNurikabeCanvas::
getCurrentSolution(const CNurikabe::Solutions &solutions)
{
  static CNurikabe::Solution no_solution;

  int numSolutions = solutions.size();

  if (numSolutions == 0) return no_solution;

  while (solutionNum_ >= numSolutions)
    solutionNum_ -= numSolutions;

  CNurikabe::Solutions::const_iterator ps1 = solutions.begin();

  for (int i = 0; i < solutionNum_; ++i) ++ps1;

  return *ps1;
}

void
CQNurikabeCanvas::
solve()
{
  solutions_.clear();

  escape_ = false;

  app_->solve();

  redraw();
}

void
CQNurikabeCanvas::
step()
{
  solutions_.clear();

  escape_ = false;

  app_->step();

  redraw();
}

void
CQNurikabeCanvas::
reset()
{
  solutions_.clear();

  escape_ = false;

  app_->reset();

  redraw();
}

CNurikabe::Cell *
CQNurikabeCanvas::
xyToCell(int x, int y) const
{
  CNurikabe *nurikabe = app_->getNurikabe();

  int num_rows = nurikabe->getNumRows();
  int num_cols = nurikabe->getNumCols();

  int x1 = dx_, x2 = x1 + num_cols*cell_size_;
  int y1 = dy_, y2 = y1 + num_rows*cell_size_;

  if (x < x1 || x > x2|| y < y1 || y > y2)
    return NULL;

  int r = (y - dy_)/cell_size_;
  int c = (x - dx_)/cell_size_;

  if (r < 0 || r >= num_rows || c < 0 || c >= num_cols)
    return NULL;

  return nurikabe->getCell(CNurikabe::Coord(r, c));
}

void
CQNurikabeCanvas::
cellToXY(int row, int col, int *x, int *y) const
{
  *x = dx_ + col*cell_size_;
  *y = dy_ + row*cell_size_;
}

bool
CQNurikabeCanvas::
getCellSize(int *csize) const
{
  int width  = this->width ();
  int height = this->height();

  int size = int(0.9*std::min(width, height));

  *csize  = size/9;

  return true;
}

//------

CQNurikabe::
CQNurikabe(CQNurikabeApp *app) :
 app_(app), timer_(-1)
{
}

void
CQNurikabe::
setBusy(bool busy) const
{
  CQNurikabe *th = const_cast<CQNurikabe *>(this);

  CNurikabe *nurikabe = app_->getNurikabe();

  if (busy) {
    app_->showMessage(QString("Busy (%1)").arg(nurikabe->getGrid()->getCoordDepth()));

    CHRTimerMgrInst->start(&th->timer_);
  }
  else {
    app_->showMessage(QString("Ready (%1)").arg(nurikabe->getGrid()->getCoordDepth()));

    if (timer_ >= 0)
      CHRTimerMgrInst->end(timer_);

    th->timer_ = -1;
  }

  //app_->getCanvas()->update();

  QApplication::processEvents();
}

bool
CQNurikabe::
checkBreak()
{
  if (timer_ < 0) {
    QApplication::processEvents();

    return app_->getCanvas()->getEscape();
  }

  CNurikabe *nurikabe = app_->getNurikabe();

  CHRTime time = CHRTimerMgrInst->elapsed(timer_);

  CHRTime d = CHRTime::diffTime(time, CHRTime::getTime());

  app_->showMessage(QString("Busy: %1 secs (%2)").arg(d.secs).
                      arg(nurikabe->getGrid()->getCoordDepth()));

  QApplication::processEvents();

  return app_->getCanvas()->getEscape();
}

void
CQNurikabe::
notifyChanged()
{
  app_->getCanvas()->redraw();

  QApplication::processEvents();
}
