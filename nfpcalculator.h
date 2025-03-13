// #ifndef NFPCALCULATOR_H
// #define NFPCALCULATOR_H
// #include <QPolygonF>
// #include <QMap>
// #include <QtWidgets/qgraphicsitem.h>


// class nfpcalculator
// {
// public:
//     nfpcalculator(const QList<QGraphicsPolygonItem*>&shapes);
//     QPolygonF getNFP(QGraphicsPolygonItem *fixedShape,QGraphicsPolygonItem *movingShape,qreal fixedRotation,qreal movingRotation);
// private:
//     QPolygonF computeMinkowskiSum(const QPolygonF &P,const QPolygonF &negQ);
//     QMap<QString,QPolygonF> nfpCache; // cache nfps by pair and rotations
//     QList<QGraphicsPolygonItem*> allShapes;

// };

// #endif // NFPCALCULATOR_H
#ifndef NFPCALCULATOR_H
#define NFPCALCULATOR_H
#include <QPolygonF>
#include <QMap>
#include <QtWidgets/qgraphicsitem.h>


class nfpcalculator
{
public:
    nfpcalculator(const QList<QGraphicsPolygonItem*>&shapes);
    QPolygonF getNFP(QGraphicsPolygonItem *fixedShape,QGraphicsPolygonItem *movingShape,qreal fixedRotation,qreal movingRotation);

    // New methods for hole handling
    bool canFitInHole(QGraphicsPolygonItem* holeShape, QGraphicsPolygonItem* smallShape,
                      qreal holeRotation, qreal smallRotation);
    QRectF getInnerRect(QGraphicsPolygonItem* shapeWithHole);
    bool isPointInHole(QGraphicsPolygonItem* shapeWithHole, const QPointF& point);

private:
    QPolygonF computeMinkowskiSum(const QPolygonF &P,const QPolygonF &negQ);
    QMap<QString,QPolygonF> nfpCache; // cache nfps by pair and rotations
    QList<QGraphicsPolygonItem*> allShapes;
};

#endif // NFPCALCULATOR_H
