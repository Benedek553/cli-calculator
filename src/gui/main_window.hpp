#pragma once

#include "cli_output.hpp"

#include <QMainWindow>

#include <functional>
#include <utility>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QStackedWidget;
class QTabWidget;
class QToolButton;
class QTextBrowser;
class QPushButton;
class QSyntaxHighlighter;
class MatrixEditor;
class QWidget;
class QSplitter;
class QAction;

struct TabEntry {
  QString title;
  QWidget *widget = nullptr;
};

struct NotesCodeBlock {
  QString code;
  QString language;
  QString output;
};

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  enum class EvalMode { Standard, BigInt, BigDouble };

  QPlainTextEdit *output_ = nullptr;

  QLineEdit *exprInput_ = nullptr;
  QCheckBox *bigIntCheck_ = nullptr;
  QPlainTextEdit *exprHistory_ = nullptr;

  QLineEdit *sqrtInput_ = nullptr;
  QLineEdit *divisorsInput_ = nullptr;
  QLineEdit *primeFactorsInput_ = nullptr;

  QComboBox *baseFromInput_ = nullptr;
  QComboBox *baseToInput_ = nullptr;
  QLineEdit *baseValueInput_ = nullptr;

  QComboBox *unitCategoryInput_ = nullptr;
  QComboBox *unitFromInput_ = nullptr;
  QComboBox *unitToInput_ = nullptr;
  QLineEdit *unitValueInput_ = nullptr;

  QLineEdit *linearAInput_ = nullptr;
  QLineEdit *linearBInput_ = nullptr;
  QLineEdit *quadraticAInput_ = nullptr;
  QLineEdit *quadraticBInput_ = nullptr;
  QLineEdit *quadraticCInput_ = nullptr;

  QLineEdit *matrixAInput_ = nullptr;
  QLineEdit *matrixBInput_ = nullptr;
  MatrixEditor *matrixAEditor_ = nullptr;
  MatrixEditor *matrixBEditor_ = nullptr;
  QStackedWidget *matrixInputStack_ = nullptr;

  QLineEdit *statsInput_ = nullptr;

  QLineEdit *varNameInput_ = nullptr;
  QLineEdit *varValueInput_ = nullptr;
  QLineEdit *unsetVarInput_ = nullptr;

  QLineEdit *graphValuesInput_ = nullptr;
  QSpinBox *graphValuesHeightSpin_ = nullptr;
  QLineEdit *graphCsvPathInput_ = nullptr;
  QLineEdit *graphCsvColumnInput_ = nullptr;
  QSpinBox *graphCsvHeightSpin_ = nullptr;
  QCheckBox *graphCsvHeadersCheck_ = nullptr;
  QLabel *graphPreviewLabel_ = nullptr;
  QString currentGraphPath_;
  QTabWidget *mainTabs_ = nullptr;
  QToolButton *helpButton_ = nullptr;
  QPlainTextEdit *notesEditor_ = nullptr;
  QTextBrowser *notesPreview_ = nullptr;
  QSplitter *notesSplitter_ = nullptr;
  QString notesPath_;
  QAction *notesNewAction_ = nullptr;
  QAction *notesOpenAction_ = nullptr;
  QAction *notesSaveAction_ = nullptr;
  QAction *notesSaveAsAction_ = nullptr;
  QAction *notesCopyAction_ = nullptr;
  QAction *notesTogglePreviewAction_ = nullptr;
  QAction *evalStandardAction_ = nullptr;
  QAction *evalBigIntAction_ = nullptr;
  QAction *evalBigDoubleAction_ = nullptr;
  QPushButton *notesRunButton_ = nullptr;
  QPushButton *notesSwitchButton_ = nullptr;
  QString notesRunOutput_;
  QSyntaxHighlighter *notesHighlighter_ = nullptr;
  std::vector<NotesCodeBlock> notesCodeBlocks_;
  std::vector<std::pair<int, int>> notesCodeRanges_;
  std::vector<std::pair<int, int>> notesPythonRanges_;
  bool notesRefreshInProgress_ = false;
  QPlainTextEdit *terminalOutput_ = nullptr;
  QLineEdit *terminalInput_ = nullptr;
  QString terminalCwd_;
  QWidget *splitBarContainer_ = nullptr;
  QStackedWidget *mainContentStack_ = nullptr;
  QWidget *splitViewContainer_ = nullptr;
  QStackedWidget *splitLeftStack_ = nullptr;
  QStackedWidget *splitRightStack_ = nullptr;
  QStackedWidget *tabPool_ = nullptr;
  QComboBox *splitLeftSelect_ = nullptr;
  QComboBox *splitRightSelect_ = nullptr;
  std::vector<TabEntry> tabEntries_;
  EvalMode evalMode_ = EvalMode::Standard;

  void showResult(const QString &title, const QString &text, int exitCode);
  void runAndShow(const QString &title,
                  const std::function<int(OutputFormat)> &action);
  void copyStructured(OutputFormat format, const QString &label);
  void showGraphPreview(const QString &title, const QString &path);
  void showGraphError(const QString &title, const QString &message);
  void clearGraphPreview();
  QString makeTempGraphPath() const;
  void updateHelpTooltip(int tabIndex);
  void registerTab(const QString &title, QWidget *widget);
  void setSplitViewEnabled(bool enabled);
  void updateSplitStacks();
  void moveTabWidget(QWidget *widget, QStackedWidget *target);
  QString buildNotesHtml(const QString &markdown);
  void updateNotesPreview();
  bool isNotesPythonMode() const;
  bool isNotesCodePosition(int pos) const;
  bool isNotesPythonPosition(int pos) const;
  void updateNotesRanges();
  void setEvalMode(EvalMode mode);

  std::function<int(OutputFormat)> lastAction_;
  QString lastTitle_;
};
