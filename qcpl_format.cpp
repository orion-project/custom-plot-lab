#include "qcpl_format.h"

#include "qcpl_format_axis.h"
#include "qcpl_format_graph.h"
#include "qcpl_format_legend.h"
#include "qcpl_format_plot.h"
#include "qcpl_format_title.h"
#include "qcpl_plot.h"
#include "qcpl_text_editor.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

using namespace Ori::Layouts;

namespace QCPL {

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props)
{
    auto editorMin = new Ori::Widgets::ValueEdit;
    auto editorMax = new Ori::Widgets::ValueEdit;
    Ori::Gui::adjustFont(editorMin);
    Ori::Gui::adjustFont(editorMax);
    editorMin->setNumberPrecision(props.precision);
    editorMax->setNumberPrecision(props.precision);
    editorMin->setValue(range.lower);
    editorMax->setValue(range.upper);
    editorMin->selectAll();

    QWidget w;
    auto layout = new QFormLayout(&w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Min") : QString("Min (%1)").arg(props.unit)), editorMin);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Max") : QString("Max (%1)").arg(props.unit)), editorMax);

    if (Ori::Dlg::Dialog(&w, false)
            .withTitle(props.title)
            .withContentToButtonsSpacingFactor(3)
            .exec())
    {
        range.lower = editorMin->value();
        range.upper = editorMax->value();
        range.normalize();
        return true;
    }
    return false;
}

template <class TProps>
bool genericTextDlg(const TProps& props,
                    std::function<QString()> getText,
                    std::function<void(const QString&)> setText)
{
    TextOnlyEditorWidget::Options opts;
    if (props.formatter)
    {
        opts.defaultText = props.defaultText;
        opts.vars = props.formatter->vars();
    }
    TextOnlyEditorWidget editor(opts);
    if (props.formatter)
        editor.setText(props.formatter->text());
    else
        editor.setText(getText());

    auto style = qApp->style();
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutTopMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        if (props.formatter)
        {
            props.formatter->setText(editor.text());
            props.formatter->format();
        }
        else
            setText(editor.text());
        return true;
    }
    return false;
}


bool axisTextDlg(QCPAxis* axis, const AxisFormatDlgProps& props)
{
    if (genericTextDlg(props,
            [axis](){ return axis->label(); },
            [axis](const QString& text){ axis->setLabel(text); }))
    {
        axis->parentPlot()->replot();
        return true;
    }
    return false;
}

bool titleTextDlg(QCPTextElement* title, const TitleFormatDlgProps& props)
{
    if (genericTextDlg(props,
            [title](){ return title->text(); },
            [title](const QString& text){ title->setText(text); }))
    {
        auto plot = qobject_cast<Plot*>(title->parentPlot());
        if (plot)
            plot->updateTitleVisibility();
        title->parentPlot()->replot();
        return true;
    }
    return false;
}

template <class TEditor, class TProps>
bool genericFormatDlg(TEditor *editor, const TProps& props)
{
    QDialog dlg(qApp->activeWindow());
    dlg.setWindowTitle(props.title);

    auto style = qApp->style();
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->connect(buttons, &QDialogButtonBox::accepted, &dlg, [&dlg, editor, props](){
        editor->apply();
        if (props.onSaveDefault and editor->needSaveDefault())
            props.onSaveDefault();
        dlg.accept();
    });
    buttons->connect(buttons, &QDialogButtonBox::rejected, &dlg, [&dlg, editor](){
        editor->restore();
        dlg.reject();
    });
    auto previewBtn = buttons->addButton(dlg.tr("Preview"), QDialogButtonBox::ApplyRole);
    previewBtn->connect(previewBtn, &QPushButton::pressed, editor, [editor](){
        editor->apply();
    });

    LayoutV({editor, buttons}).setMargin(0).useFor(&dlg);

    // Set margins after inserting buttons into layout
    // Otherwise margins can be reset when inserting into a layout having zero magrins
    // This doesn't happen on default style but coud do if app style sheet is overriden
    // (even particularly, like in reZonator which only provides styles for some components)
    buttons->layout()->setContentsMargins(
        style->pixelMetric(QStyle::PM_LayoutLeftMargin),
        style->pixelMetric(QStyle::PM_LayoutTopMargin),
        style->pixelMetric(QStyle::PM_LayoutRightMargin),
        style->pixelMetric(QStyle::PM_LayoutBottomMargin));

    return dlg.exec() == QDialog::Accepted;
}

bool axisFormatDlg(QCPAxis* axis, const AxisFormatDlgProps& props)
{
    return genericFormatDlg(new AxisFormatWidget(axis, props), props);
}

bool titleFormatDlg(QCPTextElement* title, const TitleFormatDlgProps& props)
{
    return genericFormatDlg(new TitleFormatWidget(title, props), props);
}

bool legendFormatDlg(QCPLegend* legend, const LegendFormatDlgProps& props)
{
    return genericFormatDlg(new LegendFormatWidget(legend, props), props);
}

bool plotFormatDlg(Plot* plot, const PlotFormatDlgProps &props)
{
    PlotFormatWidget editor(plot, {});

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title.isEmpty() ? "Plot Format" : props.title)
            .withOnApply([&editor, plot]{ editor.apply(); plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

bool graphFormatDlg(QCPGraph* graph, const GraphFormatDlgProps& props)
{
    GraphFormatWidget editor(graph);

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withOnApply([&editor, &props]{ editor.apply(); props.plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

//------------------------------------------------------------------------------
//                            QCPL::TextProcessor
//------------------------------------------------------------------------------

void TextProcessor::addVar(const QString& name, TextVarGetter getter)
{
    _vars.append({name, getter});
}

QString TextProcessor::process(const QString& text) const
{
    QString str(text);
    foreach (const auto& var, _vars)
    {
        int pos = str.indexOf(var.name);
        if (pos >= 0)
        {
            QString value = var.getter();
            while (pos >= 0)
            {
                str.replace(pos, var.name.length(), value);
                pos = str.indexOf(var.name, pos + value.length());
            }
        }
    }
    return str;
}

//------------------------------------------------------------------------------
//                           QCPL::TextFormatterBase
//------------------------------------------------------------------------------

void TextFormatterBase::addVar(const QString& name, const QString& descr, TextVarGetter getter)
{
    _vars.append({name, descr});
    _processor.addVar(name, getter);
}

//------------------------------------------------------------------------------
//                          QCPL::AxisTitleFormatter
//------------------------------------------------------------------------------

void AxisTextFormatter::format()
{
    _axis->setLabel(_processor.process(_text).trimmed());
}

//------------------------------------------------------------------------------
//                          QCPL::TitleTextFormatter
//------------------------------------------------------------------------------

void TitleTextFormatter::format()
{
    _title->setText(_processor.process(_text).trimmed());
}

} // namespace QCPL
