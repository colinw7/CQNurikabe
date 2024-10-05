#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QStatusBar>
#include <CNurikabe.h>
#include <CGraphTmpl.h>

class CQNurikabeApp;

class CQNurikabeCanvas : public QWidget {
  Q_OBJECT

 public:
  CQNurikabeCanvas(CQNurikabeApp *app, QWidget *parent = NULL);

  virtual ~CQNurikabeCanvas() { }

  void redraw();

  bool getEscape() const { return escape_; }

 private:
  void draw();

  void drawBoard(QPainter *painter);

  void drawRegionConstraint(QPainter *painter, const CNurikabe::Cell *cell, int x, int y);

  void drawConnections(QPainter *painter, const CNurikabe::Solutions &solutions);

  void drawCurrentConnection(QPainter *painter, const CNurikabe::Solutions &solutions);
  void drawCurrentSolution  (QPainter *painter, const CNurikabe::Solutions &solutions);

  CNurikabe::Solution getCurrentSolution();
  CNurikabe::Solution getCurrentSolution(const CNurikabe::Solutions &solutions);

  void buildGraph();

 protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent (QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void resizeEvent(QResizeEvent *) override;

  void showSolutions();

 private slots:
  void solve();
  void step();
  void reset();

 public:
  CNurikabe::Cell *xyToCell(int x, int y) const;

  void cellToXY(int i, int j, int *x, int *y) const;

  bool getCellSize(int *size) const;

 private:
  typedef CGraphMgr::GraphT<CNurikabe::Coord, int>        Graph;
  typedef CGraphMgr::GraphT<CNurikabe::Coord, int>::NodeP NodeP;

  CQNurikabeApp        *app_;
  QPixmap               pixmap_;
  int                   dx_, dy_;
  int                   cell_size_;
  QFont                 font_, sfont_;
  int                   char_height_, char_descent_;
  int                   schar_height_, schar_descent_;
  CNurikabe::Cell      *currentCell_;
  bool                  showConnection_;
  bool                  showSolution_;
  bool                  showSolutions_;
  bool                  drawConstraint_;
  int                   solutionNum_;
  int                   showSolutionsDepth_;
  CNurikabe::Solutions  solutions_;
  bool                  escape_;
  Graph                 graph_;
};

class CQNurikabeApp;

class CQNurikabe : public CNurikabe {
 public:
  CQNurikabe(CQNurikabeApp *app);

  void setBusy(bool busy) const override;
  bool checkBreak() override;

  void notifyChanged() override;

  bool isBusy() { return timer_ != -1; }

 private:
  CQNurikabeApp *app_;
  int            timer_;
};

class CQNurikabeApp {
 public:
  CQNurikabeApp();

  CNurikabe *getNurikabe() { return nurikabe_; }

  CQNurikabeCanvas *getCanvas() { return canvas_; }

  void showMessage(const QString &msg);

  void solve();
  void step();
  void reset();

  uint getCellValue(int, int) const { return 0; }

  uint getSolveCellValue(int, int, int) const { return 0; }

  uint getSolvedCellValue(int, int) const { return 0; }

  uint getSolutionCellValue(int, int) const { return 0; }

 private:
  CQNurikabe       *nurikabe_;
  CQNurikabeCanvas *canvas_;
  QStatusBar       *status_bar_;
};
