// Main window implementation for CLI Calculator GUI
// This file has been refactored for better maintainability:
// - Helper functions and classes are in main_window_helpers.cpp
// - UI setup (constructor) is in main_window_ui_setup.cpp
// - This file contains only the member function implementations

#include "main_window.hpp"

#include "app/cli_commands.hpp"
#include "cli_output.hpp"
#include "core/unit_conversion.hpp"
#include "gui/matrix_editor.hpp"

#include <QComboBox>
#include <QCheckBox>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QFrame>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QPixmap>
#include <QPalette>
#include <QPainter>
#include <QSplitter>
#include <QUrl>
#include <QTextBrowser>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QSignalBlocker>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTextDocument>
#include <QKeyEvent>
#include <QEvent>
#include <QTextEdit>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace {
bool g_isDarkTheme = false;

struct PythonRunResult {
  bool started = false;
  int exitCode = -1;
  QString stdoutText;
  QString stderrText;
  QString errorMessage;
};

struct CliRunResult {
  int exitCode = 0;
  std::string out;
  std::string err;
};

class StreamRedirect {
public:
  StreamRedirect(std::ostream &stream, std::streambuf *newBuffer)
      : stream_(stream), oldBuffer_(stream.rdbuf(newBuffer)) {}

  ~StreamRedirect() { stream_.rdbuf(oldBuffer_); }

private:
  std::ostream &stream_;
  std::streambuf *oldBuffer_;
};

// Helper functions and utility classes are included from separate file
#include "main_window_helpers.cpp"
} // namespace

// MainWindow constructor with full UI setup is in separate file for clarity
#include "main_window_ui_setup.cpp"

// MainWindow member function implementations below


void MainWindow::showResult(const QString &title, const QString &text, int exitCode) {
  QString message = text.isEmpty() ? QStringLiteral("No output") : text;
  output_->setPlainText(message);
  statusBar()->showMessage(QString("%1 (exit %2)").arg(title).arg(exitCode), 5000);
}

void MainWindow::runAndShow(const QString &title,
                            const std::function<int(OutputFormat)> &action) {
  lastAction_ = action;
  lastTitle_ = title;
  CliRunResult result = runCliAction([&]() { return action(OutputFormat::Text); });
  QString output = QString::fromStdString(result.out + result.err);
  showResult(title, output.trimmed(), result.exitCode);
}

void MainWindow::copyStructured(OutputFormat format, const QString &label) {
  if (!lastAction_) {
    statusBar()->showMessage("No recent output to copy", 3000);
    return;
  }
  CliRunResult result = runCliAction([&]() { return lastAction_(format); });
  const std::string combined = result.out + result.err;
  QApplication::clipboard()->setText(QString::fromStdString(combined));
  statusBar()->showMessage(QString("%1 (%2)").arg(label).arg(lastTitle_), 4000);
}

void MainWindow::showGraphPreview(const QString &title, const QString &path) {
  lastAction_ = nullptr;
  lastTitle_.clear();
  currentGraphPath_ = path;
  QPixmap pixmap(path);
  if (pixmap.isNull()) {
    showGraphError(title, "Failed to load generated graph.");
    return;
  }
  graphPreviewLabel_->setPixmap(pixmap);
  output_->setPlainText(QString("Saved graph to: %1").arg(path));
  statusBar()->showMessage(QString("%1 generated").arg(title), 4000);
}

void MainWindow::showGraphError(const QString &title, const QString &message) {
  lastAction_ = nullptr;
  lastTitle_.clear();
  output_->setPlainText(message.trimmed().isEmpty()
                            ? QString("Failed to generate graph.")
                            : message.trimmed());
  statusBar()->showMessage(QString("%1 failed").arg(title), 4000);
}

void MainWindow::clearGraphPreview() {
  currentGraphPath_.clear();
  graphPreviewLabel_->setText("No graph generated yet.");
  graphPreviewLabel_->setPixmap(QPixmap());
}

QString MainWindow::makeTempGraphPath() const {
  const QString letters = "abcdefghijklmnopqrstuvwxyz0123456789";
  QString random;
  for (int idx = 0; idx < 10; ++idx) {
    int pos = QRandomGenerator::global()->bounded(letters.size());
    random.append(letters[pos]);
  }
  return QDir::tempPath() + "/tmp_" + random + ".png";
}

void MainWindow::updateHelpTooltip(int tabIndex) {
  if (!helpButton_ || !mainTabs_) {
    return;
  }
  QString tabName;
  if (mainContentStack_ && mainContentStack_->currentWidget() == splitViewContainer_) {
    if (splitLeftSelect_) {
      tabName = splitLeftSelect_->currentText();
    }
  } else {
    tabName = mainTabs_->tabText(tabIndex);
  }
  QString message;
  if (tabName == "Expressions") {
    message = "Evaluate expressions and use BigInt for large integers.";
  } else if (tabName == "Numbers") {
    message = "Square roots, divisor lists, and prime factorization.";
  } else if (tabName == "Conversions") {
    message = "Convert between bases or measurement units.";
  } else if (tabName == "Equations") {
    message = "Solve linear and quadratic equations.";
  } else if (tabName == "Matrices") {
    message = "Add, subtract, or multiply matrices (editor optional).";
  } else if (tabName == "Graphs") {
    message = "Create graphs from values or CSV files and save PNGs.";
  } else if (tabName == "Statistics") {
    message = "Compute summary statistics for numeric lists.";
  } else if (tabName == "Variables") {
    message = "List, set, or unset persisted variables.";
  } else if (tabName == "Notes") {
    message = "Write Markdown notes, preview them, and save to a file.";
  } else if (tabName == "Terminal") {
    message = "Run CLI calculator commands or shell commands in-app.";
  } else {
    message = "Use the tabs to access calculator features.";
  }
  helpButton_->setToolTip(message);
}

QString MainWindow::buildNotesHtml(const QString &markdown) {
  std::vector<QString> previousOutputs;
  previousOutputs.reserve(notesCodeBlocks_.size());
  for (const auto &block : notesCodeBlocks_) {
    previousOutputs.push_back(block.output);
  }
  notesCodeBlocks_.clear();
  const QString text = renderMarkdownWithLatexSymbols(markdown);
  const QRegularExpression fenceRe(
      "```\\s*([\\w#+-]*)[^\\n]*\\n([\\s\\S]*?)```");
  QString placeholderText;
  int lastIndex = 0;
  int index = 0;
  QRegularExpressionMatchIterator it = fenceRe.globalMatch(text);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    placeholderText += text.mid(lastIndex, match.capturedStart() - lastIndex);
    const QString language = match.captured(1);
    const QString code = match.captured(2);
    QString output;
    if (index < static_cast<int>(previousOutputs.size())) {
      output = previousOutputs[index];
    }
    notesCodeBlocks_.push_back({code, language, output});
    placeholderText += QString("[[[CODEBLOCK_%1]]]").arg(index++);
    lastIndex = match.capturedEnd();
  }
  placeholderText += text.mid(lastIndex);

  QTextDocument doc;
  doc.setMarkdown(placeholderText);
  QString html = doc.toHtml();
  for (int i = 0; i < static_cast<int>(notesCodeBlocks_.size()); ++i) {
    const QString placeholder = QString("[[[CODEBLOCK_%1]]]").arg(i);
    const QString escaped = QRegularExpression::escape(placeholder);
    const QString pattern =
        QString("<p[^>]*>(?:<span[^>]*>)?%1(?:</span>)?</p>").arg(escaped);
    QRegularExpression replaceRe(pattern);
    html.replace(replaceRe,
                 wrapCodeBlockHtml(i, notesCodeBlocks_[i].code,
                                   notesCodeBlocks_[i].language,
                                   notesCodeBlocks_[i].output));
    html.replace(placeholder,
                 wrapCodeBlockHtml(i, notesCodeBlocks_[i].code,
                                   notesCodeBlocks_[i].language,
                                   notesCodeBlocks_[i].output));
  }
  return html;
}

void MainWindow::updateNotesPreview() {
  if (!notesPreview_) {
    return;
  }
  if (isPythonNotesFile(notesPath_)) {
    const QString output = notesRunOutput_.toHtmlEscaped();
    QString html = "<pre style=\"white-space:pre-wrap;\">";
    html += output;
    html += "</pre>";
    notesPreview_->setHtml(html);
    return;
  }
  QString html = buildNotesHtml(notesEditor_->toPlainText());
  if (!notesRunOutput_.isEmpty()) {
    const QString output = notesRunOutput_.toHtmlEscaped();
    html += "<hr/>";
    html += "<h3>Output</h3>";
    html += "<pre style=\"white-space:pre-wrap;\">";
    html += output;
    html += "</pre>";
  }
  notesPreview_->setHtml(html);
}

bool MainWindow::isNotesPythonMode() const {
  return isPythonNotesFile(notesPath_);
}

bool MainWindow::isNotesCodePosition(int pos) const {
  if (pos < 0) {
    return false;
  }
  if (isNotesPythonMode()) {
    return true;
  }
  for (const auto &range : notesCodeRanges_) {
    if (pos >= range.first && pos < range.second) {
      return true;
    }
  }
  return false;
}

bool MainWindow::isNotesPythonPosition(int pos) const {
  if (pos < 0) {
    return false;
  }
  if (isNotesPythonMode()) {
    return true;
  }
  for (const auto &range : notesPythonRanges_) {
    if (pos >= range.first && pos < range.second) {
      return true;
    }
  }
  return false;
}

void MainWindow::updateNotesRanges() {
  if (!notesEditor_) {
    notesCodeRanges_.clear();
    notesPythonRanges_.clear();
    return;
  }
  const QString text = notesEditor_->toPlainText();
  if (isNotesPythonMode()) {
    notesCodeRanges_.assign(1, {0, text.size()});
    notesPythonRanges_.assign(1, {0, text.size()});
    return;
  }
  notesCodeRanges_ = extractCodeRanges(text);
  notesPythonRanges_ = extractPythonRanges(text);
}

void MainWindow::setEvalMode(EvalMode mode) {
  evalMode_ = mode;
  if (evalStandardAction_) {
    evalStandardAction_->setChecked(mode == EvalMode::Standard);
  }
  if (evalBigIntAction_) {
    evalBigIntAction_->setChecked(mode == EvalMode::BigInt);
  }
  if (evalBigDoubleAction_) {
    evalBigDoubleAction_->setChecked(mode == EvalMode::BigDouble);
  }
  if (bigIntCheck_) {
    QSignalBlocker blocker(bigIntCheck_);
    bigIntCheck_->setChecked(mode == EvalMode::BigInt);
  }
  if (statusBar()) {
    QString label = "Standard (double)";
    if (mode == EvalMode::BigInt) {
      label = "BigInt (integers only)";
    } else if (mode == EvalMode::BigDouble) {
      label = "BigDouble (high-precision decimals)";
    }
    statusBar()->showMessage("Expression mode: " + label, 3000);
  }
}

void MainWindow::registerTab(const QString &title, QWidget *widget) {
  mainTabs_->addTab(widget, title);
  tabEntries_.push_back({title, widget});
}

void MainWindow::setSplitViewEnabled(bool enabled) {
  if (!mainContentStack_) {
    return;
  }
  splitBarContainer_->setVisible(enabled);
  if (enabled) {
    int currentIndex = mainTabs_->currentIndex();
    if (currentIndex < 0) {
      currentIndex = 0;
    }
    while (mainTabs_->count() > 0) {
      QWidget *widget = mainTabs_->widget(0);
      mainTabs_->removeTab(0);
      moveTabWidget(widget, tabPool_);
    }
    if (splitLeftSelect_->count() > 0) {
      QSignalBlocker blockLeft(splitLeftSelect_);
      splitLeftSelect_->setCurrentIndex(currentIndex);
    }
    if (splitRightSelect_->count() > 0) {
      int rightIndex = (currentIndex + 1) % splitRightSelect_->count();
      QSignalBlocker blockRight(splitRightSelect_);
      splitRightSelect_->setCurrentIndex(rightIndex);
    }
    mainContentStack_->setCurrentWidget(splitViewContainer_);
    updateSplitStacks();
    updateHelpTooltip(currentIndex);
  } else {
    mainContentStack_->setCurrentWidget(mainTabs_);
    while (mainTabs_->count() > 0) {
      mainTabs_->removeTab(0);
    }
    for (const auto &entry : tabEntries_) {
      if (!entry.widget) {
        continue;
      }
      if (auto stack = qobject_cast<QStackedWidget *>(entry.widget->parentWidget())) {
        stack->removeWidget(entry.widget);
      }
      entry.widget->setParent(mainTabs_);
      mainTabs_->addTab(entry.widget, entry.title);
    }
    updateHelpTooltip(mainTabs_->currentIndex());
  }
}

void MainWindow::updateSplitStacks() {
  if (!mainContentStack_ ||
      mainContentStack_->currentWidget() != splitViewContainer_) {
    return;
  }
  const int count = splitLeftSelect_->count();
  if (count == 0) {
    return;
  }
  int leftIndex = splitLeftSelect_->currentIndex();
  int rightIndex = splitRightSelect_->currentIndex();
  if (leftIndex == rightIndex && count > 1) {
    QSignalBlocker blockRight(splitRightSelect_);
    rightIndex = (rightIndex + 1) % count;
    splitRightSelect_->setCurrentIndex(rightIndex);
  }
  if (leftIndex >= 0 && leftIndex < static_cast<int>(tabEntries_.size())) {
    moveTabWidget(tabEntries_[leftIndex].widget, splitLeftStack_);
  }
  if (rightIndex >= 0 && rightIndex < static_cast<int>(tabEntries_.size())) {
    moveTabWidget(tabEntries_[rightIndex].widget, splitRightStack_);
  }
}

void MainWindow::moveTabWidget(QWidget *widget, QStackedWidget *target) {
  if (!widget || !target) {
    return;
  }
  if (auto stack = qobject_cast<QStackedWidget *>(widget->parentWidget())) {
    stack->removeWidget(widget);
  }
  widget->setParent(target);
  if (target->indexOf(widget) == -1) {
    target->addWidget(widget);
  }
  target->setCurrentWidget(widget);
}
