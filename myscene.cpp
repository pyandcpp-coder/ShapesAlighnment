#include "myscene.h"
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QPointF>
#include <QRectF>
#include <QPolygonF>

myscene::myscene(QObject *parent) : QGraphicsScene(parent) {}

void myscene::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void myscene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void myscene::dropEvent(QGraphicsSceneDragDropEvent *event) {
    if (event->mimeData()->hasText()) {
        QString shapeType = event->mimeData()->text();
        QPointF pos = event->scenePos();
        QGraphicsPolygonItem *newItem = nullptr;

        if (shapeType == "Rectangle") {
            // Define a rectangle as a polygon with 4 points
            QPolygonF rectPoly;
            rectPoly << QPointF(-25, -25) << QPointF(25, -25)
                     << QPointF(25, 25) << QPointF(-25, 25);
            newItem = new QGraphicsPolygonItem(rectPoly);
            newItem->setBrush(QBrush(Qt::blue));
        }
        else if (shapeType == "Ellipse") { // Changed from "Circle" to "Ellipse" to match button label
            // Approximate an ellipse with a polygon (e.g., 20 sides)
            QPolygonF ellipsePoly;
            int numSides = 20; // Increase for smoother approximation
            for (int i = 0; i < numSides; ++i) {
                qreal angle = 2 * M_PI * i / numSides;
                // Scale to create an ellipse (e.g., width 50, height 30)
                ellipsePoly << QPointF(25 * cos(angle), 15 * sin(angle));
            }
            newItem = new QGraphicsPolygonItem(ellipsePoly);
            newItem->setBrush(QBrush(Qt::cyan));
        }
        else if (shapeType == "Triangle") {
            // Define a triangle as a polygon with 3 points
            QPolygonF trianglePoly;
            trianglePoly << QPointF(0, 0) << QPointF(50, 0) << QPointF(25, -50);
            newItem = new QGraphicsPolygonItem(trianglePoly);
            newItem->setBrush(QBrush(Qt::red));
        }
        else if (shapeType == "Square") {
            // Define a square as a polygon with 4 points
            QPolygonF squarePoly;
            squarePoly << QPointF(0, 0) << QPointF(50, 0)
                       << QPointF(50, 50) << QPointF(0, 50);
            newItem = new QGraphicsPolygonItem(squarePoly);
            newItem->setBrush(QBrush(Qt::green));
        }
        else if(shapeType=="C"){
            // Define a C shape with an inner hole
            QPolygonF cPoly;
            cPoly << QPointF(40, 0)      // 1. Top Right corner
                  << QPointF(-10, 0)     // 2. Top Left corner
                  << QPointF(-10, 90)    // 3. Bottom Left corner
                  << QPointF(40, 90)     // 4. Bottom Right corner
                  << QPointF(40, 78)     // 5. Inner Bottom Right corner
                  << QPointF(10, 78)     // 6. Inner Bottom Left corner
                  << QPointF(10, 12)     // 7. Inner Top Left corner
                  << QPointF(40, 12);    // 8. Inner Top Right corner

            newItem = new QGraphicsPolygonItem(cPoly);
            newItem->setBrush(QBrush(Qt::green));

            // Store metadata about the hole for later use in optimization
            newItem->setData(0, "ShapeWithHole");

            // Store the rectangle that represents the inner hole (x, y, width, height)
            QRectF holeRect(10, 12, 30, 66);
            newItem->setData(1, QVariant::fromValue(holeRect));
        }
        else if (shapeType == "Star") {
            // Define a star shape
            QPolygonF starPoly;
            int numPoints = 5;
            qreal outerRadius = 25;
            qreal innerRadius = 10;

            for (int i = 0; i < numPoints * 2; ++i) {
                qreal angle = M_PI * i / numPoints;
                qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
                starPoly << QPointF(radius * cos(angle), radius * sin(angle));
            }

            newItem = new QGraphicsPolygonItem(starPoly);
            newItem->setBrush(QBrush(Qt::yellow));
        }
        else if (shapeType == "Clear") {
            // Clear all items from the scene
            clear();
            event->acceptProposedAction();
            return;
        }else if(shapeType=="Curve C"){
            // Create a C shape with curved edges using QPainterPath
            QPainterPath path;

            // Outer arc (the outer edge of the C)
            path.arcMoveTo(0, 0, 100, 100, 45);          // Position at the start angle
            path.arcTo(0, 0, 100, 100, 45, 270);         // Draw outer arc (270 degrees)

            // Inner arc (the inner edge of the C)
            path.arcTo(20, 20, 60, 60, 315, -270);       // Draw inner arc in opposite direction

            // Close the path
            path.closeSubpath();

            QPolygonF poly;
            for (qreal percent = 0; percent <= 1.0; percent += 0.02) {
                QPointF point = path.pointAtPercent(percent);
                poly << point;
            }

            newItem = new QGraphicsPolygonItem(poly);
            newItem->setBrush(QBrush(Qt::green));

            // Store metadata
            newItem->setData(0, "ShapeWithHole");
            QRectF holeRect(20, 20, 60, 60);
            newItem->setData(1, QVariant::fromValue(holeRect));
        }
        if (newItem) {
            newItem->setPos(pos);
            newItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            newItem->setFlag(QGraphicsItem::ItemIsSelectable, true); // Make it selectable for scaling
            addItem(newItem);
            event->acceptProposedAction();
        } else {
            qDebug() << "Failed to create shape for type:" << shapeType;
            event->ignore();
        }
    } else {
        event->ignore();
    }
}
