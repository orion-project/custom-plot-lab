#include "qcpl_export.h"

#include <QApplication>
#include <QClipboard>
#include <QTextStream>

#include "qcpl_types.h"

namespace QCPL {

GraphDataExporter::GraphDataExporter(const GraphDataExportSettings& settings)
{
    _stream = new QTextStream(&_result);
    _stream->setRealNumberNotation(QTextStream::SmartNotation);
    _stream->setRealNumberPrecision(settings.numberPrecision);
    _stream->setLocale(settings.systemLocale ? QLocale::system() : QLocale::c());
    _quote = settings.csv && _stream->locale().decimalPoint() == ',';
    _csv = settings.csv;
}

GraphDataExporter::~GraphDataExporter()
{
    delete _stream;
}

void GraphDataExporter::add(double v)
{
    if (_quote)
        *_stream << '"' << v << '"';
    else *_stream << v;
    *_stream << '\n';
}

void GraphDataExporter::add(double x, double y)
{
    if (_quote)
        *_stream << '"' << x << '"';
    else *_stream << x;

    *_stream << (_csv ? ',' : '\t');

    if (_quote)
        *_stream << '"' << y << '"';
    else *_stream << y;

    *_stream << '\n';
}

void GraphDataExporter::toClipboard()
{
    qApp->clipboard()->setText(_result);
}

} // namespace QCPL
