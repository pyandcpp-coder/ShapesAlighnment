#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <myscene.h>
#include <QDrag>
#include <QMimeData>
#include <QPushButton>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QApplication>
#include <QPoint>
#include <QDoubleSpinBox>
#include "nfpcalculator.h"
#include <QGraphicsItem>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ShapeButton : public QPushButton
{
    Q_OBJECT
public:
    ShapeButton(const QString &text, QWidget *parent = nullptr) : QPushButton(text, parent) {}
protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
            dragStartPosition = event->pos();
        QPushButton::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!(event->buttons() & Qt::LeftButton)) // check if the left button is pressed
            return;
        if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
            return;
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData; // carries the data button data
        mimeData->setText(text()); // set the data to button's label
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction); // starts dragging, copying the data
    }
private:
    QPoint dragStartPosition;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void onSelectionChanged();
    void onScaleChanged(double value);
    void arrangeShapes();

    // Template function to check for overlaps between shapes
    template <typename T>
    bool hasOverlaps(QGraphicsItem* shape, const QList<T*>& others) {
        for (T* other : others) {
            if (other != shape && shape->collidesWithItem(other)) {
                return true;
            }
        }
        return false;
    }

    // Declare computeCost as a member function
    qreal computeCost(const QList<QGraphicsPolygonItem*>& shapes);

private:
    QDoubleSpinBox *scaleSpinBox;
    QGraphicsItem *selectedItem;
    myscene *scene;
    QGraphicsView *view;
    nfpcalculator *nfpCalc; // Pointer to NFP calculator
};

#endif // MAINWINDOW_H
