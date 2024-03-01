#ifndef QCPL_EXPORT_H
#define QCPL_EXPORT_H

#include <QVector>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

class QCustomPlot;

namespace QCPL {

struct GraphDataExportSettings
{
    bool csv = false;
    bool systemLocale = false;
    bool transposed = false;
    int numberPrecision = 6;
    bool mergePoints = false;
};

class BaseGraphDataExporter
{
public:
    BaseGraphDataExporter(const GraphDataExportSettings& settings, bool noResult = false);
    virtual ~BaseGraphDataExporter();

    QString format(const double& v);

    void addValue(const QString& v) { addValue(_stream, v); }
    void addValue(const double& v) { addValue(_stream, format(v)); }
    void addSeparator() { addSeparator(_stream); }
    void addNewline() { addNewline(_stream); }

    virtual QString result() const { return _result; }

    void toClipboard();

protected:
    bool _quote, _csv;
    QString _formatted;
    QTextStream *_formatter;
    QString _result;
    QTextStream *_stream = nullptr;

    void addValue(QTextStream* stream, const QString& v);
    void addSeparator(QTextStream* stream);
    void addNewline(QTextStream* stream);
};

class GraphDataExporter
{
public:
    GraphDataExporter(const GraphDataExportSettings& settings);
    ~GraphDataExporter();

    void add(const double& v);
    void add(const double& x, const double& y);
    void add(const QVector<double>& vs);

    void toClipboard();

private:
    class ExporterImpl* _impl;
    QString _prev, _prevX, _prevY;
    bool _merge;
};

struct ExportToImageProps
{
    QString fileName;
    int width = 0;
    int height = 0;
    bool proportional = true;
    bool scalePixels = false;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& obj);
};

bool exportImageDlg(QCustomPlot* plot, ExportToImageProps& props);

} // namespace QCPL

#endif // QCPL_EXPORT_H
