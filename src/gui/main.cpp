#include "main_window.hpp"

#include "ansi_colors.hpp"
#include "core/variables.hpp"

#include <QApplication>

int main(int argc, char **argv) {
  setColorsEnabled(false);
  globalVariableStore().load();

  QApplication app(argc, argv);
  QApplication::setApplicationName("CLI Calculator GUI");

  MainWindow window;
  window.show();

  return app.exec();
}
