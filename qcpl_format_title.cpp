#include "qcpl_format_title.h"

#include "qcpl_format.h"
#include "qcpl_format_editors.h"
#include "qcpl_io_json.h"
#include "qcpl_text_editor.h"
#include "qcpl_plot.h"

#include "helpers/OriLayouts.h"

#include <QAction>
#include <QDebug>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

TitleFormatWidget::TitleFormatWidget(QCPTextElement* title, const TitleFormatDlgProps& props) : QWidget(), _title(title)
{
    _backup = writeTitle(title);
    _backup["text"] = title->text();

    TextEditorWidget::Options textOpts;
    textOpts.showAlignment = true;
    if (props.formatter)
    {
        _formatter = props.formatter;
        textOpts.defaultText = props.defaultText;
        textOpts.vars = _formatter->vars();
        _backup["formatter_text"] = _formatter->text();
    }
    _textProps = new TextEditorWidget(textOpts);

    _visible = new QCheckBox(tr("Visible"));
    _saveDefault = new QCheckBox(tr("Save as default format"));
    _saveDefault->setVisible(bool(props.onSaveDefault));

    MarginsEditorWidget::Options marginOpts;
    marginOpts.layoutInLine = true;
    _margins = new MarginsEditorWidget(tr("Margins"), marginOpts);

    auto header = makeDialogHeader();
    LayoutH({
        SpaceH(),
        LayoutV({ Stretch(), _visible, _saveDefault }).setMargin(6),
    }).setMargin(0).useFor(header);

    LayoutV({
        header,
        makeSeparator(),
        LayoutV({
            LayoutV({_textProps}).makeGroupBox(tr("Text")),
            _margins,
        }).setDefSpacing().setDefMargins(),
    }).setSpacing(0).setMargin(0).useFor(this);

    _visible->setChecked(title->visible());
    if (props.formatter)
        _textProps->setText(props.formatter->text());
    else
        _textProps->setText(title->text());
    _textProps->setFont(title->font());
    _textProps->setColor(title->textColor());
    _textProps->setTextFlags(title->textFlags());
    _margins->setValue(title->margins());
}

void TitleFormatWidget::restore()
{
    readTitle(_backup, _title);
    _title->setText(_backup["text"].toString());
    if (_formatter)
        _formatter->setText(_backup["formatter_text"].toString());
    auto plot = qobject_cast<Plot*>(_title->parentPlot());
    if (plot) plot->updateTitleVisibility();
    _title->parentPlot()->replot();
}

void TitleFormatWidget::apply()
{
    bool wasEmpty = _title->text().isEmpty();
    bool wasVisible = _title->visible();

    if (_formatter)
    {
        _formatter->setText(_textProps->text());
        _formatter->format();
    }
    else _title->setText(_textProps->text());
    _title->setFont(_textProps->font());
    _title->setSelectedFont(_textProps->font());
    _title->setTextColor(_textProps->color());
    _title->setVisible(_visible->isChecked());
    _title->setTextFlags(_textProps->textFlags());
    _title->setMargins(_margins->value());

    bool isEmpty = _title->text().isEmpty();
    bool isVisible = _title->visible();
    if (wasEmpty && !isEmpty && !wasVisible &&!isVisible)
    {
        _title->setVisible(true);
        _visible->setChecked(true);
    }

    if (auto plot = qobject_cast<Plot*>(_title->parentPlot()); plot)
        plot->updateTitleVisibility();
    _title->parentPlot()->replot();
}

bool TitleFormatWidget::needSaveDefault() const
{
    return _saveDefault->isChecked();
}

} // namespace QCPL
