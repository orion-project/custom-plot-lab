#ifndef QCPL_FORMAT_H
#define QCPL_FORMAT_H

#include <QString>

class QCPAxis;
class QCPRange;

namespace QCPL {

bool axisTitleDlg(QCPAxis* axis);

struct AxisLimitsDlgProps
{
    QString unit;
    int precision = 6;
};

bool axisLimitsDlg(QCPRange& range, const QString& title, const AxisLimitsDlgProps& props);

} // namespace QCPL

#endif // QCPL_FORMAT_H
