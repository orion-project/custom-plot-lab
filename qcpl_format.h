#ifndef QCPL_FORMAT_H
#define QCPL_FORMAT_H

#include <QSize>
#include <QVector>

#include "qcpl_types.h"

class QCustomPlot;
class QCPAxis;
class QCPGraph;
class QCPLegend;
class QCPRange;
class QCPTextElement;

namespace QCPL {

class Plot;
class TextFormatterBase;

//---------------------------------------------------------------------

struct AxisTitleDlgProps
{
    QString title;
    QSize iconSize;
    TextFormatterBase *formatter = nullptr;
};

/// Title dialog with font and color selectors.
bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props);

//---------------------------------------------------------------------

struct AxisTitleDlgPropsV2
{
    QString title;
    QString defaultTitle;
    TextFormatterBase *formatter = nullptr;
};

/// Simplified title dialog having only text field and variables button.
bool axisTitleDlgV2(QCPAxis* axis, const AxisTitleDlgPropsV2& props);

//---------------------------------------------------------------------

struct AxisLimitsDlgProps
{
    QString title;
    QString unit;
    int precision = 6;
};

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props);

//---------------------------------------------------------------------

struct AxisFormatDlgProps
{
    QString title;
};

bool axisFormatDlg(QCPAxis* axis, const AxisFormatDlgProps& props);

//---------------------------------------------------------------------

struct TitleFormatDlgProps
{
    QString title;
    std::function<void()> onSaveDefault;
};

bool titleFormatDlg(QCPTextElement* title, const TitleFormatDlgProps& props);

//---------------------------------------------------------------------

struct LegendFormatDlgProps
{
    QString title;
    QString sampleText;
    std::function<void()> onApplied;
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
    Plot *plot;
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

class AxisTitleFormatter : public TextFormatterBase
{
public:
    AxisTitleFormatter(QCPAxis* axis);
    void format() override;
private:
    QCPAxis *_axis;
};

} // namespace QCPL

#endif // QCPL_FORMAT_H
