CliRunResult runCliAction(const std::function<int()> &action) {
  std::ostringstream outBuffer;
  std::ostringstream errBuffer;
  StreamRedirect outRedirect(std::cout, outBuffer.rdbuf());
  StreamRedirect errRedirect(std::cerr, errBuffer.rdbuf());

  CliRunResult result;
  result.exitCode = action();
  result.out = outBuffer.str();
  result.err = errBuffer.str();
  return result;
}

std::vector<std::string> splitTokens(const QString &text) {
  std::istringstream stream(text.toStdString());
  std::vector<std::string> tokens;
  std::string token;
  while (stream >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

QGroupBox *wrapSection(const QString &title, QLayout *layout) {
  auto *group = new QGroupBox(title);
  group->setProperty("sectionCard", true);
  group->setLayout(layout);
  return group;
}

std::string toLower(std::string value) {
  for (char &ch : value) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return value;
}

QString renderMarkdownWithLatexSymbols(const QString &source) {
  QString text = source;
  static const std::vector<std::pair<QString, QString>> replacements = {
      {"\\\\alpha", "α"}, {"\\\\beta", "β"},   {"\\\\gamma", "γ"},
      {"\\\\delta", "δ"}, {"\\\\epsilon", "ε"}, {"\\\\theta", "θ"},
      {"\\\\lambda", "λ"}, {"\\\\mu", "μ"},    {"\\\\pi", "π"},
      {"\\\\sigma", "σ"}, {"\\\\phi", "φ"},    {"\\\\omega", "ω"},
      {"\\\\Delta", "Δ"}, {"\\\\Sigma", "Σ"},  {"\\\\Pi", "Π"},
      {"\\\\Omega", "Ω"}, {"\\\\times", "×"},  {"\\\\cdot", "·"},
      {"\\\\le", "≤"},    {"\\\\ge", "≥"},     {"\\\\neq", "≠"},
      {"\\\\approx", "≈"}, {"\\\\pm", "±"},    {"\\\\infty", "∞"},
      {"\\\\rightarrow", "→"}, {"\\\\leftarrow", "←"},
      {"\\\\leftrightarrow", "↔"}};

  for (const auto &item : replacements) {
    text.replace(item.first, item.second);
  }
  text.replace("$$", "");
  text.replace("$", "");
  return text;
}

QString escapeWithWhitespace(const QString &text) {
  QString out;
  out.reserve(text.size() * 2);
  for (const QChar ch : text) {
    if (ch == '&') {
      out += "&amp;";
    } else if (ch == '<') {
      out += "&lt;";
    } else if (ch == '>') {
      out += "&gt;";
    } else if (ch == '"') {
      out += "&quot;";
    } else if (ch == '\'') {
      out += "&#39;";
    } else if (ch == '\t') {
      out += "&nbsp;&nbsp;&nbsp;&nbsp;";
    } else if (ch == '\n') {
      out += "<br/>";
    } else if (ch == ' ') {
      out += "&nbsp;";
    } else {
      out += ch;
    }
  }
  return out;
}

QString wrapCodeBlockHtml(int index, const QString &code, const QString &language,
                          const QString &output) {
  const bool useDark = g_isDarkTheme;
  const QString label =
      language.trimmed().isEmpty() ? QStringLiteral("code") : language.trimmed();
  const QString lang = label.toLower();
  const bool isPython = (lang == "py" || lang == "python");
  const QString background = useDark ? "#0b0f14" : "#f7f7f9";
  const QString border = useDark ? "#1f2a3a" : "#c9c9c9";
  const QString header = useDark ? "#101622" : "#f0f1f5";
  const QString text = useDark ? "#e6edf7" : "#1b1f24";
  const QString subtle = useDark ? "#93a4bb" : "#667085";
  const QString link = useDark ? "#6ab0ff" : "#2c7be5";

  struct HighlightRule {
    QRegularExpression regex;
    QString darkStyle;
    QString lightStyle;
  };

  auto applyHighlighting = [&](const QString &source,
                               const std::vector<HighlightRule> &rules) {
    struct Token {
      int start = 0;
      int end = 0;
      QString style;
    };
    std::vector<Token> tokens;
    for (const auto &rule : rules) {
      QRegularExpressionMatchIterator it = rule.regex.globalMatch(source);
      while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const int start = match.capturedStart();
        const int end = match.capturedEnd();
        if (start < 0 || end <= start) {
          continue;
        }
        bool overlaps = false;
        for (const auto &token : tokens) {
          if (start < token.end && end > token.start) {
            overlaps = true;
            break;
          }
        }
        if (overlaps) {
          continue;
        }
        tokens.push_back(
            {start, end, useDark ? rule.darkStyle : rule.lightStyle});
      }
    }
    std::sort(tokens.begin(), tokens.end(),
              [](const Token &a, const Token &b) { return a.start < b.start; });
    QString result;
    int pos = 0;
    for (const auto &token : tokens) {
      if (token.start > pos) {
        result += escapeWithWhitespace(source.mid(pos, token.start - pos));
      }
      const QString segment = source.mid(token.start, token.end - token.start);
      result += QString("<span style=\"%1\">%2</span>")
                    .arg(token.style, escapeWithWhitespace(segment));
      pos = token.end;
    }
    if (pos < source.size()) {
      result += escapeWithWhitespace(source.mid(pos));
    }
    return result;
  };

  std::vector<HighlightRule> rules;
  const QString commentDark = "color:#565f89;";
  const QString commentLight = "color:#6e7781;";
  const QString stringDark = "color:#9ece6a;";
  const QString stringLight = "color:#0a7a3d;";
  const QString numberDark = "color:#ff9e64;";
  const QString numberLight = "color:#b36200;";
  const QString keywordDark = "color:#7aa2f7; font-weight:600;";
  const QString keywordLight = "color:#1f6feb; font-weight:600;";
  const QString typeDark = "color:#7dcfff;";
  const QString typeLight = "color:#0550ae;";
  const QString builtinDark = "color:#c0caf5;";
  const QString builtinLight = "color:#24292f;";

  auto addRule = [&](const QString &pattern, QRegularExpression::PatternOptions options,
                     const QString &dark, const QString &light) {
    rules.push_back({QRegularExpression(pattern, options), dark, light});
  };

  if (lang.contains("python") || lang == "py") {
    addRule("'''[\\s\\S]*?'''", QRegularExpression::DotMatchesEverythingOption,
            stringDark, stringLight);
    addRule("\"\"\"[\\s\\S]*?\"\"\"", QRegularExpression::DotMatchesEverythingOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)'([^'\\\\]|\\\\.)*'", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("#[^\\n]*", QRegularExpression::NoPatternOption, commentDark,
            commentLight);
    addRule("\\b\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
    addRule("\\b(def|class|return|if|elif|else|for|while|break|continue|import|from|"
            "as|pass|raise|try|except|with|lambda|True|False|None)\\b",
            QRegularExpression::NoPatternOption, keywordDark, keywordLight);
    addRule("\\b(print|len|range|dict|list|set|tuple|str|int|float|bool)\\b",
            QRegularExpression::NoPatternOption, builtinDark, builtinLight);
  } else if (lang.contains("json")) {
    addRule("\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("\\b(true|false|null)\\b", QRegularExpression::NoPatternOption,
            keywordDark, keywordLight);
    addRule("\\b-?\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
  } else if (lang.contains("yaml") || lang.contains("yml")) {
    addRule("^\\s*[^:\\n]+(?=\\s*:)", QRegularExpression::MultilineOption,
            typeDark, typeLight);
    addRule("\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("'([^'\\\\]|\\\\.)*'", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("#[^\\n]*", QRegularExpression::NoPatternOption, commentDark,
            commentLight);
    addRule("\\b(true|false|null)\\b", QRegularExpression::NoPatternOption,
            keywordDark, keywordLight);
    addRule("\\b-?\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
  } else if (lang.contains("bash") || lang.contains("sh")) {
    addRule("(?<!\\\\)\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)'([^'\\\\]|\\\\.)*'", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("#[^\\n]*", QRegularExpression::NoPatternOption, commentDark,
            commentLight);
    addRule("\\b(echo|export|cd|ls|cat|grep|sed|awk|curl|sudo|if|then|fi|for|do|done)\\b",
            QRegularExpression::NoPatternOption, keywordDark, keywordLight);
    addRule("\\b-?\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
  } else if (lang.contains("js") || lang.contains("ts") || lang.contains("javascript") ||
             lang.contains("typescript")) {
    addRule("`([^`\\\\]|\\\\.)*`", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)'([^'\\\\]|\\\\.)*'", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("/\\*[\\s\\S]*?\\*/", QRegularExpression::DotMatchesEverythingOption,
            commentDark, commentLight);
    addRule("//[^\\n]*", QRegularExpression::NoPatternOption, commentDark,
            commentLight);
    addRule("\\b(const|let|var|function|return|if|else|for|while|break|continue|"
            "class|new|try|catch|throw|import|from|export|default|async|await|"
            "true|false|null|undefined)\\b",
            QRegularExpression::NoPatternOption, keywordDark, keywordLight);
    addRule("\\b-?\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
  } else if (lang.contains("cpp") || lang.contains("c++") || lang == "c" ||
             lang == "h" || lang.contains("hpp") || lang.contains("hxx") ||
             lang.contains("cc") || lang.contains("cxx")) {
    addRule("(?<!\\\\)\"([^\"\\\\]|\\\\.)*\"", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("(?<!\\\\)'([^'\\\\]|\\\\.)*'", QRegularExpression::NoPatternOption,
            stringDark, stringLight);
    addRule("/\\*[\\s\\S]*?\\*/", QRegularExpression::DotMatchesEverythingOption,
            commentDark, commentLight);
    addRule("//[^\\n]*", QRegularExpression::NoPatternOption, commentDark,
            commentLight);
    addRule("\\b(int|float|double|auto|void|const|constexpr|struct|class|public|"
            "private|protected|virtual|override|template|typename|using|namespace|"
            "static|inline|return|if|else|switch|case|for|while|do|break|continue|"
            "new|delete|try|catch|throw|include)\\b",
            QRegularExpression::NoPatternOption, keywordDark, keywordLight);
    addRule("\\b(bool|char|short|long|size_t|string|vector|map|unordered_map)\\b",
            QRegularExpression::NoPatternOption, typeDark, typeLight);
    addRule("\\b-?\\d+(?:\\.\\d+)?\\b", QRegularExpression::NoPatternOption,
            numberDark, numberLight);
  }

  const QString rendered = rules.empty() ? escapeWithWhitespace(code)
                                         : applyHighlighting(code, rules);
  const QString outputHtml = output.trimmed().isEmpty()
                                 ? QString()
                                 : QString(
                                       "<div style=\"border-top:1px solid %1; "
                                       "background:%2;\">"
                                       "<div style=\"padding:6px 10px; "
                                       "font-size:12px; color:%3;\">OUTPUT</div>"
                                       "<pre style=\"margin:0; padding:10px; "
                                       "font-family:'JetBrains Mono','Fira Code',"
                                       "'Source Code Pro','IBM Plex Mono','Menlo',"
                                       "'Consolas',monospace; font-size:12px; "
                                       "line-height:1.5;\">%4</pre>"
                                       "</div>")
                                       .arg(border, background, subtle,
                                            escapeWithWhitespace(output));
  const QString runLink =
      isPython ? QString("<a style=\"color:%1; text-decoration:none; margin-right:18px;\" "
                         "href=\"run://%2\">Run Cell</a>")
                      .arg(link)
                      .arg(index)
               : QString();
  return QString(
             "<div style=\"border:1px solid %1; border-radius:8px; "
             "margin:10px 0; background:%2; color:%3;\">"
             "<div style=\"padding:6px 10px; background:%4; "
             "border-bottom:1px solid %1; border-top-left-radius:8px; "
             "border-top-right-radius:8px; font-size:12px; color:%5;\">"
             "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">"
             "<tr><td style=\"letter-spacing:0.6px; text-transform:uppercase;\">%6</td>"
             "<td align=\"right\">"
             "%7"
             "<a style=\"color:%8; text-decoration:none;\" href=\"copy://%9\">Copy</a>"
             "</td></tr></table>"
             "</div>"
             "<pre style=\"margin:0; padding:10px; "
             "font-family:'JetBrains Mono','Fira Code','Source Code Pro',"
             "'IBM Plex Mono','Menlo','Consolas',monospace; font-size:12px; "
             "line-height:1.5;\">%10</pre>"
             "%11"
             "</div>")
      .arg(border)
      .arg(background)
      .arg(text)
      .arg(header)
      .arg(subtle)
      .arg(label.toHtmlEscaped())
      .arg(runLink)
      .arg(link)
      .arg(index)
      .arg(rendered)
      .arg(outputHtml);
}

std::vector<QString> extractPythonBlocks(const QString &markdown) {
  std::vector<QString> blocks;
  const QRegularExpression fenceRe(
      "```\\s*(python|py)\\b[^\\n]*\\n([\\s\\S]*?)```",
      QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator it = fenceRe.globalMatch(markdown);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    const QString code = match.captured(2);
    if (!code.trimmed().isEmpty()) {
      blocks.push_back(code);
    }
  }
  return blocks;
}

bool isPythonNotesFile(const QString &path) {
  return path.endsWith(".py", Qt::CaseInsensitive);
}

std::vector<std::pair<int, int>> extractCodeRanges(const QString &markdown) {
  std::vector<std::pair<int, int>> ranges;
  const QRegularExpression fenceRe("```[^\\n]*\\n([\\s\\S]*?)```");
  QRegularExpressionMatchIterator it = fenceRe.globalMatch(markdown);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    const int start = match.capturedStart(1);
    const int end = match.capturedEnd(1);
    if (start >= 0 && end >= start) {
      ranges.emplace_back(start, end);
    }
  }
  return ranges;
}

std::vector<std::pair<int, int>> extractPythonRanges(const QString &markdown) {
  std::vector<std::pair<int, int>> ranges;
  const QRegularExpression fenceRe(
      "```\\s*(python|py)\\b[^\\n]*\\n([\\s\\S]*?)```",
      QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator it = fenceRe.globalMatch(markdown);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    const int start = match.capturedStart(2);
    const int end = match.capturedEnd(2);
    if (start >= 0 && end >= start) {
      ranges.emplace_back(start, end);
    }
  }
  return ranges;
}

bool parseCodeBlockIndex(const QUrl &url, int *index) {
  if (!index) {
    return false;
  }
  const QString display = url.toDisplayString(QUrl::FullyDecoded);
  const QRegularExpression anyDigits("(\\d+)");
  const QRegularExpressionMatch anyMatch = anyDigits.match(display);
  if (anyMatch.hasMatch()) {
    bool ok = false;
    const int value = anyMatch.captured(1).toInt(&ok);
    if (ok) {
      *index = value;
      return true;
    }
  }
  QString raw = url.path();
  raw = raw.trimmed();
  while (raw.startsWith('/')) {
    raw = raw.mid(1);
  }
  while (raw.endsWith('/')) {
    raw.chop(1);
  }
  if (raw.isEmpty()) {
    raw = url.host().trimmed();
  }
  if (raw.isEmpty()) {
    const QString full = url.toString(QUrl::FullyDecoded);
    const QRegularExpression re("^(?:copy|run):/*(\\d+)");
    const QRegularExpressionMatch match = re.match(full);
    if (match.hasMatch()) {
      raw = match.captured(1);
    } else {
      const QRegularExpression tailRe("(\\d+)$");
      const QRegularExpressionMatch tailMatch = tailRe.match(full);
      if (tailMatch.hasMatch()) {
        raw = tailMatch.captured(1);
      }
    }
  }
  bool ok = false;
  const int value = raw.toInt(&ok);
  if (!ok) {
    return false;
  }
  *index = value;
  return true;
}

PythonRunResult runPythonSnippet(const QString &code) {
  PythonRunResult result;
  if (code.trimmed().isEmpty()) {
    result.errorMessage = "No code to run.";
    return result;
  }

  auto runPython = [&code](const QString &program, PythonRunResult *out) -> bool {
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString snapRoot = QString::fromUtf8(qgetenv("SNAP"));
    if (!snapRoot.isEmpty()) {
      env.insert("PYTHONHOME", snapRoot + "/usr");
      env.insert("PYTHONPATH", snapRoot + "/usr/lib/python3/dist-packages");
    }
    process.setProcessEnvironment(env);
    process.start(program, QStringList() << "-");
    if (!process.waitForStarted(2000)) {
      return false;
    }
    process.write(code.toUtf8());
    process.closeWriteChannel();
    if (!process.waitForFinished(-1)) {
      return false;
    }
    out->stdoutText = QString::fromUtf8(process.readAllStandardOutput());
    out->stderrText = QString::fromUtf8(process.readAllStandardError());
    out->exitCode = process.exitCode();
    return true;
  };

  const QString snapRoot = QString::fromUtf8(qgetenv("SNAP"));
  QStringList candidates;
  if (!snapRoot.isEmpty()) {
    candidates << (snapRoot + "/usr/bin/python3")
               << (snapRoot + "/usr/bin/python");
  }
  candidates << "python3" << "python";
  for (const auto &candidate : candidates) {
    if (runPython(candidate, &result)) {
      result.started = true;
      break;
    }
  }
  if (!result.started) {
    result.errorMessage =
        "Failed to start Python interpreter (python3/python not found).";
  }
  return result;
}

QFont chooseFont(const QStringList &candidates, int pointSize,
                 QFont::Weight weight = QFont::Normal) {
  const QStringList availableFamilies = QFontDatabase().families();
  for (const auto &family : candidates) {
    if (availableFamilies.contains(family)) {
      QFont font(family);
      font.setPointSize(pointSize);
      font.setWeight(weight);
      return font;
    }
  }
  QFont fallback = QApplication::font();
  fallback.setPointSize(pointSize);
  fallback.setWeight(weight);
  return fallback;
}

QFont devMonoFont() {
  const QStringList monoCandidates = {
      QStringLiteral("JetBrains Mono"), QStringLiteral("Fira Code"),
      QStringLiteral("Source Code Pro"), QStringLiteral("IBM Plex Mono"),
      QStringLiteral("Menlo"), QStringLiteral("Consolas")};
  return chooseFont(monoCandidates, 10, QFont::Medium);
}

QBrush makeGridBrush(const QColor &base, const QColor &minor,
                     const QColor &major) {
  const int size = 64;
  QPixmap pixmap(size, size);
  pixmap.fill(base);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setPen(QPen(minor, 1));
  for (int i = 0; i <= size; i += 16) {
    painter.drawLine(i, 0, i, size);
    painter.drawLine(0, i, size, i);
  }
  painter.setPen(QPen(major, 1));
  for (int i = 0; i <= size; i += 32) {
    painter.drawLine(i, 0, i, size);
    painter.drawLine(0, i, size, i);
  }
  return QBrush(pixmap);
}

void applyDevTheme() {
  const QStringList uiCandidates = {
      QStringLiteral("Space Grotesk"), QStringLiteral("Manrope"),
      QStringLiteral("IBM Plex Sans"), QStringLiteral("Noto Sans"),
      QStringLiteral("Segoe UI"), QStringLiteral("Helvetica Neue")};
  qApp->setFont(chooseFont(uiCandidates, 10, QFont::Normal));

  const QColor windowColor = qApp->palette().color(QPalette::Window);
  const bool useDark = windowColor.lightness() < 128;
  g_isDarkTheme = useDark;
  const QColor base = useDark ? QColor("#0f131a") : QColor("#f7f9fd");
  const QColor minor = useDark ? QColor(28, 38, 51, 130) : QColor(221, 227, 236, 160);
  const QColor major = useDark ? QColor(34, 48, 66, 180) : QColor(206, 214, 226, 180);
  QPalette palette = qApp->palette();
  palette.setBrush(QPalette::Window, makeGridBrush(base, minor, major));
  qApp->setPalette(palette);
  const QString style = useDark ? QStringLiteral(R"(
    QMainWindow {
      background: transparent;
    }
    QWidget {
      color: #e2e7f0;
    }
    QFrame#HeroPanel {
      background: rgba(16, 22, 30, 0.92);
      border: 1px solid #223040;
      border-radius: 14px;
    }
    QLabel#HeroBrand {
      color: #e9edf5;
      font-weight: 700;
      letter-spacing: 1px;
    }
    QLabel#HeroKicker {
      color: #9da9ba;
      font-size: 12px;
    }
    QLabel#HeroTitle {
      color: #f7f9ff;
      font-size: 32px;
      font-weight: 700;
    }
    QLabel#HeroSubtitle {
      color: #b7c2d3;
      font-size: 13px;
    }
    QToolButton[navButton="true"] {
      background: transparent;
      color: #9da9ba;
      border: 1px solid transparent;
      padding: 4px 10px;
      border-radius: 6px;
    }
    QToolButton[navButton="true"]:hover {
      color: #f0f4ff;
      border: 1px solid #2c3b4f;
      background: rgba(26, 36, 50, 0.8);
    }
    QPushButton[outlineButton="true"] {
      background: transparent;
      color: #f0f4ff;
      border: 1px solid #2f3f56;
      border-radius: 8px;
      padding: 7px 16px;
      font-weight: 600;
    }
    QPushButton[outlineButton="true"]:hover {
      border: 1px solid #4aa3ff;
      color: #ffffff;
    }
    QTabWidget::pane {
      background: #1a2230;
      border: 1px solid #263142;
      border-radius: 12px;
      margin-top: 8px;
    }
    QTabBar::tab {
      background: #141b24;
      border: 1px solid #263142;
      border-bottom: none;
      padding: 7px 16px;
      margin-right: 2px;
      border-top-left-radius: 10px;
      border-top-right-radius: 10px;
      color: #aeb8c7;
    }
    QTabBar::tab:selected {
      background: #1a2230;
      color: #f3f7ff;
    }
    QTabBar::tab:hover {
      background: #1c2534;
    }
    QGroupBox[sectionCard="true"] {
      background: #1a2230;
      border: 1px solid #263142;
      border-radius: 12px;
      margin-top: 16px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      subcontrol-position: top left;
      left: 12px;
      padding: 0 6px;
      color: #c9d3e2;
      font-weight: 600;
    }
    QLineEdit,
    QComboBox,
    QSpinBox,
    QTextEdit,
    QPlainTextEdit {
      background: #121822;
      border: 1px solid #2b384b;
      border-radius: 8px;
      padding: 6px 10px;
      selection-background-color: #264b7a;
    }
    QLineEdit:focus,
    QComboBox:focus,
    QSpinBox:focus,
    QTextEdit:focus,
    QPlainTextEdit:focus {
      background: #151e2b;
      border: 1px solid #4aa3ff;
    }
    QTableWidget {
      background: #121822;
      border: 1px solid #2b384b;
      border-radius: 8px;
      gridline-color: #2b384b;
    }
    QHeaderView::section {
      background: #1f2937;
      border: 1px solid #2b384b;
      padding: 4px 6px;
      font-weight: 600;
      color: #c9d3e2;
    }
    QComboBox::drop-down {
      border-left: 1px solid #2b384b;
      width: 22px;
    }
    QPushButton {
      background: #2b6de0;
      color: #f3f7ff;
      border: none;
      border-radius: 8px;
      padding: 7px 16px;
      font-weight: 600;
    }
    QPushButton:hover {
      background: #1f57ba;
    }
    QPushButton:pressed {
      background: #164390;
    }
    QToolButton {
      background: #1f2937;
      border: 1px solid #2b384b;
      border-radius: 8px;
      padding: 4px 8px;
      color: #c6d0df;
    }
    QToolButton:hover {
      background: #243243;
    }
    QMenuBar {
      background: #121822;
      border-bottom: 1px solid #263142;
    }
    QMenuBar::item {
      padding: 6px 12px;
      margin: 2px 4px;
      border-radius: 6px;
    }
    QMenuBar::item:selected {
      background: #1f2a3a;
    }
    QMenu {
      background: #121822;
      border: 1px solid #263142;
      padding: 4px;
    }
    QMenu::item {
      padding: 6px 20px;
      border-radius: 6px;
    }
    QMenu::item:selected {
      background: #1f2a3a;
    }
    QCheckBox {
      spacing: 8px;
      color: #c9d3e2;
    }
    QCheckBox::indicator {
      width: 16px;
      height: 16px;
      border-radius: 4px;
      border: 1px solid #2b384b;
      background: #121822;
    }
    QCheckBox::indicator:checked {
      background: #2b6de0;
      border: 1px solid #2b6de0;
    }
    QStatusBar {
      background: #121822;
      border-top: 1px solid #263142;
    }
    QSplitter::handle {
      background: #263142;
    }
    QScrollBar:vertical {
      background: transparent;
      width: 10px;
      margin: 2px;
    }
    QScrollBar::handle:vertical {
      background: #2b384b;
      border-radius: 4px;
      min-height: 30px;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
      height: 0px;
    }
    QScrollBar:horizontal {
      background: transparent;
      height: 10px;
      margin: 2px;
    }
    QScrollBar::handle:horizontal {
      background: #2b384b;
      border-radius: 4px;
      min-width: 30px;
    }
    QScrollBar::add-line:horizontal,
    QScrollBar::sub-line:horizontal {
      width: 0px;
    }
  )")
                          : QStringLiteral(R"(
    QMainWindow {
      background: transparent;
    }
    QWidget {
      color: #1b1f24;
    }
    QFrame#HeroPanel {
      background: rgba(255, 255, 255, 0.95);
      border: 1px solid #d9e1ee;
      border-radius: 14px;
    }
    QLabel#HeroBrand {
      color: #1b1f24;
      font-weight: 700;
      letter-spacing: 1px;
    }
    QLabel#HeroKicker {
      color: #5b6675;
      font-size: 12px;
    }
    QLabel#HeroTitle {
      color: #0c1118;
      font-size: 32px;
      font-weight: 700;
    }
    QLabel#HeroSubtitle {
      color: #5b6675;
      font-size: 13px;
    }
    QToolButton[navButton="true"] {
      background: transparent;
      color: #4b5563;
      border: 1px solid transparent;
      padding: 4px 10px;
      border-radius: 6px;
    }
    QToolButton[navButton="true"]:hover {
      color: #111827;
      border: 1px solid #d9e1ee;
      background: rgba(236, 242, 251, 0.9);
    }
    QPushButton[outlineButton="true"] {
      background: transparent;
      color: #1b1f24;
      border: 1px solid #c9d3e2;
      border-radius: 8px;
      padding: 7px 16px;
      font-weight: 600;
    }
    QPushButton[outlineButton="true"]:hover {
      border: 1px solid #2c7be5;
      color: #0c1118;
    }
    QTabWidget::pane {
      background: #ffffff;
      border: 1px solid #d9e1ee;
      border-radius: 12px;
      margin-top: 8px;
    }
    QTabBar::tab {
      background: #e9eef6;
      border: 1px solid #d9e1ee;
      border-bottom: none;
      padding: 7px 16px;
      margin-right: 2px;
      border-top-left-radius: 10px;
      border-top-right-radius: 10px;
      color: #3b4552;
    }
    QTabBar::tab:selected {
      background: #ffffff;
      color: #1b1f24;
    }
    QTabBar::tab:hover {
      background: #f2f6fd;
    }
    QGroupBox[sectionCard="true"] {
      background: #ffffff;
      border: 1px solid #d9e1ee;
      border-radius: 12px;
      margin-top: 16px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      subcontrol-position: top left;
      left: 12px;
      padding: 0 6px;
      color: #2f3a4a;
      font-weight: 600;
    }
    QLineEdit,
    QComboBox,
    QSpinBox,
    QTextEdit,
    QPlainTextEdit {
      background: #f9fbff;
      border: 1px solid #d3dae6;
      border-radius: 8px;
      padding: 6px 10px;
      selection-background-color: #cfe3ff;
    }
    QLineEdit:focus,
    QComboBox:focus,
    QSpinBox:focus,
    QTextEdit:focus,
    QPlainTextEdit:focus {
      background: #ffffff;
      border: 1px solid #2c7be5;
    }
    QTableWidget {
      background: #ffffff;
      border: 1px solid #d3dae6;
      border-radius: 8px;
      gridline-color: #e1e7f0;
    }
    QHeaderView::section {
      background: #eef2f7;
      border: 1px solid #d3dae6;
      padding: 4px 6px;
      font-weight: 600;
      color: #2f3a4a;
    }
    QComboBox::drop-down {
      border-left: 1px solid #d3dae6;
      width: 22px;
    }
    QPushButton {
      background: #2c7be5;
      color: #ffffff;
      border: none;
      border-radius: 8px;
      padding: 7px 16px;
      font-weight: 600;
    }
    QPushButton:hover {
      background: #2366c4;
    }
    QPushButton:pressed {
      background: #1c529d;
    }
    QToolButton {
      background: #eef2f7;
      border: 1px solid #d3dae6;
      border-radius: 8px;
      padding: 4px 8px;
      color: #394454;
    }
    QToolButton:hover {
      background: #e1e8f3;
    }
    QMenuBar {
      background: #ffffff;
      border-bottom: 1px solid #d9e1ee;
    }
    QMenuBar::item {
      padding: 6px 12px;
      margin: 2px 4px;
      border-radius: 6px;
    }
    QMenuBar::item:selected {
      background: #eaf2ff;
    }
    QMenu {
      background: #ffffff;
      border: 1px solid #d9e1ee;
      padding: 4px;
    }
    QMenu::item {
      padding: 6px 20px;
      border-radius: 6px;
    }
    QMenu::item:selected {
      background: #eaf2ff;
    }
    QCheckBox {
      spacing: 8px;
      color: #2f3a4a;
    }
    QCheckBox::indicator {
      width: 16px;
      height: 16px;
      border-radius: 4px;
      border: 1px solid #c4ccda;
      background: #ffffff;
    }
    QCheckBox::indicator:checked {
      background: #2c7be5;
      border: 1px solid #2c7be5;
    }
    QStatusBar {
      background: #ffffff;
      border-top: 1px solid #d9e1ee;
    }
    QSplitter::handle {
      background: #d9e1ee;
    }
    QScrollBar:vertical {
      background: transparent;
      width: 10px;
      margin: 2px;
    }
    QScrollBar::handle:vertical {
      background: #c9d3e2;
      border-radius: 4px;
      min-height: 30px;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
      height: 0px;
    }
    QScrollBar:horizontal {
      background: transparent;
      height: 10px;
      margin: 2px;
    }
    QScrollBar::handle:horizontal {
      background: #c9d3e2;
      border-radius: 4px;
      min-width: 30px;
    }
    QScrollBar::add-line:horizontal,
    QScrollBar::sub-line:horizontal {
      width: 0px;
    }
  )");

  qApp->setStyleSheet(style);
}

QString makePythonNotesPath() {
  const QString sessionNumber = QString::number(QDateTime::currentSecsSinceEpoch());
  return QDir::currentPath() + "/cli_calc_notes_" + sessionNumber + ".py";
}

class PythonSyntaxHighlighter : public QSyntaxHighlighter {
public:
  explicit PythonSyntaxHighlighter(QTextDocument *parent,
                                   std::function<bool(int)> allowHighlight)
      : QSyntaxHighlighter(parent),
        allowHighlight_(std::move(allowHighlight)) {
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(QColor(0, 70, 140));
    keywordFormat.setFontWeight(QFont::Bold);
    const QStringList keywords = {
        "and",   "as",     "assert", "break", "class", "continue", "def",
        "del",   "elif",   "else",   "except","False", "finally",  "for",
        "from",  "global", "if",     "import","in",    "is",       "lambda",
        "None",  "nonlocal","not",   "or",    "pass",  "raise",    "return",
        "True",  "try",    "while",  "with",  "yield"};
    for (const auto &kw : keywords) {
      rules_.push_back({QRegularExpression(QString("\\b%1\\b").arg(kw)), keywordFormat});
    }

    QTextCharFormat stringFormat;
    stringFormat.setForeground(QColor(150, 80, 20));
    rules_.push_back({QRegularExpression(R"('([^'\\]|\\.)*')"), stringFormat});
    rules_.push_back({QRegularExpression(R"("([^"\\]|\\.)*")"), stringFormat});

    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(120, 0, 120));
    rules_.push_back({QRegularExpression(R"(\b\d+(\.\d+)?\b)"), numberFormat});

    commentFormat_.setForeground(QColor(110, 110, 110));
    commentFormat_.setFontItalic(true);
    commentRegex_ = QRegularExpression(R"(#.*$)");
  }

protected:
  void highlightBlock(const QString &text) override {
    if (allowHighlight_) {
      const int blockStart = currentBlock().position();
      if (!allowHighlight_(blockStart)) {
        return;
      }
    }
    for (const auto &rule : rules_) {
      QRegularExpressionMatchIterator it = rule.regex.globalMatch(text);
      while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), rule.format);
      }
    }
    QRegularExpressionMatch commentMatch = commentRegex_.match(text);
    if (commentMatch.hasMatch()) {
      setFormat(commentMatch.capturedStart(), commentMatch.capturedLength(), commentFormat_);
    }
  }

private:
  struct HighlightRule {
    QRegularExpression regex;
    QTextCharFormat format;
  };
  std::vector<HighlightRule> rules_;
  QRegularExpression commentRegex_;
  QTextCharFormat commentFormat_;
  std::function<bool(int)> allowHighlight_;
};

class BracketPairer : public QObject {
public:
  explicit BracketPairer(QPlainTextEdit *editor,
                         std::function<bool(int)> allowPairing)
      : QObject(editor),
        editor_(editor),
        allowPairing_(std::move(allowPairing)) {}

protected:
  bool eventFilter(QObject *obj, QEvent *event) override {
    if (obj != editor_ || event->type() != QEvent::KeyPress) {
      return QObject::eventFilter(obj, event);
    }
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    const QString text = keyEvent->text();
    if (text.isEmpty() || !editor_) {
      return QObject::eventFilter(obj, event);
    }
    const int cursorPos = editor_->textCursor().position();
    if (allowPairing_ && !allowPairing_(cursorPos)) {
      return QObject::eventFilter(obj, event);
    }
    if (text == "(" || text == "[" || text == "{") {
      const QChar open = text[0];
      const QChar close = (open == '(') ? ')' : (open == '[' ? ']' : '}');
      QTextCursor cursor = editor_->textCursor();
      cursor.beginEditBlock();
      cursor.insertText(QString(open) + close);
      cursor.movePosition(QTextCursor::Left);
      cursor.endEditBlock();
      editor_->setTextCursor(cursor);
      return true;
    }
    if (keyEvent->key() == Qt::Key_Backspace) {
      QTextCursor cursor = editor_->textCursor();
      if (!cursor.hasSelection()) {
        const int pos = cursor.position();
        const QString text = editor_->toPlainText();
        if (pos > 0 && pos < text.size()) {
          if (allowPairing_ && !(allowPairing_(pos) || allowPairing_(pos - 1))) {
            return QObject::eventFilter(obj, event);
          }
          const QChar prev = text.at(pos - 1);
          const QChar next = text.at(pos);
          if ((prev == '(' && next == ')') ||
              (prev == '[' && next == ']') ||
              (prev == '{' && next == '}')) {
            cursor.beginEditBlock();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            cursor.removeSelectedText();
            cursor.endEditBlock();
            editor_->setTextCursor(cursor);
            return true;
          }
        }
      }
    }
    return QObject::eventFilter(obj, event);
  }

private:
  QPlainTextEdit *editor_ = nullptr;
  std::function<bool(int)> allowPairing_;
};

class BracketMatchHighlighter : public QObject {
public:
  explicit BracketMatchHighlighter(QPlainTextEdit *editor,
                                   std::function<bool(int)> allowHighlight)
      : QObject(editor),
        editor_(editor),
        allowHighlight_(std::move(allowHighlight)) {
    if (editor_) {
      connect(editor_, &QPlainTextEdit::cursorPositionChanged, this,
              &BracketMatchHighlighter::updateHighlight);
    }
  }

private:
  static bool isOpenBracket(QChar ch) {
    return ch == '(' || ch == '[' || ch == '{';
  }

  static bool isCloseBracket(QChar ch) {
    return ch == ')' || ch == ']' || ch == '}';
  }

  static QChar matchingBracket(QChar ch) {
    switch (ch.unicode()) {
      case '(':
        return ')';
      case ')':
        return '(';
      case '[':
        return ']';
      case ']':
        return '[';
      case '{':
        return '}';
      case '}':
        return '{';
      default:
        return QChar();
    }
  }

  int findMatchForward(const QString &text, int pos) const {
    const QChar open = text.at(pos);
    const QChar close = matchingBracket(open);
    int depth = 0;
    for (int i = pos + 1; i < text.size(); ++i) {
      const QChar ch = text.at(i);
      if (ch == open) {
        ++depth;
      } else if (ch == close) {
        if (depth == 0) {
          return i;
        }
        --depth;
      }
    }
    return -1;
  }

  int findMatchBackward(const QString &text, int pos) const {
    const QChar close = text.at(pos);
    const QChar open = matchingBracket(close);
    int depth = 0;
    for (int i = pos - 1; i >= 0; --i) {
      const QChar ch = text.at(i);
      if (ch == close) {
        ++depth;
      } else if (ch == open) {
        if (depth == 0) {
          return i;
        }
        --depth;
      }
    }
    return -1;
  }

  void updateHighlight() {
    if (!editor_) {
      return;
    }
    QList<QTextEdit::ExtraSelection> selections;
    const QString text = editor_->toPlainText();
    if (text.isEmpty()) {
      editor_->setExtraSelections(selections);
      return;
    }

    QTextCursor cursor = editor_->textCursor();
    int pos = cursor.position();
    int bracketPos = -1;
    if (pos >= 0 && pos < text.size() && (isOpenBracket(text.at(pos)) ||
                                          isCloseBracket(text.at(pos)))) {
      bracketPos = pos;
    } else if (pos > 0 && (isOpenBracket(text.at(pos - 1)) ||
                           isCloseBracket(text.at(pos - 1)))) {
      bracketPos = pos - 1;
    }

    if (bracketPos == -1) {
      editor_->setExtraSelections(selections);
      return;
    }
    if (allowHighlight_ && !allowHighlight_(bracketPos)) {
      editor_->setExtraSelections(selections);
      return;
    }

    const QChar bracket = text.at(bracketPos);
    int matchPos = -1;
    if (isOpenBracket(bracket)) {
      matchPos = findMatchForward(text, bracketPos);
    } else if (isCloseBracket(bracket)) {
      matchPos = findMatchBackward(text, bracketPos);
    }
    if (matchPos == -1) {
      editor_->setExtraSelections(selections);
      return;
    }
    if (allowHighlight_ && !allowHighlight_(matchPos)) {
      editor_->setExtraSelections(selections);
      return;
    }

    QTextCharFormat format;
    format.setForeground(QColor(180, 0, 0));
    format.setBackground(QColor(255, 220, 220));

    QTextEdit::ExtraSelection first;
    QTextCursor firstCursor(editor_->document());
    firstCursor.setPosition(bracketPos);
    firstCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    first.cursor = firstCursor;
    first.format = format;
    selections.append(first);

    QTextEdit::ExtraSelection second;
    QTextCursor secondCursor(editor_->document());
    secondCursor.setPosition(matchPos);
    secondCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    second.cursor = secondCursor;
    second.format = format;
    selections.append(second);

    editor_->setExtraSelections(selections);
  }

  QPlainTextEdit *editor_ = nullptr;
  std::function<bool(int)> allowHighlight_;
};
