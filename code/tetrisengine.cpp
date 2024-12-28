#include "tetrisengine.h"

// Конструктор
TetrisEngine::TetrisEngine(std::shared_ptr<Renderer> renderer, Scene *scene)
    : renderer(std::move(renderer)), dropSpeed(20.0f), renderSpeed(16), isRunning(false), gameOver(false) {
    if (!this->renderer) {
        throw std::invalid_argument("GameField and Renderer cannot be null");
    }
    connect(scene, &Scene::mouseMoveDeltaEvent, this, &TetrisEngine::rotateField);
    //connect(scene, &Scene::wheelScrolledEvent, this, &TetrisEngine::scaleField);

    renderTimer = new QTimer(this);
    connect(renderTimer, &QTimer::timeout, this, &TetrisEngine::updateScene);

    dropTimer = new QTimer(this);
    connect(dropTimer, &QTimer::timeout, this, &TetrisEngine::dropActiveBlock);

    fpsTimer = new QTimer(this);
    connect(fpsTimer, &QTimer::timeout, this, &TetrisEngine::updateFPS);

    currentScore = 0;
    emit currentScoreUpdateEvent(currentScore);
    bestScore = loadBestScoreFromFile(SCORE_FILE_NAME);

    fps = 0;
}

TetrisEngine::~TetrisEngine() {
    isRunning = false;
    saveBestScoreToFile(SCORE_FILE_NAME);

    stop();
    renderTimer->stop();
    fpsTimer->stop();

    activeBlock.reset();
    renderer.reset();

    delete renderTimer;
    delete dropTimer;
    delete fpsTimer;

    qDebug() << "TetrisEngine destroyed.";
}

void TetrisEngine::setUp(const Settings& settings) {
    emit bestScoreUpdateEvent(bestScore);

    renderer->enableShadows(settings.withShadows);

    float cellSize = 50;
    setUpCube(settings, cellSize);

    if (field.getWidth() != settings.fieldWidth || field.getDepth() != settings.fieldDepth
            || field.getHeight() != settings.fieldHeight) {
        gameOver = true;
        setUpField(settings, cellSize);
        generateNewBlock();
    } else {
//        auto cubes = field.getAllCubes();
//        for (auto& cube : cubes) {
//            cube->setRounding(rounding);
//            cube->generateMesh(settings.approxEdges);
//        }
//        for (auto& cube : *activeBlock.get()) {
//            cube->setRounding(rounding);
//            cube->generateMesh(settings.approxEdges);
//            qDebug() << cube->getColor();
//        }
//        for (auto& cube : *activeBlockProjection.get()) {
//            cube->setRounding(rounding);
//            cube->generateMesh(settings.approxEdges);
//        }
    }

    setUpLight(settings.lightPos);

    updateProjectionPosition();
    renderTimer->start(renderSpeed);
    fpsTimer->start(1000);
}

void TetrisEngine::setUpCube(const Settings& settings, float cellSize) {
    projTransparency = settings.projectionTransparency;
    double rounding = settings.roundingPercent * 0.01 * cellSize / 2;
    baseCube = Cube(QVector3D(0, 0, 0), cellSize, rounding, QColor());
    baseCube.generateMesh(settings.approxEdges);
}

void TetrisEngine::setUpField(const Settings& settings, float cellSize) {
    QColor fieldColor = QColor(255,255,255); // QColor(244, 244, 244); //QColor(209, 164, 235);
    field = GameField({0, 0, 0}, settings.fieldWidth,
                      settings.fieldDepth, settings.fieldHeight,
                      cellSize, fieldColor, 1.0f);

    switch (settings.fieldHeight) {
    case 7:
        renderer->setCameraPos({400.0f, 600.0f, 800.0f});
        break;
    case 8:
        renderer->setCameraPos({466.0f, 700.0f, 850.0f});
        break;
    case 9:
        renderer->setCameraPos({533.0f, 800.0f, 800.0f});
        break;
    case 10:
        renderer->setCameraPos({600.0f, 900.0f, 900.0f});
        break;
    }
}

void TetrisEngine::setUpLight(const int& lightPos) {
    switch (lightPos) {
    case 0:
        renderer->setLightPos({700.0f, 200.0f, 800.0f});
        break;
    case 1:
        renderer->setLightPos({700.0f, 200.0f, 300});
        break;
    case 2:
        renderer->setLightPos({150, 150, 800});
        break;
    }
}

void TetrisEngine::run() {
    if (gameOver) {
        newGame();
    }

    updateProjectionPosition();
    isRunning = true;
    renderTimer->start(renderSpeed);
    dropTimer->start(static_cast<int>(1000 / dropSpeed));
    fpsTimer->start(1000);
}

void TetrisEngine::stop() {
    isRunning = false;
    dropTimer->stop();
    //renderTimer->stop();
    //fpsTimer->stop();
}

void TetrisEngine::newGame() {
    stop();
    field.clear();
    gameOver = false;

    currentScore = 0;
    emit currentScoreUpdateEvent(currentScore);

    generateNewBlock();
    updateProjectionPosition();
}

// Основной метод обновления
void TetrisEngine::updateScene() {
//    qDebug() << (activeBlock.get())->getCenter();
//    qDebug() << activeBlockProjection.get()->getCenter();
    renderer->renderScene(field, *activeBlock.get(), *activeBlockProjection.get());
    fps++;
}

void TetrisEngine::dropActiveBlock() {
    if (canMoveBlock({0, 0, -field.getCellSize() / dropSpeed})) {
        activeBlock->move({0, 0, -field.getCellSize() / dropSpeed});
        //updateProjectionPosition();
        return;
    }

    bool ok = placeActiveBlock();
    if (!ok) {
        isRunning = false;
        gameOver = true;
        stop();
        emit gameOverEvent();
        return;
    }

    int countRemovedLayers = field.removeFullLayers();
    currentScore += countRemovedLayers * countRemovedLayers;
    emit currentScoreUpdateEvent(currentScore);
    if (currentScore > bestScore) {
        bestScore = currentScore;
        emit bestScoreUpdateEvent(bestScore);
    }

    ok = generateNewBlock();
    if (!ok) {
        updateScene();
        isRunning = false;
        gameOver = true;
        stop();
        emit gameOverEvent();
    }

    updateProjectionPosition();
}

void TetrisEngine::updateFPS() {
    qDebug() << "fps" << fps;
    fps = 0;
}


// Перемещение активного блока
void TetrisEngine::moveActiveBlock(int dx, int dy, int dz) {
    qDebug() << "move";
    float cellSize = field.getCellSize();
    if (dz < 0) {
        dz = 0;
        while (canMoveBlock({0, 0, (dz - 1) * cellSize})) {
            dz--;
        }
    }

    QVector3D offset(dx * cellSize, dy * cellSize, dz * cellSize);
    if (canMoveBlock(offset)) {
        activeBlock->move(offset);
        updateProjectionPosition();
    }
}

// Вращение активного блока
void TetrisEngine::rotateActiveBlock(const QVector3D& axis, bool clockwise) {
    // Убедимся, что вращение возможно
    if (!canRotateBlock(axis, clockwise)) {
        qDebug() << axis << clockwise;
        return;
    }

    QMatrix4x4 rotationMatrix;
    float angle = clockwise ? 90.0f : -90.0f;
    rotationMatrix.rotate(angle, axis);

    activeBlock->rotate(rotationMatrix);
    updateProjectionPosition();
}

// Проверка возможности перемещения блока
bool TetrisEngine::canMoveBlock(const QVector3D& offset) const {
    for (auto cube = activeBlock->begin(); cube != activeBlock->end(); ++cube) {

        auto globalPos = (cube->get())->getPosition() + offset;
        if ((globalPos - field.getPosition()).z() < field.getCellSize()/2) {
            return false;
        }

        auto pos = field.convertToPosInField(globalPos);

        if (!field.isPositionValid(pos[0], pos[1], pos[2]) || !field.isPositionEmpty(pos[0], pos[1], pos[2])) {
            return false;
        }
    }

    return true;
}

// Проверка возможности вращения блока
bool TetrisEngine::canRotateBlock(const QVector3D& axis, bool clockwise) const {
    QMatrix4x4 rotationMatrix;
    float angle = clockwise ? 90.0f : -90.0f;
    rotationMatrix.rotate(angle, axis);

    // Проверка каждой позиции кубов после вращения
    for (const auto& cube : *activeBlock.get()) {
        QVector3D relativePosition = cube->getPosition() - activeBlock->getCenter();
        QVector3D rotatedPosition = rotationMatrix.map(relativePosition);
        QVector3D newPosition = activeBlock->getCenter() + rotatedPosition;

        auto pos = field.convertToPosInField(newPosition);

        if (!field.isPositionValid(pos[0], pos[1], pos[2]) ||
            !field.isPositionEmpty(pos[0], pos[1], pos[2])) {
            return false;
        }
    }

    return true;
}


bool TetrisEngine::placeActiveBlock() {
    for (const auto& cube : *activeBlock.get()) {
        auto pos = field.convertToPosInField(cube->getPosition());
        bool ok = field.addCube(pos[0], pos[1], pos[2], cube);
        if (!ok || pos[2] == field.getHeight() - 1)
            return false;
    }

    activeBlock.reset();
    activeBlockProjection.reset();
    return true;
}

void TetrisEngine::updateProjectionPosition() {
    int heightDelta = 0;
    float cellSize = field.getCellSize();
    for (; heightDelta < field.getHeight() && canMoveBlock({0, 0, -(heightDelta+1) * cellSize}); heightDelta++) {}

    activeBlockProjection->clear();
    QVector3D offset = QVector3D(0, 0, heightDelta * cellSize);
    if (offset.z() < cellSize) {
        return;
    }

    for (const auto& cube : *activeBlock.get()) {
        QVector3D globalPos = cube->getPosition() - offset;
        float remainsZ = std::fmod(globalPos.z() - cellSize/2, cellSize);
        if (remainsZ > 0) {
            globalPos.setZ(globalPos.z() - remainsZ);
        }

        Cube newCube = Cube(*cube.get(), globalPos);
        QColor color = cube->getColor();
        color.setAlphaF(projTransparency);
        newCube.setColor(color);
        activeBlockProjection->addCube(std::make_shared<Cube>(newCube));
    }
}

void TetrisEngine::rotateField(const QPointF& delta) {
    renderer->rotateModelMatrix(delta);
    updateProjectionPosition();
    //updateScene();
}

//void TetrisEngine::scaleField(const int& delta) {
//    renderer->scaleModelMatrix(delta);
//    updateProjectionPosition();
//}


bool TetrisEngine::generateNewBlock() {
    float cellSize = field.getCellSize();
    QVector3D position((field.getWidth() / 2) * field.getCellSize() - cellSize/2,
                       (field.getDepth() / 2) * field.getCellSize() - cellSize/2,
                       field.getCellSize() * field.getHeight() - cellSize/2);
    generateRandomBlock(position + field.getPosition());

    return canMoveBlock({0, 0, 0});
}

int getRandomNumber(int min, int max) {
    static std::mt19937 generator(static_cast<unsigned>(std::time(nullptr))); // Инициализация генератора
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

void TetrisEngine::generateRandomBlock(QVector3D centerPosition)
{
    float edgeLength = field.getCellSize();
    if (activeBlock != nullptr) {
        activeBlock->clear();
        activeBlockProjection->clear();
    } else {
        activeBlock = std::make_shared<Block>(Block(centerPosition));
        activeBlockProjection = std::make_shared<Block>(Block(centerPosition));
    }

    std::vector<QColor> colors = {
        QColor(166, 189, 157),
        QColor(144, 158, 197),
        QColor(192,124,116),
        QColor(112,198,210),
        QColor(245, 212, 25),
        QColor(67, 175, 70),
        QColor(236, 175, 209),
        QColor(238, 221, 175),
    };

    std::vector<QVector3D> centers = {
        centerPosition,
        centerPosition + QVector3D(edgeLength/2, edgeLength/2, edgeLength/2),
        centerPosition,
        centerPosition,
        centerPosition + QVector3D(edgeLength/2, edgeLength/2, edgeLength/2),
        centerPosition + QVector3D(edgeLength/2, edgeLength/2, edgeLength/2),
        centerPosition + QVector3D(edgeLength/2, edgeLength/2, edgeLength/2),
        centerPosition - QVector3D(edgeLength/2, edgeLength/2, edgeLength/2),
    };

    std::vector<std::vector<Cube>> cubes = {
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(0, edgeLength, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition - QVector3D(edgeLength, 0, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition - QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, edgeLength, 0))
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, edgeLength, 0)),
            Cube(baseCube, centerPosition + QVector3D(0, edgeLength, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(0, edgeLength, 0)),
            Cube(baseCube, centerPosition + QVector3D(-edgeLength, edgeLength, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition - QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(2 * edgeLength, 0, 0)),
        },
        {
            Cube(baseCube, centerPosition),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, 0)),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, edgeLength, 0)),
            Cube(baseCube, centerPosition + QVector3D(0, edgeLength, 0)),
            Cube(baseCube, centerPosition + QVector3D(0, 0, -edgeLength)),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, 0, -edgeLength)),
            Cube(baseCube, centerPosition + QVector3D(edgeLength, edgeLength, -edgeLength)),
            Cube(baseCube, centerPosition + QVector3D(0, edgeLength, -edgeLength)),
        },
    };

    int cubesIdx = getRandomNumber(0, cubes.size() - 1);
    int colorIdx = getRandomNumber(0, colors.size() - 1);
    int maxHeight = 0;

    for (auto& cube : cubes[cubesIdx]) {
        cube.setColor(colors[colorIdx]);
        activeBlock->addCube(std::make_shared<Cube>(cube));
        activeBlock->setCenter(centers[cubesIdx]);

        QColor color = colors[colorIdx];
        color.setAlphaF(projTransparency);

        Cube projectionCube = Cube(cube, cube.getPosition());
        projectionCube.setColor(color);
        activeBlockProjection->addCube(std::make_shared<Cube>(projectionCube));
    }
}

void TetrisEngine::keyPressHandler(int key) {
    if (!isRunning) {
        return;
    }

    switch (key) {
        case Qt::Key_Left:
            moveActiveBlock(1, 0, 0);
            break;
        case Qt::Key_Right:
            moveActiveBlock(-1, 0, 0);
            break;
        case Qt::Key_Up:
            moveActiveBlock(0, -1, 0);
            break;
        case Qt::Key_Down:
            moveActiveBlock(0, 1, 0);
            break;
        case Qt::Key_Space:
            moveActiveBlock(0, 0, -1);
            break;

        // Управление вращением
        case Qt::Key_W: // Вращение вокруг оси X (вперед)
            rotateActiveBlock(QVector3D(1.0f, 0.0f, 0.0f), true);
            break;
        case Qt::Key_S: // Вращение вокруг оси X (назад)
            rotateActiveBlock(QVector3D(1.0f, 0.0f, 0.0f), false);
            break;
        case Qt::Key_Q: // Вращение вокруг оси Z (против часовой стрелки)
            rotateActiveBlock(QVector3D(0.0f, 1.0f, 0.0f), true);
            break;
        case Qt::Key_E: // Вращение вокруг оси Z (по часовой стрелке)
            rotateActiveBlock(QVector3D(0.0f, 1.0f, 0.0f), false);
            break;
        case Qt::Key_A: // Вращение вокруг оси Y (влево)
            rotateActiveBlock(QVector3D(0.0f, 0.0f, 1.0f), true);
            break;
        case Qt::Key_D: // Вращение вокруг оси Y (вправо)
            rotateActiveBlock(QVector3D(0.0f, 0.0f, 1.0f), false);
            break;
    }
}

int TetrisEngine::loadBestScoreFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (file.is_open()) {
        int score;
        file >> score; // Чтение значения bestScore
        file.close();
        return score;
    }

    return 0;
}

void TetrisEngine::saveBestScoreToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << bestScore; // Запись лучшего счёта в файл
        file.close();
    }
}

void TetrisEngine::measureRenderTimeCubes(int numCubesStart, int numCubesEnd, int step) {
    qDebug() << "Starting render time measurement (cubes)...";
    int nTests = 20;

    for (int numCubes = numCubesStart; numCubes <= numCubesEnd; numCubes += step) {
        long long duration = 0;
        for (int i = 0; i < nTests; i++) {
            auto cubes = generateTestCubes(numCubes, 50, 10, 5);
            auto start = std::chrono::high_resolution_clock::now();
            renderer->renderCubes(cubes);
            auto end = std::chrono::high_resolution_clock::now();

            duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        qDebug() << "Number of cubes:" << numCubes << "Render time (ms):" << duration / nTests;
    }
}

std::vector<Cube> TetrisEngine::generateTestCubes(int numCubes, float edge, float rounding, int approx) {
    QColor testColor = QColor(255, 0, 0);

    std::vector<Cube> cubes;

    for (int i = 0; i < numCubes; ++i) {
        int x = i % field.getWidth();
        int y = (i / field.getWidth()) % field.getDepth();
        int z = (i / (field.getWidth() * field.getDepth())) % field.getHeight();

        QVector3D position(x * edge, y * edge, z * edge);
        auto cube = Cube(position, edge, rounding, testColor);
        cube.generateMesh(approx);
        cubes.push_back(cube);
    }

    return cubes;
}

void TetrisEngine::measureRenderTimeEdges(int numEdgesStart, int numEdgesEnd, int step) {
    qDebug() << "Starting render time measurement (edges)...";
    int nTests = 20;

    for (int numEdges = numEdgesStart; numEdges <= numEdgesEnd; numEdges += step) {
        long long duration;
        for (int i = 0; i < nTests; i++) {
            auto cubes = generateTestCubes(100, 50, 10, numEdges);
            auto start = std::chrono::high_resolution_clock::now();
            renderer->renderCubes(cubes);
            auto end = std::chrono::high_resolution_clock::now();

            duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        qDebug() << "Number of edges:" << numEdges << "Render time (ms):" << duration / nTests;
    }
}

