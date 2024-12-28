#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector3D>
#include "tetrisengine.h"
#include <memory>
#include <QtCore>
#include <QKeyEvent>
#include <QEvent>
#include <string>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Game
    void setCurrentScore(int score);
    void setBestScore(int score);
    void startStopEngine();
    void newGame();
    void newGameAfterGameOver();

    // Settings
    void updateRoundingLabel(int value);
    void updateApproxEdgesLabel(int value);
    void updateSettings();

private:
    void setShortcuts();
    void createShortcut(Qt::Key key, std::function<void()> slot);
    void getSettings(Settings &settings);

    int showMessage(QMessageBox::Icon type, QString title, QString msg);

    TetrisEngine *engine;
    Scene *scene;
    Ui::MainWindow *ui;
    Settings settings;
    bool gameOver;
};
#endif // MAINWINDOW_H
