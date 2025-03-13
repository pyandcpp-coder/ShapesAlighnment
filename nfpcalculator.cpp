// #include "nfpcalculator.h"
// #include <QTransform>
// #include <algorithm>

// // Helper function to compute the cross product of vectors OA and OB
// // Used to determine the orientation of three points
// static qreal crossProduct(const QPointF& O, const QPointF& A, const QPointF& B) {
//     return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
// }

// // Compute the convex hull of a set of points using Andrew's monotone chain algorithm
// static QPolygonF convexHull(QPolygonF points) {
//     if (points.size() <= 3) return points; // If 3 or fewer points, it's already a convex hull

//     // Sort points lexicographically (by x-coordinate, then y-coordinate)
//     std::sort(points.begin(), points.end(), [](const QPointF& a, const QPointF& b) {
//         return (a.x() < b.x()) || (a.x() == b.x() && a.y() < b.y());
//     });

//     // Build the lower hull
//     QPolygonF lower;
//     for (const QPointF& p : points) {
//         while (lower.size() >= 2 && crossProduct(lower[lower.size() - 2], lower[lower.size() - 1], p) <= 0) {
//             lower.removeLast(); // Remove the last point if it makes a non-left turn
//         }
//         lower.append(p);
//     }

//     // Build the upper hull
//     QPolygonF upper;
//     for (int i = points.size() - 1; i >= 0; --i) {
//         const QPointF& p = points[i];
//         while (upper.size() >= 2 && crossProduct(upper[upper.size() - 2], upper[upper.size() - 1], p) <= 0) {
//             upper.removeLast(); // Remove the last point if it makes a non-left turn
//         }
//         upper.append(p);
//     }

//     // Combine the hulls, excluding the first and last points of upper to avoid duplicates
//     lower.removeLast(); // Remove the last point of lower (already in upper)
//     upper.removeLast(); // Remove the last point of upper (already in lower)
//     lower += upper;     // Concatenate the upper hull to the lower hull

//     return lower;
// }

// nfpcalculator::nfpcalculator(const QList<QGraphicsPolygonItem*>& shapes) : allShapes(shapes) {}

// // Get or compute NFP for a shape pair at given rotations
// QPolygonF nfpcalculator::getNFP(QGraphicsPolygonItem* fixedShape, QGraphicsPolygonItem* movingShape,
//                                 qreal fixedRotation, qreal movingRotation) {
//     // Create a unique key for this pair and rotations
//     QString key = QString("%1_%2_%3_%4_%5_%6")
//                       .arg((quintptr)fixedShape)
//                       .arg((quintptr)movingShape)
//                       .arg(fixedRotation)
//                       .arg(movingRotation)
//                       .arg(fixedShape->scale()) // Include scaling in the key
//                       .arg(movingShape->scale());

//     // Return cached NFP if available
//     if (nfpCache.contains(key)) {
//         return nfpCache[key];
//     }

//     // Get polygons in local coordinates
//     QPolygonF polyA = fixedShape->polygon();
//     QPolygonF polyB = movingShape->polygon();

//     // Apply scaling transformations
//     QTransform scaleTransformA, scaleTransformB;
//     scaleTransformA.scale(fixedShape->scale(), fixedShape->scale());
//     scaleTransformB.scale(movingShape->scale(), movingShape->scale());
//     polyA = scaleTransformA.map(polyA);
//     polyB = scaleTransformB.map(polyB);

//     // Apply rotations
//     QTransform rotateTransformA, rotateTransformB;
//     rotateTransformA.rotate(fixedRotation);
//     rotateTransformB.rotate(movingRotation);
//     QPolygonF rotatedA = rotateTransformA.map(polyA);
//     QPolygonF rotatedB = rotateTransformB.map(polyB);

//     // Negate B’s vertices
//     QPolygonF negB;
//     for (const QPointF& p : rotatedB) {
//         negB << -p;
//     }

//     // Compute NFP and cache it
//     QPolygonF nfp = computeMinkowskiSum(rotatedA, negB);
//     nfpCache[key] = nfp;
//     return nfp;
// }

// // Compute Minkowski sum for convex polygons and return its convex hull
// QPolygonF nfpcalculator::computeMinkowskiSum(const QPolygonF& P, const QPolygonF& negQ) {
//     QPolygonF result;
//     // Compute pairwise sums of vertices
//     for (const QPointF& p : P) {
//         for (const QPointF& q : negQ) {
//             result << (p + q);
//         }
//     }
//     // Return convex hull using our custom implementation
//     return convexHull(result);
// }
#include "nfpcalculator.h"
#include <QTransform>
#include <algorithm>
#include <QDebug>

// Helper function to compute the cross product of vectors OA and OB
// Used to determine the orientation of three points
static qreal crossProduct(const QPointF& O, const QPointF& A, const QPointF& B) {
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

// Compute the convex hull of a set of points using Andrew's monotone chain algorithm
static QPolygonF convexHull(QPolygonF points) {
    if (points.size() <= 3) return points; // If 3 or fewer points, it's already a convex hull

    // Sort points lexicographically (by x-coordinate, then y-coordinate)
    std::sort(points.begin(), points.end(), [](const QPointF& a, const QPointF& b) {
        return (a.x() < b.x()) || (a.x() == b.x() && a.y() < b.y());
    });

    // Build the lower hull
    QPolygonF lower;
    for (const QPointF& p : points) {
        while (lower.size() >= 2 && crossProduct(lower[lower.size() - 2], lower[lower.size() - 1], p) <= 0) {
            lower.removeLast(); // Remove the last point if it makes a non-left turn
        }
        lower.append(p);
    }

    // Build the upper hull
    QPolygonF upper;
    for (int i = points.size() - 1; i >= 0; --i) {
        const QPointF& p = points[i];
        while (upper.size() >= 2 && crossProduct(upper[upper.size() - 2], upper[upper.size() - 1], p) <= 0) {
            upper.removeLast(); // Remove the last point if it makes a non-left turn
        }
        upper.append(p);
    }

    // Combine the hulls, excluding the first and last points of upper to avoid duplicates
    lower.removeLast(); // Remove the last point of lower (already in upper)
    upper.removeLast(); // Remove the last point of upper (already in lower)
    lower += upper;     // Concatenate the upper hull to the lower hull

    return lower;
}

nfpcalculator::nfpcalculator(const QList<QGraphicsPolygonItem*>& shapes) : allShapes(shapes) {}

// Get or compute NFP for a shape pair at given rotations
QPolygonF nfpcalculator::getNFP(QGraphicsPolygonItem* fixedShape, QGraphicsPolygonItem* movingShape,
                                qreal fixedRotation, qreal movingRotation) {
    // Create a unique key for this pair and rotations
    QString key = QString("%1_%2_%3_%4_%5_%6")
                      .arg((quintptr)fixedShape)
                      .arg((quintptr)movingShape)
                      .arg(fixedRotation)
                      .arg(movingRotation)
                      .arg(fixedShape->scale()) // Include scaling in the key
                      .arg(movingShape->scale());

    // Return cached NFP if available
    if (nfpCache.contains(key)) {
        return nfpCache[key];
    }

    // Get polygons in local coordinates
    QPolygonF polyA = fixedShape->polygon();
    QPolygonF polyB = movingShape->polygon();

    // Apply scaling transformations
    QTransform scaleTransformA, scaleTransformB;
    scaleTransformA.scale(fixedShape->scale(), fixedShape->scale());
    scaleTransformB.scale(movingShape->scale(), movingShape->scale());
    polyA = scaleTransformA.map(polyA);
    polyB = scaleTransformB.map(polyB);

    // Apply rotations
    QTransform rotateTransformA, rotateTransformB;
    rotateTransformA.rotate(fixedRotation);
    rotateTransformB.rotate(movingRotation);
    QPolygonF rotatedA = rotateTransformA.map(polyA);
    QPolygonF rotatedB = rotateTransformB.map(polyB);

    // Negate B's vertices
    QPolygonF negB;
    for (const QPointF& p : rotatedB) {
        negB << -p;
    }

    // Compute NFP and cache it
    QPolygonF nfp = computeMinkowskiSum(rotatedA, negB);
    nfpCache[key] = nfp;
    return nfp;
}

// Compute Minkowski sum for convex polygons and return its convex hull
QPolygonF nfpcalculator::computeMinkowskiSum(const QPolygonF& P, const QPolygonF& negQ) {
    QPolygonF result;
    // Compute pairwise sums of vertices
    for (const QPointF& p : P) {
        for (const QPointF& q : negQ) {
            result << (p + q);
        }
    }
    // Return convex hull using our custom implementation
    return convexHull(result);
}

// Get the inner rectangle of a shape with a hole
QRectF nfpcalculator::getInnerRect(QGraphicsPolygonItem* shapeWithHole) {
    // Get the hole rectangle if it exists
    if (shapeWithHole->data(0).toString() == "ShapeWithHole") {
        return shapeWithHole->data(1).value<QRectF>();
    }
    return QRectF(); // Empty rectangle if no hole
}

// Check if a small shape can fit inside a hole of another shape
bool nfpcalculator::canFitInHole(QGraphicsPolygonItem* holeShape, QGraphicsPolygonItem* smallShape,
                                 qreal holeRotation, qreal smallRotation) {
    // Check if the shape has a hole
    if (holeShape->data(0).toString() != "ShapeWithHole") {
        return false;
    }

    // Get the hole rectangle in local coordinates
    QRectF holeRect = holeShape->data(1).value<QRectF>();

    // Get transformed bounding rects
    QTransform holeTransform;
    holeTransform.rotate(holeRotation);
    holeTransform.scale(holeShape->scale(), holeShape->scale());
    QRectF transformedHole = holeTransform.mapRect(holeRect);

    // Get the small shape's transformed bounding rect
    QTransform smallTransform;
    smallTransform.rotate(smallRotation);
    smallTransform.scale(smallShape->scale(), smallShape->scale());
    QRectF smallBounds = smallTransform.mapRect(smallShape->boundingRect());

    // Check if small shape can fit in the hole (with margin)
    const qreal margin = 2.0;
    return (smallBounds.width() + margin < transformedHole.width() &&
            smallBounds.height() + margin < transformedHole.height());
}

// Check if a point is inside the hole of a shape
bool nfpcalculator::isPointInHole(QGraphicsPolygonItem* shapeWithHole, const QPointF& point) {
    // Check if the shape has a hole
    if (shapeWithHole->data(0).toString() != "ShapeWithHole") {
        return false;
    }

    // Get the hole rectangle in scene coordinates
    QRectF holeRect = shapeWithHole->data(1).value<QRectF>();
    QTransform transform;
    transform.rotate(shapeWithHole->rotation());
    transform.scale(shapeWithHole->scale(), shapeWithHole->scale());
    QRectF sceneHoleRect = transform.mapRect(holeRect);
    sceneHoleRect.translate(shapeWithHole->pos());

    // Check if the point is inside the hole
    return sceneHoleRect.contains(point);
}
