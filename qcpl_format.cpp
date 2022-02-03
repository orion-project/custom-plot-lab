#include "qcpl_format.h"
#include "qcpl_plot.h"
#include "qcpl_title_editor.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

namespace QCPL {

bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props)
{
    TitleEditorOptions opts;
    TitleEditor editor(opts);
    editor.setText(axis->label());
    editor.setFont(axis->labelFont());
    editor.setColor(axis->labelColor());

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("axis-title")
            .exec())
    {
        axis->setLabel(editor.text());
        axis->setLabelFont(editor.font());
        axis->setLabelColor(editor.color());
        axis->setSelectedLabelFont(editor.font());
        return true;
    }
    return false;
}

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
    layout->setMargin(0);
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

} // namespace QCPL
