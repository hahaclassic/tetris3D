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
    void gameOverEvent();

private slots:
    void updateScene();
    void dropActiveBlock();
    void updateFPS();
    void rotateField(const QPointF& delta);
    // void scaleField(const int& delta);

private:
    int currentScore, bestScore;

    GameField field;          
    std::shared_ptr<Block> activeBlock;       
    std::shared_ptr<Block> activeBlockProjection; 
    std::shared_ptr<Renderer> renderer;       

    Cube baseCube;
    float projTransparency;

    float dropSpeed;
    float renderSpeed;
    bool isRunning, gameOver; 
    int fps;

    QTimer* renderTimer;    
    QTimer* dropTimer;     
    QTimer* fpsTimer;

    // setup
    void setUpField(const Settings& settings, float cellSize);
    void setUpCube(const Settings& settings, float cellSize);
    void setUpLight(const int& lightPos);

    // block
    bool canMoveBlock(const QVector3D& offset) const;
    bool canRotateBlock(const QVector3D& axis, bool clockwise) const;
    bool placeActiveBlock();                   

    bool generateNewBlock();
    void generateRandomBlock(QVector3D centerPosition);

    void moveActiveBlock(int dx, int dy, int dz);
    void rotateActiveBlock(const QVector3D& axis, bool clockwise);
    void updateProjectionPosition();

    int loadBestScoreFromFile(const std::string& filename);
    void saveBestScoreToFile(const std::string& filename);
};

#endif // TETRISENGINE_H
