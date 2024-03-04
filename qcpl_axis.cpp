#include "qcpl_axis.h"

namespace QCPL {

Axis::Axis(QCPAxisRect *parent, AxisType type) : QCPAxis(parent, type)
{
}

void Axis::draw(QCPPainter *painter)
{
    if (_highlight)
    {
        QRect axisRect = this->axisRect()->rect();
        int offset = this->offset();
        auto type = this->axisType();

        QPen pen;
        pen.setWidth(mBasePen.width() + 6);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor(0, 240, 255, 120));
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

      // The same code for drawing axis as in QCPAxisPainterPrivate::draw
      QPoint origin;
      switch (type)
      {
        case QCPAxis::atLeft:   origin = axisRect.bottomLeft() +QPoint(-offset, 0); break;
        case QCPAxis::atRight:  origin = axisRect.bottomRight()+QPoint(+offset, 0); break;
        case QCPAxis::atTop:    origin = axisRect.topLeft()    +QPoint(0, -offset); break;
        case QCPAxis::atBottom: origin = axisRect.bottomLeft() +QPoint(0, +offset); break;
      }

      double xCor = 0, yCor = 0; // paint system correction, for pixel exact matches (affects baselines and ticks of top/right axes)
      switch (type)
      {
        case QCPAxis::atTop: yCor = -1; break;
        case QCPAxis::atRight: xCor = 1; break;
        default: break;
      }
      // draw baseline:
      QLineF baseLine;
      painter->setPen(pen);
      if (QCPAxis::orientation(type) == Qt::Horizontal)
        baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(axisRect.width()+xCor, yCor));
      else
        baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(xCor, -axisRect.height()+yCor));
      painter->drawLine(baseLine);
    }
    QCPAxis::draw(painter);
}

} // namespace QCPL
