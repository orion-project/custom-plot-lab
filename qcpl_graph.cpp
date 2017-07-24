#include "qcpl_graph.h"

namespace QCPL {

const int maxSelectorHandles = 20;

QCPSelectionDecorator* LineGraph::_sharedSelectionDecorator = nullptr;

void LineGraph::setSharedSelectionDecorator(QCPSelectionDecorator* decorator)
{
    if (_sharedSelectionDecorator && _sharedSelectionDecorator != decorator)
    {
        delete _sharedSelectionDecorator;
        _sharedSelectionDecorator = nullptr;
    }
    _sharedSelectionDecorator = decorator;
}

LineGraph::LineGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) : QCPGraph(keyAxis, valueAxis)
{
}

void LineGraph::draw(QCPPainter *painter)
{
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mKeyAxis.data()->range().size() <= 0 || mDataContainer->isEmpty()) return;
  if (mLineStyle == lsNone && mScatterStyle.isNone()) return;

  QVector<QPointF> lines, scatters; // line and (if necessary) scatter pixel coordinates will be stored here while iterating over segments

  QCPSelectionDecorator* selectionDecorator = _sharedSelectionDecorator ? _sharedSelectionDecorator : mSelectionDecorator;

  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i=0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();
    // get line pixel points appropriate to line style:
    QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getLines takes care)
    getLines(&lines, lineDataRange);

    // draw selection:
    if (isSelectedSegment && selectionDecorator && selectionDecorator->pen() != Qt::NoPen)
    {
      QPen selectionPen(selectionDecorator->pen());
      if (mLineStyle != lsNone)
        selectionPen.setWidth(mPen.width() + 2*selectionDecorator->pen().width());
      selectionPen.setJoinStyle(Qt::RoundJoin);

      painter->setPen(selectionPen);
      painter->setBrush(Qt::NoBrush);

      drawLinePlot(painter, lines);
    }

    // draw line:
    if (mLineStyle != lsNone)
    {
      painter->setPen(mPen);
      painter->setBrush(Qt::NoBrush);

      drawLinePlot(painter, lines); // also step plots can be drawn as a line plot
    }

    // draw scatters:
    if (!mScatterStyle.isNone())
    {
      getScatters(&scatters, allSegments.at(i));
      drawScatterPlot(painter, scatters, mScatterStyle);
    }

    // draw selection:
    if (isSelectedSegment && selectionDecorator && !selectionDecorator->scatterStyle().isNone())
    {
      if (scatters.isEmpty())
        getScatters(&scatters, allSegments.at(i));

      if (scatters.size() <= maxSelectorHandles)
      {
        drawScatterPlot(painter, scatters, selectionDecorator->scatterStyle());
      }
      else
      {
        QVector<QPointF> selectorHandles;
        selectorHandles.resize(maxSelectorHandles);
        int step = scatters.size() / maxSelectorHandles;
        for (int si = 0, hi = 0; si < scatters.size() && hi < maxSelectorHandles; si += step, hi++)
        {
          const QPointF& p = scatters.at(si);
          selectorHandles[hi].setX(p.x());
          selectorHandles[hi].setY(p.y());
        }
        drawScatterPlot(painter, selectorHandles, selectionDecorator->scatterStyle());
      }
    }
  }

  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (selectionDecorator)
    selectionDecorator->drawDecoration(painter, selection());
}

} // namespace QCPL
