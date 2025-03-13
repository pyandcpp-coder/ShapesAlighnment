#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QList>
#include <QtWidgets/qgraphicsitem.h>
#include <QRandomGenerator>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), nfpCalc(nullptr)
{
    scene = new myscene(this);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setSceneRect(-200, -200, 400, 400); // Initial scene size
    view->setAcceptDrops(true);

    scaleSpinBox = new QDoubleSpinBox;
    scaleSpinBox->setRange(0.1, 10.0);
    scaleSpinBox->setValue(1.0); // default scale is 100 percent
    scaleSpinBox->setSingleStep(0.1); // increment by 10 percent

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout;
    ShapeButton *rectButton = new ShapeButton("Rectangle");
    ShapeButton *ellipseButton = new ShapeButton("Ellipse");
    ShapeButton *triangleButton = new ShapeButton("Triangle");
    ShapeButton *squareButton = new ShapeButton("Square");
    ShapeButton *cButton = new ShapeButton("C");
    ShapeButton *starButton = new ShapeButton("Star");
    ShapeButton *clearButton = new ShapeButton("Clear");
    ShapeButton *curveButton = new ShapeButton("Curve C");



    leftLayout->addWidget(clearButton);
    leftLayout->addWidget(curveButton);

    leftLayout->addWidget(cButton);
    leftLayout->addWidget(starButton);

    leftLayout->addWidget(rectButton);
    leftLayout->addWidget(ellipseButton);
    leftLayout->addWidget(triangleButton);
    leftLayout->addWidget(squareButton);
    leftLayout->addWidget(scaleSpinBox);


    // Add Arrange Shapes button
    QPushButton *arrangeButton = new QPushButton("Arrange Shapes");
    leftLayout->addWidget(arrangeButton);
    connect(arrangeButton, &QPushButton::clicked, this, &MainWindow::arrangeShapes);

    connect(scaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onScaleChanged);
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(view);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow()
{
    delete nfpCalc; // Clean up NFP calculator
    delete scene;
}

void MainWindow::onSelectionChanged()
{
    QList<QGraphicsItem*> selected = scene->selectedItems();
    if (!selected.isEmpty()) {
        selectedItem = selected.first();
        scaleSpinBox->setValue(selectedItem->scale());
    } else {
        selectedItem = nullptr;
    }
}

void MainWindow::onScaleChanged(double value)
{
    if (selectedItem) {
        // Set the transform origin to the center of the item
        QRectF bounds = selectedItem->boundingRect();
        selectedItem->setTransformOriginPoint(bounds.center());
        // Apply the scale
        selectedItem->setScale(value);
        // Force the scene to update to reflect the change
        scene->update();
        qDebug() << "Scaled item to" << value;
    }
}

void MainWindow::arrangeShapes()
{
    // Collect all movable shapes as QGraphicsPolygonItem*
    QList<QGraphicsPolygonItem*> polygonShapes;
    for (QGraphicsItem* item : scene->items()) {
        if (QGraphicsPolygonItem* polyItem = dynamic_cast<QGraphicsPolygonItem*>(item)) {
            if (polyItem->flags() & QGraphicsItem::ItemIsMovable) {
                polygonShapes.append(polyItem);
            }
        }
    }

    if (polygonShapes.isEmpty()) {
        qDebug() << "No movable shapes to arrange.";
        return;
    }

    // Clean up previous NFP calculator instance
    delete nfpCalc;
    nfpCalc = new nfpcalculator(polygonShapes);

    // Initial placement: first shape at origin, others unchanged
    if (!polygonShapes.isEmpty()) {
        polygonShapes[0]->setPos(0, 0);
        polygonShapes[0]->setRotation(0);
    }

    // Store best arrangement
    qreal bestCost = computeCost(polygonShapes);
    QMap<QGraphicsItem*, QPointF> bestPositions;
    QMap<QGraphicsItem*, qreal> bestRotations;
    for (QGraphicsItem* shape : polygonShapes) {
        bestPositions[shape] = shape->pos();
        bestRotations[shape] = shape->rotation();
    }

    // Simulated Annealing parameters (adjusted for more shapes)
    qreal T = 200.0;            // Increased initial temperature for more exploration
    qreal coolingRate = 0.95;   // Cooling rate
    int iterationsPerTemp = 150; // Increased iterations for better optimization
    qreal minT = 0.01;          // Stopping temperature
    QList<qreal> rotationAngles = {0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165,
                                   180, 195, 210, 225, 240, 255, 270, 285, 300, 315, 330, 345};

    // Main Simulated Annealing loop
    while (T > minT) {
        for (int iter = 0; iter < iterationsPerTemp; ++iter) {
            // Skip the first shape (fixed at origin)
            if (polygonShapes.size() <= 1) break;
            int index = QRandomGenerator::global()->bounded(1, polygonShapes.size());
            QGraphicsPolygonItem* shape = polygonShapes[index];

            // Save current state
            QPointF oldPos = shape->pos();
            qreal oldRotation = shape->rotation();

            // Generate trial position using NFPs
            bool validPosition = false;
            QPointF newPos;
            qreal newRotation = rotationAngles[QRandomGenerator::global()->bounded(rotationAngles.size())];

            // Try placing near a random existing shape
            int anchorIdx = QRandomGenerator::global()->bounded(polygonShapes.size());
            QGraphicsPolygonItem* anchorShape = polygonShapes[anchorIdx];
            if (anchorShape && anchorShape != shape) {
                // Check if we should try placing in a hole
                bool tryHolePlacement = (anchorShape->data(0).toString() == "ShapeWithHole") &&
                                        nfpCalc->canFitInHole(anchorShape, shape, anchorShape->rotation(), newRotation);

                if (tryHolePlacement) {
                    // Get the hole rectangle
                    QRectF holeRect = nfpCalc->getInnerRect(anchorShape);

                    // Transform hole to scene coordinates
                    QTransform holeTransform;
                    holeTransform.rotate(anchorShape->rotation());
                    holeTransform.scale(anchorShape->scale(), anchorShape->scale());
                    QRectF sceneHoleRect = holeTransform.mapRect(holeRect);
                    sceneHoleRect.translate(anchorShape->pos());

                    // Try to position the shape inside the hole with random offsets
                    int holeAttempts = 15;
                    while (holeAttempts-- > 0 && !validPosition) {
                        // Calculate position within the hole with small random offset
                        qreal offsetX = QRandomGenerator::global()->generateDouble() * (sceneHoleRect.width() * 0.8) -
                                        (sceneHoleRect.width() * 0.4);
                        qreal offsetY = QRandomGenerator::global()->generateDouble() * (sceneHoleRect.height() * 0.8) -
                                        (sceneHoleRect.height() *
                                         0.4);
                        newPos = sceneHoleRect.center() + QPointF(offsetX, offsetY);

                        shape->setPos(newPos);
                        shape->setRotation(newRotation);

                        if (!hasOverlaps(shape, polygonShapes)) {
                            validPosition = true;
                            qDebug() << "Successfully placed shape inside a hole!";
                        }
                    }
                }

                // If hole placement failed or wasn't attempted, try normal NFP placement
                if (!validPosition) {
                    QPolygonF nfp = nfpCalc->getNFP(anchorShape, shape, anchorShape->rotation(), newRotation);
                    // Adjust sampling range based on number of shapes
                    qreal samplingRange = 100.0 + (polygonShapes.size() * 20.0); // Dynamic range
                    QRectF nfpBounds = nfp.boundingRect();
                    int attempts = 20 + (polygonShapes.size() * 2); // Dynamic attempts
                    while (attempts-- > 0 && !validPosition) {
                        qreal offsetX = QRandomGenerator::global()->generateDouble() * samplingRange - (samplingRange / 2.0);
                        qreal offsetY = QRandomGenerator::global()->generateDouble() * samplingRange - (samplingRange / 2.0);
                        QPointF candidate = nfpBounds.center() + QPointF(offsetX, offsetY);
                        if (nfp.containsPoint(candidate, Qt::OddEvenFill)) { // Outside NFP
                            newPos = anchorShape->pos() + candidate;
                            shape->setPos(newPos);
                            shape->setRotation(newRotation);
                            // Use hasOverlaps for precise collision detection
                            if (!hasOverlaps(shape, polygonShapes)) {
                                validPosition = true;
                            } else {
                                qDebug() << "NFP positioning overlap detected at" << newPos;
                            }
                        }
                    }
                }
            }

            // Fallback to random perturbation if NFP fails
            if (!validPosition) {
                qreal perturbationRange = 50.0 + (polygonShapes.size() * 10.0); // Dynamic range
                int fallbackAttempts = 30 + (polygonShapes.size() * 2); // Dynamic attempts
                while (fallbackAttempts-- > 0 && !validPosition) {
                    newPos = oldPos + QPointF(QRandomGenerator::global()->generateDouble() * perturbationRange - (perturbationRange / 2.0),
                                              QRandomGenerator::global()->generateDouble() * perturbationRange - (perturbationRange / 2.0));
                    shape->setPos(newPos);
                    shape->setRotation(newRotation);
                    if (!hasOverlaps(shape, polygonShapes)) {
                        validPosition = true;
                    } else {
                        qDebug() << "Fallback positioning overlap detected at" << newPos;
                    }
                }
            }

            if (validPosition) {
                qreal newCost = computeCost(polygonShapes);
                qreal deltaCost = newCost - bestCost;

                // Metropolis criterion
                if (deltaCost < 0 || QRandomGenerator::global()->generateDouble() < exp(-deltaCost / T)) {
                    // Accept the move
                    bestCost = newCost;
                    bestPositions[shape] = newPos;
                    bestRotations[shape] = newRotation;
                } else {
                    // Revert
                    shape->setPos(oldPos);
                    shape->setRotation(oldRotation);
                }
            } else {
                // Revert if invalid
                shape->setPos(oldPos);
                shape->setRotation(oldRotation);
                qDebug() << "Could not find a valid position for shape after attempts";
            }
        }
        // Cool down
        T *= coolingRate;
    }

    // Apply best arrangement
    for (QGraphicsItem* shape : polygonShapes) {
        shape->setPos(bestPositions[shape]);
        shape->setRotation(bestRotations[shape]);
    }
    // Update the scene to reflect the new positions and adjust scene rect if needed
    QRectF totalRect = scene->itemsBoundingRect();
    view->setSceneRect(totalRect.adjusted(-100, -100, 100, 100)); // Add padding
    scene->update();
}

// Compute the cost: area of the bounding rectangle with
qreal MainWindow::computeCost(const QList<QGraphicsPolygonItem*>& shapes) {
    if (shapes.isEmpty()) return 0;
    QRectF totalRect = shapes.first()->sceneBoundingRect();
    for (int i = 1; i < shapes.size(); ++i) {
        totalRect = totalRect.united(shapes[i]->sceneBoundingRect());
    }
    return totalRect.width() * totalRect.height();
}
