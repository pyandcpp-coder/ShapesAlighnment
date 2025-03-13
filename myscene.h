#ifndef MYSCENE_H
#define MYSCENE_H

#include <QGraphicsScene>

class myscene : public QGraphicsScene{
    Q_OBJECT

public:
    myscene(QObject *parent = nullptr);
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

};

#endif // MYSCENE_H
