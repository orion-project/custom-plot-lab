#include "qcpl_cursor.h"

#include <QApplication>

QCPCursor::QCPCursor(QCustomPlot *plot) : QCPGraph(plot->xAxis, plot->yAxis)
{
    setAntialiased(false);
    setPen(QPen(QColor::fromRgb(80, 80, 255))); // TODO make customizable

    connect(plot, SIGNAL(emptySpaceDoubleClicked(QMouseEvent*)), this, SLOT(mouseDoubleClick(QMouseEvent*)));
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));

    removeFromLegend();
    setPosition(0, 0);
}

void QCPCursor::setVisible(bool on)
{
    QCPGraph::setVisible(on);
    parentPlot()->replot();
}

void QCPCursor::setShape(CursorShape value)
{
    _shape = value;
    parentPlot()->replot();
}

void QCPCursor::mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details)
{
    Q_UNUSED(details)
    _followMouse = false;
    setPixelPosition(event->pos());
}

void QCPCursor::mouseDoubleClick(QMouseEvent *evt)
{
    setPixelPosition(evt->pos());
}

void QCPCursor::mousePress(QMouseEvent *evt)
{
    if (!visible()) return;
    if (evt->button() == Qt::LeftButton)
    {
        if (_canDragX) _dragX = true;
        if (_canDragY) _dragY = true;
    }
}

void QCPCursor::mouseRelease(QMouseEvent*)
{
    if (!visible()) return;
    _dragX = false;
    _dragY = false;
#ifdef Q_OS_MAC
    parentPlot()->skipDragging = false;
#endif
}

void QCPCursor::mouseMove(QMouseEvent *evt)
{
    if (!visible()) return;
    if (_followMouse)
    {
        setPixelPosition(evt->pos());
        return;
    }
    else
    {
        if (_dragX || _dragY)
        {
            if (_dragX && _dragY)
            {
                setPixelPosition(evt->pos());
            }
            else if (_dragX)
            {
                double cursorX, cursorY;
                pixelPosition(cursorX, cursorY);
                setPixelPosition(evt->x(), cursorY);
            }
            else
            {
                double cursorX, cursorY;
                pixelPosition(cursorX, cursorY);
                setPixelPosition(cursorX, evt->y());
            }
        }
        else
        {
            double mouseX = evt->x();
            double mouseY = evt->y();
            double cursorX, cursorY;
            pixelPosition(cursorX, cursorY);
            QCPAxisRect *r = parentPlot()->axisRect();
            _canDragX = (_shape == VerticalLine || _shape == CrossLines)
                    && mouseY > r->top()
                    && mouseY < r->bottom() && qAbs(cursorX - mouseX) <= 2;
            _canDragY = (_shape == HorizontalLine || _shape == CrossLines)
                    && mouseX > r->left()
                    && mouseX < r->right() && qAbs(cursorY - mouseY) <= 2;
#ifdef Q_OS_MAC
            parentPlot()->skipDragging = _canDragX or _canDragY;
#endif
            QApplication::restoreOverrideCursor();
            if (_canDragX && _canDragY)
                QApplication::setOverrideCursor(Qt::SizeAllCursor);
            else if (_canDragX)
                QApplication::setOverrideCursor(Qt::SizeHorCursor);
            else if (_canDragY)
                QApplication::setOverrideCursor(Qt::SizeVerCursor);
        }
    }
}

void QCPCursor::draw(QCPPainter *painter)
{
    painter->setPen(mPen);
    painter->setBrush(Qt::NoBrush);

    double x, y;
    pixelPosition(x, y);
    QCPAxisRect *r = parentPlot()->axisRect();
    if (_shape == HorizontalLine || _shape == CrossLines)
        painter->drawLine(QPointF(r->left(), y), QPointF(r->right(), y));
    if (_shape == VerticalLine || _shape == CrossLines)
        painter->drawLine(QPointF(x, r->bottom()), QPointF(x, r->top()));
}

QPointF QCPCursor::position() const
{
    auto point = data()->constBegin();
    return QPointF(point->key, point->value);
}

void QCPCursor::setPosition(const double& x, const double& y, bool replot)
{
    setData(QVector<double>() << x, QVector<double>() << y);
    if (replot)
        parentPlot()->replot();
    emit positionChanged();
}

void QCPCursor::pixelPosition(double& x, double& y) const
{
    QPointF point = position();
    coordsToPixels(point.x(), point.y(), x, y);
}

void QCPCursor::setPixelPosition(const double &x, const double &y, bool replot)
{
    double key, value;
    pixelsToCoords(x, y, key, value);
    setPosition(key, value, replot);
}

void QCPCursor::moveToCenter(bool replot)
{
    QCPAxisRect *r = parentPlot()->axisRect();
    setPixelPosition((r->right() + r->left())/2, (r->top() + r->bottom())/2, replot);
}

void QCPCursor::setFollowMouse(bool value)
{
    _followMouse = value;
    if (_followMouse)
    {
        _dragX = _canDragX = false;
        _dragY = _canDragY = false;
        QApplication::restoreOverrideCursor();
        setPixelPosition(parentPlot()->mapFromGlobal(QCursor::pos()));
    }
}
