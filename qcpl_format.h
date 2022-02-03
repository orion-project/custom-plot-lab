#ifndef QCPL_FORMAT_H
#define QCPL_FORMAT_H

#include <QString>

class QCPAxis;
class QCPRange;

namespace QCPL {

struct AxisTitleDlgProps
{
    QString title;
};

bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props);

struct AxisLimitsDlgProps
{
    QString title;
    QString unit;
    int precision = 6;
};

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props);

} // namespace QCPL

#endif // QCPL_FORMAT_H
