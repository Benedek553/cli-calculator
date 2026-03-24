MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("CLI Calculator GUI");
  resize(1280, 640);
  applyDevTheme();

  auto *fileMenu = menuBar()->addMenu("File");
  auto *settingsMenu = menuBar()->addMenu("Settings");
  auto *helpMenu = menuBar()->addMenu("Help");

  auto *clearOutputAction = fileMenu->addAction("Clear Output");
  auto *clearHistoryAction = fileMenu->addAction("Clear Expression History");
  fileMenu->addSeparator();
  auto *evalModeMenu = fileMenu->addMenu("Evaluation Mode");
  auto *evalModeGroup = new QActionGroup(this);
  evalStandardAction_ = evalModeMenu->addAction("Standard (double)");
  evalStandardAction_->setCheckable(true);
  evalBigIntAction_ = evalModeMenu->addAction("BigInt (integers only)");
  evalBigIntAction_->setCheckable(true);
  evalBigDoubleAction_ = evalModeMenu->addAction("BigDouble (high-precision decimals)");
  evalBigDoubleAction_->setCheckable(true);
  evalModeGroup->addAction(evalStandardAction_);
  evalModeGroup->addAction(evalBigIntAction_);
  evalModeGroup->addAction(evalBigDoubleAction_);
  evalStandardAction_->setChecked(true);
  notesNewAction_ = fileMenu->addAction("Notes: New");
  notesOpenAction_ = fileMenu->addAction("Notes: Open");
  notesSaveAction_ = fileMenu->addAction("Notes: Save");
  notesSaveAsAction_ = fileMenu->addAction("Notes: Save As");
  notesCopyAction_ = fileMenu->addAction("Notes: Copy");
  fileMenu->addSeparator();
  auto *exitAction = fileMenu->addAction("Exit");

  auto *defaultBigIntAction = settingsMenu->addAction("Use BigInt by Default");
  defaultBigIntAction->setCheckable(true);

  auto *modeMenu = settingsMenu->addMenu("Mode");
  auto *desktopModeAction = modeMenu->addAction("Desktop");
  auto *touchModeAction = modeMenu->addAction("Touchscreen");
  desktopModeAction->setCheckable(true);
  touchModeAction->setCheckable(true);
  desktopModeAction->setChecked(true);

  auto *matrixEditorAction = settingsMenu->addAction("Enable Matrix Editor");
  matrixEditorAction->setCheckable(true);
  matrixEditorAction->setChecked(true);

  auto *wrapOutputAction = settingsMenu->addAction("Wrap Output Text");
  wrapOutputAction->setCheckable(true);
  wrapOutputAction->setChecked(false);

  auto *splitViewAction = settingsMenu->addAction("Split View");
  splitViewAction->setCheckable(true);
  splitViewAction->setChecked(false);

  auto *toggleNotesEditorAction = settingsMenu->addAction("Toggle Notes Editor");
  toggleNotesEditorAction->setCheckable(true);
  toggleNotesEditorAction->setChecked(true);
  notesTogglePreviewAction_ = settingsMenu->addAction("Toggle Notes Preview");
  notesTogglePreviewAction_->setCheckable(true);
  notesTogglePreviewAction_->setChecked(true);

  auto *aboutAction = helpMenu->addAction("About");
  auto *contributeAction = helpMenu->addAction("Contribute");
  auto *reportBugAction = helpMenu->addAction("Report a Bug");
  auto *requestFeatureAction = helpMenu->addAction("Request a Feature");

  auto *central = new QWidget(this);
  central->setAutoFillBackground(true);
  auto *layout = new QVBoxLayout(central);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  mainTabs_ = new QTabWidget(central);
  mainContentStack_ = new QStackedWidget(central);
  splitViewContainer_ = new QWidget(central);
  splitLeftStack_ = new QStackedWidget(splitViewContainer_);
  splitRightStack_ = new QStackedWidget(splitViewContainer_);
  tabPool_ = new QStackedWidget(central);
  tabPool_->setVisible(false);

  splitBarContainer_ = new QWidget(central);
  auto *splitBarLayout = new QHBoxLayout(splitBarContainer_);
  splitBarLayout->setContentsMargins(0, 0, 0, 0);
  auto *splitLabel = new QLabel("Split View");
  splitLeftSelect_ = new QComboBox();
  splitRightSelect_ = new QComboBox();
  splitBarLayout->addWidget(splitLabel);
  splitBarLayout->addWidget(new QLabel("Left"));
  splitBarLayout->addWidget(splitLeftSelect_);
  splitBarLayout->addSpacing(12);
  splitBarLayout->addWidget(new QLabel("Right"));
  splitBarLayout->addWidget(splitRightSelect_);
  splitBarLayout->addStretch();
  splitBarContainer_->setVisible(false);

  auto *splitTabsSplitter = new QSplitter(Qt::Horizontal, splitViewContainer_);
  splitTabsSplitter->addWidget(splitLeftStack_);
  splitTabsSplitter->addWidget(splitRightStack_);
  splitTabsSplitter->setStretchFactor(0, 1);
  splitTabsSplitter->setStretchFactor(1, 1);
  auto *splitViewLayout = new QVBoxLayout(splitViewContainer_);
  splitViewLayout->setContentsMargins(0, 0, 0, 0);
  splitViewLayout->addWidget(splitTabsSplitter);

  mainContentStack_->addWidget(mainTabs_);
  mainContentStack_->addWidget(splitViewContainer_);

  auto *expressionTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(expressionTab);
    auto *formLayout = new QFormLayout();
    exprInput_ = new QLineEdit();
    exprInput_->setPlaceholderText("e.g. (2 + 3) * 4 - sin(1)");
    bigIntCheck_ = new QCheckBox("Use BigInt (integers only)");
    formLayout->addRow("Expression", exprInput_);
    formLayout->addRow("", bigIntCheck_);
    auto *runButton = new QPushButton("Evaluate");
    auto *buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget(runButton);
    exprHistory_ = new QPlainTextEdit();
    exprHistory_->setReadOnly(true);
    exprHistory_->setPlaceholderText("Expression history...");
    auto *historyLayout = new QVBoxLayout();
    historyLayout->addWidget(exprHistory_);
    tabLayout->addLayout(formLayout);
    tabLayout->addLayout(buttonRow);
    tabLayout->addWidget(wrapSection("History", historyLayout));
    tabLayout->addStretch();

    connect(runButton, &QPushButton::clicked, this, [this]() {
      const auto expression = exprInput_->text().toStdString();
      const bool useBigInt = (evalMode_ == EvalMode::BigInt);
      const bool useBigDouble = (evalMode_ == EvalMode::BigDouble);
      runAndShow("Expression",
                 [expression, useBigInt, useBigDouble](OutputFormat format) {
                   return runEval(expression, format, nullptr, useBigInt,
                                  useBigDouble);
      });
      if (exprHistory_) {
        const QString outputText = output_ ? output_->toPlainText().trimmed()
                                           : QString();
        QString entry = QString(">> %1").arg(exprInput_->text().trimmed());
        if (!outputText.isEmpty()) {
          entry += "\n" + outputText;
        }
        exprHistory_->appendPlainText(entry);
        exprHistory_->appendPlainText("");
      }
    });
    connect(bigIntCheck_, &QCheckBox::toggled, this, [this](bool checked) {
      if (checked) {
        setEvalMode(EvalMode::BigInt);
      } else if (evalMode_ == EvalMode::BigInt) {
        setEvalMode(EvalMode::Standard);
      }
    });
    connect(exprInput_, &QLineEdit::returnPressed, runButton, &QPushButton::click);
  }
  registerTab("Expressions", expressionTab);

  auto *numberTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(numberTab);
    auto *numberTabs = new QTabWidget(numberTab);

    auto *sqrtTab = new QWidget(numberTabs);
    auto *sqrtLayout = new QVBoxLayout(sqrtTab);
    auto *sqrtForm = new QFormLayout();
    sqrtInput_ = new QLineEdit();
    sqrtInput_->setPlaceholderText("e.g. 9");
    sqrtForm->addRow("Value", sqrtInput_);
    auto *sqrtButton = new QPushButton("Square Root");
    auto *sqrtRow = new QHBoxLayout();
    sqrtRow->addStretch();
    sqrtRow->addWidget(sqrtButton);
    sqrtForm->addRow("", sqrtRow);
    sqrtLayout->addWidget(wrapSection("Square Root", sqrtForm));
    sqrtLayout->addStretch();
    numberTabs->addTab(sqrtTab, "Square Root");

    auto *divTab = new QWidget(numberTabs);
    auto *divLayout = new QVBoxLayout(divTab);
    auto *divForm = new QFormLayout();
    divisorsInput_ = new QLineEdit();
    divisorsInput_->setPlaceholderText("e.g. 28");
    divForm->addRow("Number", divisorsInput_);
    auto *divButton = new QPushButton("Divisors");
    auto *divRow = new QHBoxLayout();
    divRow->addStretch();
    divRow->addWidget(divButton);
    divForm->addRow("", divRow);
    divLayout->addWidget(wrapSection("Divisors", divForm));
    divLayout->addStretch();
    numberTabs->addTab(divTab, "Divisors");

    auto *primeTab = new QWidget(numberTabs);
    auto *primeLayout = new QVBoxLayout(primeTab);
    auto *primeForm = new QFormLayout();
    primeFactorsInput_ = new QLineEdit();
    primeFactorsInput_->setPlaceholderText("e.g. 84");
    primeForm->addRow("Number", primeFactorsInput_);
    auto *primeButton = new QPushButton("Prime Factorization");
    auto *primeRow = new QHBoxLayout();
    primeRow->addStretch();
    primeRow->addWidget(primeButton);
    primeForm->addRow("", primeRow);
    primeLayout->addWidget(wrapSection("Prime Factors", primeForm));
    primeLayout->addStretch();
    numberTabs->addTab(primeTab, "Prime Factors");

    tabLayout->addWidget(numberTabs);
    tabLayout->addStretch();

    connect(sqrtButton, &QPushButton::clicked, this, [this]() {
      const auto value = sqrtInput_->text().toStdString();
      runAndShow("Square Root", [value](OutputFormat format) {
        return runSquareRoot(value, format, nullptr);
      });
    });
    connect(sqrtInput_, &QLineEdit::returnPressed, sqrtButton, &QPushButton::click);

    connect(divButton, &QPushButton::clicked, this, [this]() {
      const auto value = divisorsInput_->text().toStdString();
      runAndShow("Divisors", [value](OutputFormat format) {
        return runDivisors(value, format);
      });
    });
    connect(divisorsInput_, &QLineEdit::returnPressed, divButton, &QPushButton::click);

    connect(primeButton, &QPushButton::clicked, this, [this]() {
      const auto value = primeFactorsInput_->text().toStdString();
      runAndShow("Prime Factors", [value](OutputFormat format) {
        return runPrimeFactorization(value, format);
      });
    });
    connect(primeFactorsInput_, &QLineEdit::returnPressed, primeButton, &QPushButton::click);
  }
  registerTab("Numbers", numberTab);

  auto *convertTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(convertTab);
    auto *convertTabs = new QTabWidget(convertTab);

    auto *baseTab = new QWidget(convertTabs);
    auto *baseLayout = new QVBoxLayout(baseTab);
    auto *baseForm = new QFormLayout();
    baseFromInput_ = new QComboBox();
    baseToInput_ = new QComboBox();
    const std::vector<std::pair<QString, QString>> baseOptions = {
        {QStringLiteral("Binary (2)"), QStringLiteral("2")},
        {QStringLiteral("Decimal (10)"), QStringLiteral("10")},
        {QStringLiteral("Hex (16)"), QStringLiteral("16")},
    };
    for (const auto &option : baseOptions) {
      baseFromInput_->addItem(option.first, option.second);
      baseToInput_->addItem(option.first, option.second);
    }
    baseValueInput_ = new QLineEdit();
    baseValueInput_->setPlaceholderText("Value to convert");
    baseForm->addRow("From base", baseFromInput_);
    baseForm->addRow("To base", baseToInput_);
    baseForm->addRow("Value", baseValueInput_);
    auto *baseButton = new QPushButton("Convert Bases");
    auto *baseRow = new QHBoxLayout();
    baseRow->addStretch();
    baseRow->addWidget(baseButton);
    baseForm->addRow("", baseRow);
    baseLayout->addWidget(wrapSection("Base Conversion", baseForm));
    baseLayout->addStretch();
    convertTabs->addTab(baseTab, "Base Conversion");

    auto *unitTab = new QWidget(convertTabs);
    auto *unitLayout = new QVBoxLayout(unitTab);
    auto *unitForm = new QFormLayout();
    unitCategoryInput_ = new QComboBox();
    unitFromInput_ = new QComboBox();
    unitToInput_ = new QComboBox();
    unitCategoryInput_->addItem("Length", "length");
    unitCategoryInput_->addItem("Mass", "mass");
    unitCategoryInput_->addItem("Volume", "volume");
    unitCategoryInput_->addItem("Temperature", "temperature");
    unitValueInput_ = new QLineEdit();
    unitForm->addRow("Category", unitCategoryInput_);
    unitForm->addRow("From unit", unitFromInput_);
    unitForm->addRow("To unit", unitToInput_);
    unitForm->addRow("Value", unitValueInput_);
    auto *unitButton = new QPushButton("Convert Units");
    auto *unitRow = new QHBoxLayout();
    unitRow->addStretch();
    unitRow->addWidget(unitButton);
    unitForm->addRow("", unitRow);
    unitLayout->addWidget(wrapSection("Unit Conversion", unitForm));
    unitLayout->addStretch();
    convertTabs->addTab(unitTab, "Unit Conversion");

    tabLayout->addWidget(convertTabs);
    tabLayout->addStretch();

    auto refreshUnits = [this]() {
      unitFromInput_->clear();
      unitToInput_->clear();
      const QString categoryToken = unitCategoryInput_->currentData().toString();
      if (categoryToken == "temperature") {
        for (const auto &unit : temperatureUnits()) {
          const QString label = QString::fromStdString(unit.name + " (" + unit.symbol + ")");
          const QString token = QString::fromStdString(unit.symbol);
          unitFromInput_->addItem(label, token);
          unitToInput_->addItem(label, token);
        }
        return;
      }
      for (const auto &category : linearCategories()) {
        if (toLower(category.name) == categoryToken.toStdString()) {
          for (const auto &unit : category.units) {
            const QString label = QString::fromStdString(unit.name + " (" + unit.symbol + ")");
            const QString token = QString::fromStdString(unit.symbol);
            unitFromInput_->addItem(label, token);
            unitToInput_->addItem(label, token);
          }
          break;
        }
      }
    };
    refreshUnits();

    connect(unitCategoryInput_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [refreshUnits](int) { refreshUnits(); });

    connect(baseButton, &QPushButton::clicked, this, [this]() {
      const auto fromBase = baseFromInput_->currentData().toString().toStdString();
      const auto toBase = baseToInput_->currentData().toString().toStdString();
      const auto value = baseValueInput_->text().toStdString();
      runAndShow("Base Conversion", [fromBase, toBase, value](OutputFormat format) {
        return runConvert(fromBase, toBase, value, format);
      });
    });
    connect(baseValueInput_, &QLineEdit::returnPressed, baseButton, &QPushButton::click);

    connect(unitButton, &QPushButton::clicked, this, [this]() {
      const auto category = unitCategoryInput_->currentData().toString().toStdString();
      const auto fromUnit = unitFromInput_->currentData().toString().toStdString();
      const auto toUnit = unitToInput_->currentData().toString().toStdString();
      const auto value = unitValueInput_->text().toStdString();
      runAndShow("Unit Conversion", [category, fromUnit, toUnit, value](OutputFormat format) {
        return runUnitConvert(category, fromUnit, toUnit, value, format);
      });
    });
    connect(unitValueInput_, &QLineEdit::returnPressed, unitButton, &QPushButton::click);
  }
  registerTab("Conversions", convertTab);

  auto *equationsTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(equationsTab);
    auto *equationTabs = new QTabWidget(equationsTab);

    auto *linearTab = new QWidget(equationTabs);
    auto *linearLayout = new QVBoxLayout(linearTab);
    auto *linearForm = new QFormLayout();
    linearAInput_ = new QLineEdit();
    linearBInput_ = new QLineEdit();
    linearForm->addRow("a", linearAInput_);
    linearForm->addRow("b", linearBInput_);
    auto *linearButton = new QPushButton("Solve Linear");
    auto *linearRow = new QHBoxLayout();
    linearRow->addStretch();
    linearRow->addWidget(linearButton);
    linearForm->addRow("", linearRow);
    linearLayout->addWidget(wrapSection("Linear: a*x + b = 0", linearForm));
    linearLayout->addStretch();
    equationTabs->addTab(linearTab, "Linear");

    auto *quadTab = new QWidget(equationTabs);
    auto *quadLayout = new QVBoxLayout(quadTab);
    auto *quadForm = new QFormLayout();
    quadraticAInput_ = new QLineEdit();
    quadraticBInput_ = new QLineEdit();
    quadraticCInput_ = new QLineEdit();
    quadForm->addRow("a", quadraticAInput_);
    quadForm->addRow("b", quadraticBInput_);
    quadForm->addRow("c", quadraticCInput_);
    auto *quadButton = new QPushButton("Solve Quadratic");
    auto *quadRow = new QHBoxLayout();
    quadRow->addStretch();
    quadRow->addWidget(quadButton);
    quadForm->addRow("", quadRow);
    quadLayout->addWidget(wrapSection("Quadratic: a*x^2 + b*x + c = 0", quadForm));
    quadLayout->addStretch();
    equationTabs->addTab(quadTab, "Quadratic");

    tabLayout->addWidget(equationTabs);
    tabLayout->addStretch();

    connect(linearButton, &QPushButton::clicked, this, [this]() {
      const auto aValue = linearAInput_->text().toStdString();
      const auto bValue = linearBInput_->text().toStdString();
      runAndShow("Solve Linear", [aValue, bValue](OutputFormat format) {
        return runSolveLinear(aValue, bValue, format);
      });
    });
    connect(linearAInput_, &QLineEdit::returnPressed, linearButton, &QPushButton::click);
    connect(linearBInput_, &QLineEdit::returnPressed, linearButton, &QPushButton::click);

    connect(quadButton, &QPushButton::clicked, this, [this]() {
      const auto aValue = quadraticAInput_->text().toStdString();
      const auto bValue = quadraticBInput_->text().toStdString();
      const auto cValue = quadraticCInput_->text().toStdString();
      runAndShow("Solve Quadratic", [aValue, bValue, cValue](OutputFormat format) {
        return runSolveQuadratic(aValue, bValue, cValue, format);
      });
    });
    connect(quadraticAInput_, &QLineEdit::returnPressed, quadButton, &QPushButton::click);
    connect(quadraticBInput_, &QLineEdit::returnPressed, quadButton, &QPushButton::click);
    connect(quadraticCInput_, &QLineEdit::returnPressed, quadButton, &QPushButton::click);
  }
  registerTab("Equations", equationsTab);

  auto *matrixTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(matrixTab);
    matrixInputStack_ = new QStackedWidget();

    auto *matrixEditorWidget = new QWidget(matrixInputStack_);
    auto *editorLayout = new QVBoxLayout(matrixEditorWidget);
    matrixAEditor_ = new MatrixEditor(matrixEditorWidget);
    matrixBEditor_ = new MatrixEditor(matrixEditorWidget);
    auto *matrixALayout = new QVBoxLayout();
    matrixALayout->addWidget(matrixAEditor_);
    auto *matrixBLayout = new QVBoxLayout();
    matrixBLayout->addWidget(matrixBEditor_);
    editorLayout->addWidget(wrapSection("Matrix A", matrixALayout));
    editorLayout->addWidget(wrapSection("Matrix B", matrixBLayout));
    matrixEditorWidget->setLayout(editorLayout);

    auto *matrixTextWidget = new QWidget(matrixInputStack_);
    auto *matrixForm = new QFormLayout(matrixTextWidget);
    matrixAInput_ = new QLineEdit();
    matrixAInput_->setPlaceholderText("Rows ';', columns ',' or spaces");
    matrixBInput_ = new QLineEdit();
    matrixBInput_->setPlaceholderText("Rows ';', columns ',' or spaces");
    matrixForm->addRow("Matrix A", matrixAInput_);
    matrixForm->addRow("Matrix B", matrixBInput_);
    matrixTextWidget->setLayout(matrixForm);

    matrixInputStack_->addWidget(matrixEditorWidget);
    matrixInputStack_->addWidget(matrixTextWidget);
    matrixInputStack_->setCurrentIndex(0);

    auto *buttonsRow = new QHBoxLayout();
    auto *addButton = new QPushButton("Add");
    auto *subButton = new QPushButton("Subtract");
    auto *mulButton = new QPushButton("Multiply");
    buttonsRow->addStretch();
    buttonsRow->addWidget(addButton);
    buttonsRow->addWidget(subButton);
    buttonsRow->addWidget(mulButton);

    auto *matrixGroupLayout = new QVBoxLayout();
    matrixGroupLayout->addWidget(matrixInputStack_);
    matrixGroupLayout->addLayout(buttonsRow);
    tabLayout->addWidget(wrapSection("Matrix Operations", matrixGroupLayout));
    tabLayout->addStretch();

    connect(addButton, &QPushButton::clicked, this, [this]() {
      const auto matrixA = matrixInputStack_->currentIndex() == 0
                               ? matrixAEditor_->toCliString()
                               : matrixAInput_->text().toStdString();
      const auto matrixB = matrixInputStack_->currentIndex() == 0
                               ? matrixBEditor_->toCliString()
                               : matrixBInput_->text().toStdString();
      runAndShow("Matrix Add", [matrixA, matrixB](OutputFormat format) {
        return runMatrixAdd(matrixA, matrixB, format);
      });
    });

    connect(subButton, &QPushButton::clicked, this, [this]() {
      const auto matrixA = matrixInputStack_->currentIndex() == 0
                               ? matrixAEditor_->toCliString()
                               : matrixAInput_->text().toStdString();
      const auto matrixB = matrixInputStack_->currentIndex() == 0
                               ? matrixBEditor_->toCliString()
                               : matrixBInput_->text().toStdString();
      runAndShow("Matrix Subtract", [matrixA, matrixB](OutputFormat format) {
        return runMatrixSubtract(matrixA, matrixB, format);
      });
    });

    connect(mulButton, &QPushButton::clicked, this, [this]() {
      const auto matrixA = matrixInputStack_->currentIndex() == 0
                               ? matrixAEditor_->toCliString()
                               : matrixAInput_->text().toStdString();
      const auto matrixB = matrixInputStack_->currentIndex() == 0
                               ? matrixBEditor_->toCliString()
                               : matrixBInput_->text().toStdString();
      runAndShow("Matrix Multiply", [matrixA, matrixB](OutputFormat format) {
        return runMatrixMultiply(matrixA, matrixB, format);
      });
    });
    connect(matrixAInput_, &QLineEdit::returnPressed, addButton, &QPushButton::click);
    connect(matrixBInput_, &QLineEdit::returnPressed, addButton, &QPushButton::click);
  }
  registerTab("Matrices", matrixTab);

  auto *graphTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(graphTab);
    auto *graphTabs = new QTabWidget(graphTab);

    auto *manualTab = new QWidget(graphTabs);
    auto *manualLayout = new QVBoxLayout(manualTab);
    auto *valuesForm = new QFormLayout();
    graphValuesInput_ = new QLineEdit();
    graphValuesInput_->setPlaceholderText("e.g. 1 2 3 4 5");
    graphValuesHeightSpin_ = new QSpinBox();
    graphValuesHeightSpin_->setRange(2, 40);
    graphValuesHeightSpin_->setValue(10);
    valuesForm->addRow("Values", graphValuesInput_);
    valuesForm->addRow("Height", graphValuesHeightSpin_);
    auto *valuesButton = new QPushButton("Generate from Values");
    auto *valuesRow = new QHBoxLayout();
    valuesRow->addStretch();
    valuesRow->addWidget(valuesButton);
    valuesForm->addRow("", valuesRow);
    manualLayout->addWidget(wrapSection("Manual Values", valuesForm));
    manualLayout->addStretch();
    graphTabs->addTab(manualTab, "Manual Values");

    auto *csvTab = new QWidget(graphTabs);
    auto *csvLayout = new QVBoxLayout(csvTab);
    auto *csvForm = new QFormLayout();
    graphCsvPathInput_ = new QLineEdit();
    graphCsvPathInput_->setPlaceholderText("Path to CSV file");
    auto *csvBrowseButton = new QPushButton("Browse");
    auto *csvPathRow = new QHBoxLayout();
    csvPathRow->addWidget(graphCsvPathInput_);
    csvPathRow->addWidget(csvBrowseButton);
    csvForm->addRow("CSV path", csvPathRow);
    graphCsvColumnInput_ = new QLineEdit();
    graphCsvColumnInput_->setPlaceholderText("Column name or index (1-based)");
    graphCsvHeightSpin_ = new QSpinBox();
    graphCsvHeightSpin_->setRange(2, 40);
    graphCsvHeightSpin_->setValue(10);
    graphCsvHeadersCheck_ = new QCheckBox("CSV includes headers");
    graphCsvHeadersCheck_->setChecked(true);
    csvForm->addRow("Column", graphCsvColumnInput_);
    csvForm->addRow("Height", graphCsvHeightSpin_);
    csvForm->addRow("", graphCsvHeadersCheck_);
    auto *csvButton = new QPushButton("Generate from CSV");
    auto *csvRow = new QHBoxLayout();
    csvRow->addStretch();
    csvRow->addWidget(csvButton);
    csvForm->addRow("", csvRow);
    csvLayout->addWidget(wrapSection("Import from CSV", csvForm));
    csvLayout->addStretch();
    graphTabs->addTab(csvTab, "Import from CSV");

    tabLayout->addWidget(graphTabs);

    graphPreviewLabel_ = new QLabel("No graph generated yet.");
    graphPreviewLabel_->setAlignment(Qt::AlignCenter);
    graphPreviewLabel_->setMinimumHeight(220);
    graphPreviewLabel_->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    graphPreviewLabel_->setScaledContents(true);
    auto *saveGraphButton = new QPushButton("Save As...");
    auto *previewRow = new QHBoxLayout();
    previewRow->addStretch();
    previewRow->addWidget(saveGraphButton);

    auto *previewLayout = new QVBoxLayout();
    previewLayout->addWidget(graphPreviewLabel_);
    previewLayout->addLayout(previewRow);
    tabLayout->addWidget(wrapSection("Graph Preview", previewLayout));

    tabLayout->addStretch();

    connect(csvBrowseButton, &QPushButton::clicked, this, [this]() {
      const QString path = QFileDialog::getOpenFileName(
          this, "Select CSV", QString(), "CSV Files (*.csv);;All Files (*)");
      if (!path.isEmpty()) {
        graphCsvPathInput_->setText(path);
      }
    });

    connect(valuesButton, &QPushButton::clicked, this, [this]() {
      const QString valuesText = graphValuesInput_->text();
      const auto tokens = splitTokens(valuesText);
      if (tokens.empty()) {
        showGraphError("Graph Values", "Provide at least one value.");
        return;
      }
      std::vector<std::string> args;
      const QString outputPath = makeTempGraphPath();
      args.push_back(outputPath.toStdString());
      args.insert(args.end(), tokens.begin(), tokens.end());
      args.push_back("--height");
      args.push_back(std::to_string(graphValuesHeightSpin_->value()));
      CliRunResult result = runCliAction([&]() {
        return runGraphValues(args, OutputFormat::Text);
      });
      if (result.exitCode != 0) {
        const std::string combined = result.err + result.out;
        showGraphError("Graph Values", QString::fromStdString(combined));
        return;
      }
      showGraphPreview("Graph Values", outputPath);
    });

    connect(csvButton, &QPushButton::clicked, this, [this]() {
      const QString csvPath = graphCsvPathInput_->text().trimmed();
      const QString column = graphCsvColumnInput_->text().trimmed();
      if (csvPath.isEmpty() || column.isEmpty()) {
        showGraphError("Graph CSV", "CSV path and column are required.");
        return;
      }
      std::vector<std::string> args;
      const QString outputPath = makeTempGraphPath();
      args.push_back(outputPath.toStdString());
      args.push_back(csvPath.toStdString());
      args.push_back(column.toStdString());
      args.push_back("--height");
      args.push_back(std::to_string(graphCsvHeightSpin_->value()));
      if (!graphCsvHeadersCheck_->isChecked()) {
        args.push_back("--no-headers");
      }
      CliRunResult result = runCliAction([&]() {
        return runGraphCsv(args, OutputFormat::Text);
      });
      if (result.exitCode != 0) {
        const std::string combined = result.err + result.out;
        showGraphError("Graph CSV", QString::fromStdString(combined));
        return;
      }
      showGraphPreview("Graph CSV", outputPath);
    });
    connect(graphValuesInput_, &QLineEdit::returnPressed, valuesButton, &QPushButton::click);
    connect(graphCsvPathInput_, &QLineEdit::returnPressed, csvButton, &QPushButton::click);
    connect(graphCsvColumnInput_, &QLineEdit::returnPressed, csvButton, &QPushButton::click);

    connect(saveGraphButton, &QPushButton::clicked, this, [this]() {
      if (currentGraphPath_.isEmpty()) {
        statusBar()->showMessage("No graph to save yet", 3000);
        return;
      }
      QString path = QFileDialog::getSaveFileName(
          this, "Save Graph", "graph.png", "PNG Image (*.png)");
      if (path.isEmpty()) {
        return;
      }
      if (!path.endsWith(".png", Qt::CaseInsensitive)) {
        path += ".png";
      }
      if (QFile::exists(path)) {
        QFile::remove(path);
      }
      if (!QFile::copy(currentGraphPath_, path)) {
        statusBar()->showMessage("Failed to save graph", 3000);
        return;
      }
      statusBar()->showMessage("Graph saved", 3000);
    });
  }
  registerTab("Graphs", graphTab);

  auto *statsTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(statsTab);
    auto *statsForm = new QFormLayout();
    statsInput_ = new QLineEdit();
    statsInput_->setPlaceholderText("Values separated by spaces");
    statsForm->addRow("Values", statsInput_);
    auto *statsButton = new QPushButton("Statistics");
    auto *statsRow = new QHBoxLayout();
    statsRow->addStretch();
    statsRow->addWidget(statsButton);
    statsForm->addRow("", statsRow);
    tabLayout->addWidget(wrapSection("Summary Statistics", statsForm));
    tabLayout->addStretch();

    connect(statsButton, &QPushButton::clicked, this, [this]() {
      const auto tokens = splitTokens(statsInput_->text());
      runAndShow("Statistics", [tokens](OutputFormat format) {
        return runStatistics(tokens, format);
      });
    });
    connect(statsInput_, &QLineEdit::returnPressed, statsButton, &QPushButton::click);
  }
  registerTab("Statistics", statsTab);

  auto *varsTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(varsTab);
    auto *varsTabs = new QTabWidget(varsTab);

    auto *listTab = new QWidget(varsTabs);
    auto *listLayout = new QVBoxLayout(listTab);
    auto *listButton = new QPushButton("List Variables");
    auto *listSectionLayout = new QVBoxLayout();
    listSectionLayout->addWidget(listButton);
    listLayout->addWidget(wrapSection("List", listSectionLayout));
    listLayout->addStretch();
    varsTabs->addTab(listTab, "List");

    auto *setTab = new QWidget(varsTabs);
    auto *setLayout = new QVBoxLayout(setTab);
    auto *setForm = new QFormLayout();
    varNameInput_ = new QLineEdit();
    varValueInput_ = new QLineEdit();
    setForm->addRow("Name", varNameInput_);
    setForm->addRow("Value", varValueInput_);
    auto *setButton = new QPushButton("Set Variable");
    auto *setRow = new QHBoxLayout();
    setRow->addStretch();
    setRow->addWidget(setButton);
    setForm->addRow("", setRow);
    setLayout->addWidget(wrapSection("Set", setForm));
    setLayout->addStretch();
    varsTabs->addTab(setTab, "Set");

    auto *unsetTab = new QWidget(varsTabs);
    auto *unsetLayout = new QVBoxLayout(unsetTab);
    auto *unsetForm = new QFormLayout();
    unsetVarInput_ = new QLineEdit();
    unsetForm->addRow("Name", unsetVarInput_);
    auto *unsetButton = new QPushButton("Unset Variable");
    auto *unsetRow = new QHBoxLayout();
    unsetRow->addStretch();
    unsetRow->addWidget(unsetButton);
    unsetForm->addRow("", unsetRow);
    unsetLayout->addWidget(wrapSection("Unset", unsetForm));
    unsetLayout->addStretch();
    varsTabs->addTab(unsetTab, "Unset");

    tabLayout->addWidget(varsTabs);
    tabLayout->addStretch();

    connect(listButton, &QPushButton::clicked, this, [this]() {
      runAndShow("Variables", [](OutputFormat format) { return runListVariables(format); });
    });

    connect(setButton, &QPushButton::clicked, this, [this]() {
      const auto name = varNameInput_->text().toStdString();
      const auto value = varValueInput_->text().toStdString();
      runAndShow("Set Variable", [name, value](OutputFormat format) {
        return runSetVariable(name, value, format);
      });
    });
    connect(varNameInput_, &QLineEdit::returnPressed, setButton, &QPushButton::click);
    connect(varValueInput_, &QLineEdit::returnPressed, setButton, &QPushButton::click);

    connect(unsetButton, &QPushButton::clicked, this, [this]() {
      const auto name = unsetVarInput_->text().toStdString();
      runAndShow("Unset Variable", [name](OutputFormat format) {
        return runUnsetVariable(name, format);
      });
    });
    connect(unsetVarInput_, &QLineEdit::returnPressed, unsetButton, &QPushButton::click);
  }
  registerTab("Variables", varsTab);

  auto *terminalTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(terminalTab);
    terminalOutput_ = new QPlainTextEdit(terminalTab);
    terminalOutput_->setReadOnly(true);
    terminalOutput_->setFont(devMonoFont());
    terminalOutput_->setPlaceholderText("Terminal output...");

    terminalInput_ = new QLineEdit(terminalTab);
    terminalInput_->setPlaceholderText("Enter a command and press Enter");

    auto *inputRow = new QHBoxLayout();
    auto *runButton = new QPushButton("Run");
    inputRow->addWidget(terminalInput_);
    inputRow->addWidget(runButton);

    tabLayout->addWidget(terminalOutput_);
    tabLayout->addLayout(inputRow);

    terminalCwd_ = QDir::currentPath();
    terminalOutput_->appendPlainText(
        "Welcome to the CLI Calculator terminal.\n"
        "Run calculator commands (e.g. calculator --help) or any shell command.\n"
        "Working directory: " + terminalCwd_ + "\n");

    auto runCommand = [this]() {
      const QString command = terminalInput_->text().trimmed();
      if (command.isEmpty()) {
        return;
      }
      terminalOutput_->appendPlainText(
          QString("%1$ %2").arg(terminalCwd_, command));
      terminalInput_->clear();

      if (command.startsWith("cd ")) {
        QString target = command.mid(3).trimmed();
        if (target.isEmpty()) {
          target = QDir::homePath();
        }
        QDir dir(terminalCwd_);
        if (dir.cd(target)) {
          terminalCwd_ = dir.absolutePath();
        } else {
          terminalOutput_->appendPlainText("cd: no such directory\n");
        }
        return;
      }
      if (command == "cd") {
        terminalCwd_ = QDir::homePath();
        return;
      }

      QProcess process;
      process.setWorkingDirectory(terminalCwd_);
      process.start("/bin/bash", QStringList() << "-lc" << command);
      if (!process.waitForFinished(-1)) {
        terminalOutput_->appendPlainText("Command failed to start.\n");
        return;
      }
      const QString stdoutText = QString::fromUtf8(process.readAllStandardOutput());
      const QString stderrText = QString::fromUtf8(process.readAllStandardError());
      if (!stdoutText.isEmpty()) {
        terminalOutput_->appendPlainText(stdoutText);
      }
      if (!stderrText.isEmpty()) {
        terminalOutput_->appendPlainText(stderrText);
      }
    };

    connect(runButton, &QPushButton::clicked, this, runCommand);
    connect(terminalInput_, &QLineEdit::returnPressed, this, runCommand);
  }
  registerTab("Terminal", terminalTab);

  auto *notesTab = new QWidget(mainTabs_);
  {
    auto *tabLayout = new QVBoxLayout(notesTab);
    auto *headerLayout = new QHBoxLayout();
    auto *newButton = new QPushButton("New");
    auto *openButton = new QPushButton("Open");
    auto *saveButton = new QPushButton("Save");
    auto *saveAsButton = new QPushButton("Save As");
    auto *copyButton = new QPushButton("Copy");
    auto *clearNotesOutputButton = new QPushButton("Clear Output");
    notesRunButton_ = new QPushButton("Run");
    notesRunButton_->setEnabled(false);
    notesSwitchButton_ = new QPushButton("Switch to Python");
    headerLayout->addWidget(newButton);
    headerLayout->addWidget(openButton);
    headerLayout->addWidget(saveButton);
    headerLayout->addWidget(saveAsButton);
    headerLayout->addWidget(copyButton);
    headerLayout->addWidget(clearNotesOutputButton);
    headerLayout->addWidget(notesRunButton_);
    headerLayout->addWidget(notesSwitchButton_);
    headerLayout->addStretch();
    tabLayout->addLayout(headerLayout);

    notesSplitter_ = new QSplitter(Qt::Horizontal, notesTab);
    notesEditor_ = new QPlainTextEdit(notesSplitter_);
    notesEditor_->setPlaceholderText("Write notes in Markdown...");
    notesPreview_ = new QTextBrowser(notesSplitter_);
    notesPreview_->setOpenExternalLinks(true);
    notesPreview_->setOpenLinks(false);
    notesSplitter_->addWidget(notesEditor_);
    notesSplitter_->addWidget(notesPreview_);
    notesSplitter_->setStretchFactor(0, 1);
    notesSplitter_->setStretchFactor(1, 1);
    tabLayout->addWidget(notesSplitter_);

    notesHighlighter_ = new PythonSyntaxHighlighter(
        notesEditor_->document(), [this](int pos) {
          return isNotesPythonPosition(pos);
        });
    auto *notesBracketPairer = new BracketPairer(
        notesEditor_, [this](int pos) { return isNotesCodePosition(pos); });
    notesEditor_->installEventFilter(notesBracketPairer);
    auto *notesBracketHighlighter = new BracketMatchHighlighter(
        notesEditor_, [this](int pos) { return isNotesCodePosition(pos); });
    Q_UNUSED(notesBracketHighlighter);

    auto makeNotesPath = []() {
      const QString sessionNumber =
          QString::number(QDateTime::currentSecsSinceEpoch());
      return QDir::currentPath() + "/cli_calc_notes_" + sessionNumber + ".md";
    };
    notesPath_ = makeNotesPath();

    auto refreshNotesActions = [this]() {
      if (notesRefreshInProgress_) {
        return;
      }
      notesRefreshInProgress_ = true;
      updateNotesRanges();
      if (notesSwitchButton_) {
        const bool isPython = isPythonNotesFile(notesPath_);
        notesSwitchButton_->setText(isPython ? "Switch to Markdown" : "Switch to Python");
      }
      if (notesRunButton_) {
        const QString text = notesEditor_ ? notesEditor_->toPlainText() : QString();
        const bool hasBlocks = !extractPythonBlocks(text).empty();
        const bool hasPython = hasBlocks || (isPythonNotesFile(notesPath_) && !text.trimmed().isEmpty());
        notesRunButton_->setEnabled(hasPython);
        if (!hasPython && !notesRunOutput_.isEmpty()) {
          notesRunOutput_.clear();
        }
      }
      if (notesHighlighter_) {
        notesHighlighter_->rehighlight();
      }
      notesRefreshInProgress_ = false;
    };

    connect(notesEditor_, &QPlainTextEdit::textChanged, this, [this, refreshNotesActions]() {
      updateNotesPreview();
      refreshNotesActions();
    });

    connect(notesPreview_, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
      const QString scheme = url.scheme();
      if (scheme != "copy" && scheme != "run") {
        QDesktopServices::openUrl(url);
        return;
      }
      int index = -1;
      if (!parseCodeBlockIndex(url, &index) ||
          index < 0 || index >= static_cast<int>(notesCodeBlocks_.size())) {
        statusBar()->showMessage("Invalid code block link", 2000);
        return;
      }
      const auto &block = notesCodeBlocks_[index];
      if (scheme == "copy") {
        QApplication::clipboard()->setText(block.code);
        statusBar()->showMessage("Code copied", 2000);
        return;
      }
      const QString lang = block.language.trimmed().toLower();
      if (!lang.isEmpty() && lang != "py" && lang != "python") {
        statusBar()->showMessage("Run supports Python blocks only", 3000);
        return;
      }
      const PythonRunResult runResult = runPythonSnippet(block.code);
      const bool success = runResult.started && runResult.exitCode == 0;
      if (output_) {
        output_->setPlainText(success ? "Python: Success" : "Python: Fail");
      }
      statusBar()->showMessage(
          success ? "Python run succeeded" : "Python run failed", 3000);
      if (!runResult.started) {
        notesRunOutput_.clear();
        notesCodeBlocks_[index].output = runResult.errorMessage;
      } else {
        const QString combined =
            (runResult.stdoutText + runResult.stderrText).trimmed();
        notesRunOutput_.clear();
        notesCodeBlocks_[index].output = combined.isEmpty() ? "No output." : combined;
      }
      updateNotesPreview();
    });
    updateNotesPreview();

    auto copyNotes = [this]() {
      QApplication::clipboard()->setText(notesEditor_->toPlainText());
      statusBar()->showMessage("Notes copied", 3000);
    };

    auto openNotes = [this, refreshNotesActions]() {
      QString path = QFileDialog::getOpenFileName(
          this, "Open Notes", QDir::currentPath(),
          "Markdown (*.md *.markdown);;Text Files (*.txt);;All Files (*)");
      if (path.isEmpty()) {
        return;
      }
      QFile file(path);
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to open notes", 3000);
        return;
      }
      notesEditor_->setPlainText(QString::fromUtf8(file.readAll()));
      notesPath_ = path;
      refreshNotesActions();
      updateNotesPreview();
      statusBar()->showMessage("Notes loaded", 3000);
    };

    auto saveNotes = [this]() {
      QFile file(notesPath_);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to save notes", 3000);
        return;
      }
      const QByteArray data = notesEditor_->toPlainText().toUtf8();
      file.write(data);
      statusBar()->showMessage("Notes saved", 3000);
    };

    auto saveNotesAs = [this]() {
      QString path = QFileDialog::getSaveFileName(
          this, "Save Notes As", notesPath_, "Markdown (*.md);;All Files (*)");
      if (path.isEmpty()) {
        return;
      }
      if (!path.endsWith(".md", Qt::CaseInsensitive)) {
        path += ".md";
      }
      notesPath_ = path;
      QFile file(notesPath_);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to save notes", 3000);
        return;
      }
      const QByteArray data = notesEditor_->toPlainText().toUtf8();
      file.write(data);
      statusBar()->showMessage("Notes saved", 3000);
    };

    auto newNotes = [this, makeNotesPath, refreshNotesActions]() {
      notesEditor_->clear();
      notesPath_ = makeNotesPath();
      refreshNotesActions();
      updateNotesPreview();
      statusBar()->showMessage("New notes created", 3000);
    };

    connect(copyButton, &QPushButton::clicked, this, [copyNotes]() {
      copyNotes();
    });

    connect(openButton, &QPushButton::clicked, this, [openNotes]() { openNotes(); });

    connect(saveButton, &QPushButton::clicked, this, [saveNotes]() { saveNotes(); });

    connect(saveAsButton, &QPushButton::clicked, this, [saveNotesAs]() { saveNotesAs(); });

    connect(newButton, &QPushButton::clicked, this, [newNotes]() { newNotes(); });

    connect(clearNotesOutputButton, &QPushButton::clicked, this, [this]() {
      notesRunOutput_.clear();
      for (auto &block : notesCodeBlocks_) {
        block.output.clear();
      }
      updateNotesPreview();
      statusBar()->showMessage("Notes output cleared", 3000);
    });

    if (notesSwitchButton_) {
      connect(notesSwitchButton_, &QPushButton::clicked, this,
              [this, makeNotesPath, refreshNotesActions]() {
                const bool isPython = isPythonNotesFile(notesPath_);
                notesEditor_->clear();
                notesPath_ = isPython ? makeNotesPath() : makePythonNotesPath();
                refreshNotesActions();
                updateNotesPreview();
                statusBar()->showMessage(
                    isPython ? "Switched to Markdown notes" : "Switched to Python notes", 3000);
              });
    }

    if (notesRunButton_) {
      connect(notesRunButton_, &QPushButton::clicked, this, [this]() {
        const QString text = notesEditor_->toPlainText();
        const auto blocks = extractPythonBlocks(text);
        QString code;
        if (!blocks.empty()) {
          for (size_t idx = 0; idx < blocks.size(); ++idx) {
            if (idx != 0) {
              code += "\n\n";
            }
            code += blocks[idx];
          }
        } else if (isPythonNotesFile(notesPath_)) {
          code = text;
        }
        if (code.trimmed().isEmpty()) {
          return;
        }

        const PythonRunResult runResult = runPythonSnippet(code);
        const bool success = runResult.started && runResult.exitCode == 0;
        if (output_) {
          output_->setPlainText(success ? "Python: Success" : "Python: Fail");
        }
        statusBar()->showMessage(
            success ? "Python run succeeded" : "Python run failed", 3000);
        if (!runResult.started) {
          notesRunOutput_ = runResult.errorMessage;
        } else {
          const QString combined =
              (runResult.stdoutText + runResult.stderrText).trimmed();
          notesRunOutput_ = combined.isEmpty() ? "No output." : combined;
        }
        for (auto &block : notesCodeBlocks_) {
          block.output.clear();
        }
        updateNotesPreview();
      });
    }

    refreshNotesActions();

    if (notesNewAction_) {
      connect(notesNewAction_, &QAction::triggered, this, [newNotes]() {
        newNotes();
      });
    }
    if (notesOpenAction_) {
      connect(notesOpenAction_, &QAction::triggered, this, [openNotes]() {
        openNotes();
      });
    }
    if (notesSaveAction_) {
      connect(notesSaveAction_, &QAction::triggered, this, [saveNotes]() {
        saveNotes();
      });
    }
    if (notesSaveAsAction_) {
      connect(notesSaveAsAction_, &QAction::triggered, this, [saveNotesAs]() {
        saveNotesAs();
      });
    }
    if (notesCopyAction_) {
      connect(notesCopyAction_, &QAction::triggered, this, [copyNotes]() {
        copyNotes();
      });
    }
  }
  registerTab("Notes", notesTab);

  for (const auto &entry : tabEntries_) {
    splitLeftSelect_->addItem(entry.title);
    splitRightSelect_->addItem(entry.title);
  }
  if (!tabEntries_.empty()) {
    splitLeftSelect_->setCurrentIndex(0);
    splitRightSelect_->setCurrentIndex(tabEntries_.size() > 1 ? 1 : 0);
  }

  output_ = new QPlainTextEdit();
  output_->setReadOnly(true);
  output_->setPlaceholderText("Output appears here");
  output_->setFont(devMonoFont());

  auto *outputGroup = new QVBoxLayout();
  auto *outputHeader = new QHBoxLayout();
  auto *outputLabel = new QLabel("Output");
  auto *copyButton = new QPushButton("Copy");
  auto *moreButton = new QToolButton();
  auto *moreMenu = new QMenu(moreButton);
  auto *copyJson = moreMenu->addAction("Copy as JSON");
  auto *copyYaml = moreMenu->addAction("Copy as YAML");
  auto *copyXml = moreMenu->addAction("Copy as XML");
  moreButton->setText("...");
  moreButton->setMenu(moreMenu);
  moreButton->setPopupMode(QToolButton::InstantPopup);
  outputHeader->addWidget(outputLabel);
  outputHeader->addStretch();
  outputHeader->addWidget(copyButton);
  outputHeader->addWidget(moreButton);
  outputGroup->addLayout(outputHeader);
  outputGroup->addWidget(output_);

  auto *outputContainer = new QWidget(central);
  outputContainer->setLayout(outputGroup);

  auto *mainPanel = new QWidget(central);
  auto *mainPanelLayout = new QVBoxLayout(mainPanel);
  mainPanelLayout->setContentsMargins(0, 0, 0, 0);
  mainPanelLayout->addWidget(splitBarContainer_);
  mainPanelLayout->addWidget(mainContentStack_);

  auto *splitter = new QSplitter(Qt::Vertical, central);
  splitter->addWidget(mainPanel);
  splitter->addWidget(outputContainer);
  splitter->setStretchFactor(0, 3);
  splitter->setStretchFactor(1, 1);
  layout->addWidget(splitter);
  setCentralWidget(central);

  statusBar()->showMessage("Ready");

  helpButton_ = new QToolButton(this);
  helpButton_->setText("?");
  helpButton_->setToolTip("Help");
  helpButton_->setAutoRaise(true);
  statusBar()->addWidget(helpButton_);
  updateHelpTooltip(mainTabs_->currentIndex());

  connect(mainTabs_, &QTabWidget::currentChanged, this,
          [this](int index) { updateHelpTooltip(index); });

  connect(splitLeftSelect_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int) {
            updateSplitStacks();
            updateHelpTooltip(mainTabs_->currentIndex());
          });

  connect(splitRightSelect_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int) { updateSplitStacks(); });

  connect(splitViewAction, &QAction::toggled, this,
          [this](bool enabled) { setSplitViewEnabled(enabled); });

  connect(toggleNotesEditorAction, &QAction::toggled, this, [this](bool enabled) {
    if (!notesEditor_ || !notesSplitter_) {
      return;
    }
    notesEditor_->setVisible(enabled);
    if (enabled) {
      notesSplitter_->setStretchFactor(0, 1);
      notesSplitter_->setStretchFactor(1, 1);
    } else {
      notesSplitter_->setStretchFactor(0, 0);
      notesSplitter_->setStretchFactor(1, 1);
    }
  });

  if (notesTogglePreviewAction_) {
    connect(notesTogglePreviewAction_, &QAction::toggled, this, [this](bool enabled) {
      if (!notesPreview_ || !notesSplitter_) {
        return;
      }
      notesPreview_->setVisible(enabled);
      if (enabled) {
        notesSplitter_->setStretchFactor(0, 1);
        notesSplitter_->setStretchFactor(1, 1);
      } else {
        notesSplitter_->setStretchFactor(0, 1);
        notesSplitter_->setStretchFactor(1, 0);
      }
    });
  }

  connect(clearOutputAction, &QAction::triggered, this, [this]() {
    output_->clear();
    statusBar()->showMessage("Output cleared", 3000);
  });

  connect(clearHistoryAction, &QAction::triggered, this, [this]() {
    if (exprHistory_) {
      exprHistory_->clear();
    }
    statusBar()->showMessage("Expression history cleared", 3000);
  });

  connect(exitAction, &QAction::triggered, this, [this]() { close(); });

  connect(evalStandardAction_, &QAction::triggered, this,
          [this]() { setEvalMode(EvalMode::Standard); });
  connect(evalBigIntAction_, &QAction::triggered, this,
          [this]() { setEvalMode(EvalMode::BigInt); });
  connect(evalBigDoubleAction_, &QAction::triggered, this,
          [this]() { setEvalMode(EvalMode::BigDouble); });

  connect(defaultBigIntAction, &QAction::toggled, this, [this](bool checked) {
    if (checked) {
      setEvalMode(EvalMode::BigInt);
    } else if (evalMode_ == EvalMode::BigInt) {
      setEvalMode(EvalMode::Standard);
    }
  });

  connect(wrapOutputAction, &QAction::toggled, this, [this](bool checked) {
    output_->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                    : QPlainTextEdit::NoWrap);
  });

  connect(matrixEditorAction, &QAction::toggled, this, [this](bool checked) {
    matrixInputStack_->setCurrentIndex(checked ? 0 : 1);
    statusBar()->showMessage(
        checked ? "Matrix editor enabled" : "Matrix editor disabled", 3000);
  });

  connect(desktopModeAction, &QAction::triggered, this, [this, desktopModeAction, touchModeAction]() {
    desktopModeAction->setChecked(true);
    touchModeAction->setChecked(false);
    statusBar()->showMessage("Mode: Desktop", 3000);
  });

  connect(touchModeAction, &QAction::triggered, this, [this, desktopModeAction, touchModeAction]() {
    touchModeAction->setChecked(true);
    desktopModeAction->setChecked(false);
    statusBar()->showMessage("Mode: Touchscreen", 3000);
  });

  connect(aboutAction, &QAction::triggered, this, [this]() {
    QMessageBox::information(
        this, "About",
        "CLI Calculator\n GitHub: https://github.com/benedek553/cli-calculator\n\n");
  });

  connect(contributeAction, &QAction::triggered, this, []() {
    QDesktopServices::openUrl(QUrl("https://github.com/benedek553/cli-calculator"));
  });

  connect(reportBugAction, &QAction::triggered, this, []() {
    QDesktopServices::openUrl(QUrl("https://github.com/Benedek553/cli-calculator/issues/new?template=bug_report.yml"));
  });

  connect(requestFeatureAction, &QAction::triggered, this, []() {
    QDesktopServices::openUrl(QUrl("https://github.com/Benedek553/cli-calculator/issues/new?template=feature_request.yml"));
  });

  connect(copyButton, &QPushButton::clicked, this, [this]() {
    const QString text = output_->toPlainText();
    QApplication::clipboard()->setText(text);
    statusBar()->showMessage("Copied output", 3000);
  });

  connect(copyJson, &QAction::triggered, this,
          [this]() { copyStructured(OutputFormat::Json, "Copy as JSON"); });
  connect(copyYaml, &QAction::triggered, this,
          [this]() { copyStructured(OutputFormat::Yaml, "Copy as YAML"); });
  connect(copyXml, &QAction::triggered, this,
          [this]() { copyStructured(OutputFormat::Xml, "Copy as XML"); });
}
