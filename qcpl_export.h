#ifndef QCPL_EXPORT_H
#define QCPL_EXPORT_H

#include <QString>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace QCPL {

struct GraphDataExportSettings
{
    bool csv = false;
    bool systemLocale = false;
    bool transposed = false;
    int numberPrecision = 6;
};

class GraphDataExporter
{
public:
    GraphDataExporter(const GraphDataExportSettings& settings);
    ~GraphDataExporter();

    void add(double v);
    void add(double x, double y);
    void add(const QVector<double>& v);

    void toClipboard();

private:
    QString _result, _result1;
    QTextStream *_stream;
    QTextStream *_stream1 = nullptr;
    bool _quote, _csv;

    void addToRow(QTextStream* stream, double v);
};

} // namespace QCPL

#endif // QCPL_EXPORT_H
