#include "qcpl_utils.h"

#include <QRandomGenerator>

namespace QCPL {

GraphData makeRandomSample(int count, double height)
{
    QVector<double> xs(count);
    QVector<double> ys(count);
    auto rnd = QRandomGenerator::global();
    double y = (rnd->generate()%100)*height*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (rnd->generate()%100)*height*0.01 - height*0.5);
        xs[i] = i;
        ys[i] = y;
    }
    return {xs, ys};
}

} // namespace QCPL
