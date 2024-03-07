#ifndef QCPL_AXIS_H
#define QCPL_AXIS_H

#include "qcpl_types.h"
#include "qcustomplot/qcustomplot.h"

namespace QCPL {

class Plot;

typedef XYPair<QCPAxis*> AxisPair;

class Axis : public QCPAxis
{
public:
    Axis(QCPAxisRect *parent, AxisType type);

    bool hightlight() const { return _highlight; }
    void setHightlight(bool on) { _highlight = on; }

    inline bool isX() const { return orientation() == Qt::Horizontal; }
    inline bool isY() const { return orientation() == Qt::Vertical; }

protected:
    void draw(QCPPainter *painter) override;

private:
    bool _highlight = false;
};

AxisPair chooseAxes(Plot* plot, const AxisPair& chosenAxes);
QCPAxis* chooseAxis(Plot* plot);

} // namespace QCPL


#endif // QCPL_AXIS_H
