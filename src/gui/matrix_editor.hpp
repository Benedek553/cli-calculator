#pragma once

#include <QWidget>

#include <string>

class QSpinBox;
class QTableWidget;

class MatrixEditor : public QWidget {
public:
  explicit MatrixEditor(QWidget *parent = nullptr);

  std::string toCliString() const;

private:
  void resizeTable(int rows, int columns);

  QSpinBox *rowsSpin_ = nullptr;
  QSpinBox *colsSpin_ = nullptr;
  QTableWidget *table_ = nullptr;
};
