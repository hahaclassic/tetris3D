#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), gameOver(true)
{
    ui->setupUi(this);

    scene = new Scene();

    scene->setSceneRect(0, 0, 790, 742);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
//    ui->graphicsView->setFocusPolicy(Qt::NoFocus);
//    ui->centralwidget->setFocusPolicy(Qt::NoFocus);
//    setFocusPolicy(Qt::StrongFocus);
//    setFocus();

    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(scene);
    engine = new TetrisEngine(renderer, scene);

    connect(ui->roundingSlider, &QSlider::valueChanged, this, &MainWindow::updateRoundingLabel);
    connect(ui->approxEdgesSlider, &QSlider::valueChanged, this, &MainWindow::updateApproxEdgesLabel);

    connect(ui->startStopBtn, &QPushButton::clicked, this, &MainWindow::startStopEngine);
    connect(ui->newGameBtn, &QPushButton::clicked, this, &MainWindow::newGame);
    connect(ui->setSettingsBtn, &QPushButton::clicked, this, &MainWindow::updateSettings);

    connect(engine, &TetrisEngine::gameOverEvent, this, &MainWindow::newGameAfterGameOver);
    connect(engine, &TetrisEngine::currentScoreUpdateEvent, this, &MainWindow::setCurrentScore);
    connect(engine, &TetrisEngine::bestScoreUpdateEvent, this, &MainWindow::setBestScore);

    getSettings(settings);
    engine->setUp(settings);
    setShortcuts();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete engine;
}

void MainWindow::setCurrentScore(int score) {
    ui->currentScore->setText(QString::number(score));
}

void MainWindow::setBestScore(int score) {
    ui->bestScore->setText(QString::number(score));
}

void MainWindow::newGame() {
    gameOver = false;
    ui->startStopBtn->setText("Start");
    engine->newGame();
}

void MainWindow::newGameAfterGameOver() {
    ui->startStopBtn->setText("Start");
    gameOver = true;
    QString msg = "Игра окончена! Ваш счёт: " + ui->currentScore->text();
    if (ui->currentScore->text() == ui->bestScore->text())
        msg += "\nНовый рекорд!";
    showMessage(QMessageBox::Information, "Game over", msg);
}

void MainWindow::startStopEngine() {
    gameOver = false;
    QString currState = ui->startStopBtn->text();
    if (currState == "Start" || currState == "Continue") {
        engine->run();
        ui->startStopBtn->setText("Stop");
    } else {
        engine->stop();
        ui->startStopBtn->setText("Continue");
    }
}

void MainWindow::updateRoundingLabel(int num) {
    ui->rounding->setText(QString::number(num));
}

void MainWindow::updateApproxEdgesLabel(int value) {
    ui->approxEdges->setText(QString::number(value));
}

void MainWindow::getSettings(Settings &newSettings) {
    newSettings.approxEdges = ui->approxEdges->text().toInt();
    newSettings.roundingPercent = ui->rounding->text().toInt();
    newSettings.projectionTransparency = ui->blockTransparency->value();

    newSettings.fieldWidth = ui->width->currentText().toInt();
    newSettings.fieldDepth = ui->depth->currentText().toInt();
    newSettings.fieldHeight = ui->height->currentText().toInt();

    newSettings.withShadows = (ui->withShadows->checkState() == Qt::Checked);

    newSettings.lightPos = ui->lightPos->currentIndex();
}

void MainWindow::updateSettings() {
    Settings newSettings;
    getSettings(newSettings);

    if (!gameOver && (settings.fieldDepth != newSettings.fieldDepth ||
            settings.fieldHeight != newSettings.fieldHeight ||
            settings.fieldWidth != newSettings.fieldWidth)) {
        int answer = showMessage(QMessageBox::Warning, "Внимание!",
            "При изменении размеров игрового поля текущий прогресс сбросится. Продолжить?");
        if (answer == QMessageBox::Ok) {
            gameOver = false;
            ui->startStopBtn->setText("Start");
        } else {
            return;
        }
    }

    settings = newSettings;
    engine->setUp(newSettings);
}

int MainWindow::showMessage(QMessageBox::Icon type, QString title, QString msg) {
    QMessageBox msgBox;

    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(msg);

    if (type == QMessageBox::Information)
        msgBox.setStandardButtons(QMessageBox::Ok);
    else
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);

    return msgBox.exec();
}


void MainWindow::createShortcut(Qt::Key key, std::function<void()> slot) {
    QShortcut* shortcut = new QShortcut(QKeySequence(key), this);
    connect(shortcut, &QShortcut::activated, this, slot);
}

void MainWindow::setShortcuts() {
    createShortcut(Qt::Key_Left, [=]() { engine->keyPressHandler(Qt::Key_Left); });
    createShortcut(Qt::Key_Right, [=]() { engine->keyPressHandler(Qt::Key_Right); });
    createShortcut(Qt::Key_Up, [=]() { engine->keyPressHandler(Qt::Key_Up); });
    createShortcut(Qt::Key_Down, [=]() { engine->keyPressHandler(Qt::Key_Down); });
    createShortcut(Qt::Key_Space, [=]() { engine->keyPressHandler(Qt::Key_Space); });

    createShortcut(Qt::Key_W, [=]() { engine->keyPressHandler(Qt::Key_W); });
    createShortcut(Qt::Key_S, [=]() { engine->keyPressHandler(Qt::Key_S); });
    createShortcut(Qt::Key_Q, [=]() { engine->keyPressHandler(Qt::Key_Q); });
    createShortcut(Qt::Key_E, [=]() { engine->keyPressHandler(Qt::Key_E); });
    createShortcut(Qt::Key_A, [=]() { engine->keyPressHandler(Qt::Key_A); });
    createShortcut(Qt::Key_D, [=]() { engine->keyPressHandler(Qt::Key_D); });
}
