#include "stslider.h"
#include <QKeyEvent>

StSlider::StSlider(Qt::Orientation orientation, QWidget *parent) :
    QSlider(orientation, parent)
{
    this->setMouseTracking(true);
    setOrientation(orientation);
    setStyle(new StStyle);
}

void StSlider::keyPressEvent(QKeyEvent *ev)
{
    ev->ignore();
}

void StSlider::keyReleaseEvent(QKeyEvent *ev)
{
    ev->ignore();
}

