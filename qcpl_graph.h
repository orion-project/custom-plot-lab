#ifndef QCPL_GRAPH_H
#define QCPL_GRAPH_H

#include "qcustomplot/qcustomplot.h"

namespace QCPL {

class LineGraph : public QCPGraph
{
public:
    explicit LineGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    static QCPSelectionDecorator* sharedSelectionDecorator() { return _sharedSelectionDecorator; }
    static void setSharedSelectionDecorator(QCPSelectionDecorator* decorator);

protected:
    void draw(QCPPainter *painter) override;

private:
    static QCPSelectionDecorator* _sharedSelectionDecorator;
};

} // namespace QCPL

#endif // QCPL_GRAPH_H
