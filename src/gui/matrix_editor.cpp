#include "matrix_editor.hpp"

#include <QFormLayout>
#include <QHeaderView>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <sstream>

MatrixEditor::MatrixEditor(QWidget *parent) : QWidget(parent) {
  rowsSpin_ = new QSpinBox(this);
  colsSpin_ = new QSpinBox(this);
  rowsSpin_->setRange(1, 10);
  colsSpin_->setRange(1, 10);
  rowsSpin_->setValue(2);
  colsSpin_->setValue(2);

  auto *sizeLayout = new QFormLayout();
  sizeLayout->addRow("Rows", rowsSpin_);
  sizeLayout->addRow("Columns", colsSpin_);

  table_ = new QTableWidget(rowsSpin_->value(), colsSpin_->value(), this);
  table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table_->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table_->setAlternatingRowColors(true);

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(sizeLayout);
  layout->addWidget(table_);
  setLayout(layout);

  resizeTable(rowsSpin_->value(), colsSpin_->value());

  connect(rowsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, [this](int value) { resizeTable(value, colsSpin_->value()); });
  connect(colsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, [this](int value) { resizeTable(rowsSpin_->value(), value); });
}

void MatrixEditor::resizeTable(int rows, int columns) {
  table_->setRowCount(rows);
  table_->setColumnCount(columns);
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < columns; ++col) {
      if (!table_->item(row, col)) {
        table_->setItem(row, col, new QTableWidgetItem("0"));
      }
    }
  }
}

std::string MatrixEditor::toCliString() const {
  std::ostringstream output;
  const int rows = table_->rowCount();
  const int cols = table_->columnCount();
  for (int row = 0; row < rows; ++row) {
    if (row > 0) {
      output << "; ";
    }
    for (int col = 0; col < cols; ++col) {
      if (col > 0) {
        output << ' ';
      }
      const auto *item = table_->item(row, col);
      std::string cell = item ? item->text().trimmed().toStdString() : "0";
      if (cell.empty()) {
        cell = "0";
      }
      output << cell;
    }
  }
  return output.str();
}
