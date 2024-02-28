#ifndef QCPL_FORMAT_H
#define QCPL_FORMAT_H

#include <QSize>
#include <QVector>

#include "qcpl_types.h"

class QCustomPlot;
class QCPAxis;
class QCPColorScale;
class QCPGraph;
class QCPLegend;
class QCPRange;
class QCPTextElement;

namespace QCPL {

class Plot;
class TextFormatterBase;

//---------------------------------------------------------------------

struct AxisLimitsDlgProps
{
    QString title;
    QString unit;
    int precision = 6;
};

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props);

//---------------------------------------------------------------------

struct AxisFactorDlgProps
{
    QString title;
    Plot *plot;
};

bool axisFactorDlg(QCPAxis* axis, const AxisFactorDlgProps& props);

//---------------------------------------------------------------------

struct AxisFormatDlgProps
{
    QString title;
    QString defaultText;
    TextFormatterBase *formatter = nullptr;
    std::function<void()> onSaveDefault;
};

bool axisTextDlg(QCPAxis* axis, const AxisFormatDlgProps& props);
bool axisFormatDlg(QCPAxis* axis, const AxisFormatDlgProps& props);
bool colorScaleFormatDlg(QCPColorScale* scale, const AxisFormatDlgProps& props);

//---------------------------------------------------------------------

struct TitleFormatDlgProps
{
    QString title;
    QString defaultText;
    TextFormatterBase *formatter = nullptr;
    std::function<void()> onSaveDefault;
};

bool titleTextDlg(QCPTextElement* title, const TitleFormatDlgProps& props);
bool titleFormatDlg(QCPTextElement* title, const TitleFormatDlgProps& props);

//---------------------------------------------------------------------

struct LegendFormatDlgProps
{
    QString title;
    QString sampleText;
    std::function<void()> onSaveDefault;
};

bool legendFormatDlg(QCPLegend* legend, const LegendFormatDlgProps& props);

//---------------------------------------------------------------------

struct PlotFormatDlgProps
{
    QString title;
};

bool plotFormatDlg(Plot* plot, const PlotFormatDlgProps& props = PlotFormatDlgProps());

//---------------------------------------------------------------------

struct GraphFormatDlgProps
{
    QString title;
    // Required for genericFormatDlg()
    std::function<void()> onSaveDefault;
};

bool graphFormatDlg(QCPGraph* graph, const GraphFormatDlgProps& props);

//---------------------------------------------------------------------

class TextProcessor
{
public:
    void addVar(const QString& name, TextVarGetter getter);

    QString process(const QString& text) const;

private:
    struct Var
    {
        QString name;
        TextVarGetter getter;
    };

    QVector<Var> _vars;
};

//---------------------------------------------------------------------

/**
    Wrapper around @ref TextProcessor that knows a target object for text and can set text to it.
*/
class TextFormatterBase
{
public:
    virtual ~TextFormatterBase() {}
    void addVar(const QString& name, const QString& descr, TextVarGetter getter);
    const QVector<TextVariable>& vars() const { return _vars; }
    virtual void format() = 0;
    QString text() const { return _text; };
    void setText(const QString& text) { _text = text; }
protected:
    QString _text;
    TextProcessor _processor;
    QVector<TextVariable> _vars;
};

//---------------------------------------------------------------------

class AxisTextFormatter : public TextFormatterBase
{
public:
    AxisTextFormatter(QCPAxis* axis) : _axis(axis) {}
    void format() override;
private:
    QCPAxis *_axis;
};

//---------------------------------------------------------------------

class TitleTextFormatter : public TextFormatterBase
{
public:
    TitleTextFormatter(QCPTextElement* title) : _title(title) {}
    void format() override;
private:
    QCPTextElement *_title;
};

} // namespace QCPL

#endif // QCPL_FORMAT_H
