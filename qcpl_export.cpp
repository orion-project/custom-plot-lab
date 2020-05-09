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
    if (settings.transposed)
    {
        _stream1 = new QTextStream(&_result1);
        _stream1->setRealNumberNotation(QTextStream::SmartNotation);
        _stream1->setRealNumberPrecision(settings.numberPrecision);
        _stream1->setLocale(settings.systemLocale ? QLocale::system() : QLocale::c());
    }
    _quote = settings.csv && _stream->locale().decimalPoint() == ',';
    _csv = settings.csv;
}

GraphDataExporter::~GraphDataExporter()
{
    delete _stream;
    if (_stream1)
        delete _stream1;
}

void GraphDataExporter::add(double v)
{
    if (_stream1)
    {
        // don't use _stream1 here, it's only a marker that we use row-mode
        addToRow(_stream, v);
    }
    else
    {
        if (_quote)
            *_stream << '"' << v << '"';
        else *_stream << v;
        *_stream << '\n';
    }
}

void GraphDataExporter::addToRow(QTextStream* stream, double v)
{
    if (_quote)
        *stream << '"' << v << '"';
    else *stream << v;

    *_stream << (_csv ? ',' : '\t');
}

void GraphDataExporter::add(double x, double y)
{
    if (_stream1)
    {
        addToRow(_stream, x);
        addToRow(_stream1, y);
    }
    else
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
}

void GraphDataExporter::add(const QVector<double>& v)
{
    int sz = v.size();
    for (int i = 0; i < sz; i++)
    {
        if (_quote)
            *_stream << '"' << v.at(i) << '"';
        else *_stream << v.at(i);

        if (i < sz-1)
            *_stream << (_csv ? ',' : '\t');
    }
    *_stream << '\n';
}

void GraphDataExporter::toClipboard()
{
    QString res;
    if (_stream1)
    {
        auto r = _result.trimmed();
        auto r1 = _result1.trimmed();
        if (r.endsWith(',')) r = r.left(r.length()-1);
        if (r1.endsWith(',')) r1 = r1.left(r1.length()-1);
        res = r1.isEmpty() ? r : (r + '\n' + r1 + '\n');
    }
    else res = _result;
    qApp->clipboard()->setText(res);
}

} // namespace QCPL
