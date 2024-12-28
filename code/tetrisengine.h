#ifndef TETRISENGINE_H
#define TETRISENGINE_H

#include <stdexcept>
#include <chrono>
#include <thread>
#include <iostream>
#include "renderer.h"
#include "gamefield.h"
#include "block.h"
#include <memory>
#include <gamefield.h>
#include <QTimer>
#include <random>
#include <ctime>
#include <QShortcut>
#include <QKeySequence>
#include <fstream>
#include <scene.h>

#define SCORE_FILE_NAME "best_score.txt"

struct Settings {
    int approxEdges;
    int roundingPercent;
    float projectionTransparency;
    int fieldWidth;
    int fieldDepth;
    int fieldHeight;
    bool withShadows;
    int lightPos; // 0 - сверху сбоку, 1 - сверху, 2 - сбоку
};


class TetrisEngine : public QObject {
    Q_OBJECT
public:
    // Конструктор
    TetrisEngine(std::shared_ptr<Renderer> renderer, Scene *scene);
    ~TetrisEngine();

    void setUp(const Settings& settings);
    void run();
    void stop();
    void newGame();
    void keyPressHandler(int key);

signals:
    void currentScoreUpdateEvent(int score);
    void bestScoreUpdateEvent(int score);
    // void stopEvent();
    void gameOverEvent();

private slots:
    // Основной метод обновления
    void updateScene();
    void dropActiveBlock();
    void updateFPS();
    void rotateField(const QPointF& delta);
    // void scaleField(const int& delta);

private:
    int currentScore, bestScore;

    GameField field;          // Игровое пространство
    std::shared_ptr<Block> activeBlock;        // Текущий активный блок
    std::shared_ptr<Block> activeBlockProjection; // Текущий активный блок
    std::shared_ptr<Renderer> renderer;        // Рендерер сцены

    Cube baseCube;
    float projTransparency;

    float dropSpeed;
    float renderSpeed;
    bool isRunning, gameOver; // Флаг работы игрового цикла
    int fps;

    QTimer* renderTimer;     // Таймер для рендеринга
    QTimer* dropTimer;       // Таймер для падения блока
    QTimer* fpsTimer;

    // setup
    void setUpField(const Settings& settings, float cellSize);
    void setUpCube(const Settings& settings, float cellSize);
    void setUpLight(const int& lightPos);

    // block
    bool canMoveBlock(const QVector3D& offset) const;
    bool canRotateBlock(const QVector3D& axis, bool clockwise) const;
    bool placeActiveBlock();                   // Поставить блок

    bool generateNewBlock();
    void generateRandomBlock(QVector3D centerPosition);

    void moveActiveBlock(int dx, int dy, int dz);
    void rotateActiveBlock(const QVector3D& axis, bool clockwise);
    void updateProjectionPosition();

    // best score loading
    int loadBestScoreFromFile(const std::string& filename);
    void saveBestScoreToFile(const std::string& filename);

    void measureRenderTimeCubes(int numCubesStart, int numCubesEnd, int step);

    std::vector<Cube> generateTestCubes(int numCubes, float edge, float rounding, int approx);

    void measureRenderTimeEdges(int numEdgesStart, int numEdgesEnd, int step);
};

#endif // TETRISENGINE_H
