#include "qcpl_format_editors.h"

#include "widgets/OriLabels.h"
#include "helpers/OriLayouts.h"
#include "widgets/OriMenuToolButton.h"
#include "widgets/OriColorSelectors.h"

#include <QAction>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>
#include <QToolButton>
#include <QtMath>

using namespace Ori::Layouts;

namespace QCPL {

//---------------------------------------------------------------------
//                         Helpers
//---------------------------------------------------------------------

QSpinBox* makeSpinBox(int min, int max)
{
    auto sb = new QSpinBox;
    sb->setRange(min, max);
    return sb;
}

QWidget* makeSeparator()
{
    auto s = new QFrame();
    s->setFrameShape(QFrame::HLine);
    s->setLineWidth(0);
    s->setFixedHeight(1);
    return s;
}

QWidget* makeLabelSeparator(const QString& title)
{
    auto s = new Ori::Widgets::LabelSeparator(title);
    QFont f = s->font();
    f.setBold(true);
    s->flat = true;
    s->setFont(f);
    return s;
}

QWidget* makeDialogHeader()
{
    auto header = new QFrame();
    header->setProperty("qcpl_role", "dlg_header");
    header->setBackgroundRole(QPalette::Light);
    header->setAutoFillBackground(true);
    return header;
}

QWidget* makeParamLabel(const QString& name, const QString& hint, const QString& hintColor)
{
    return new QLabel(QString("%1 <span style='color:%3'>(%2)</span>   ").arg(name, hint, hintColor));
}

//---------------------------------------------------------------------
//                         MarginsEditorWidget
//---------------------------------------------------------------------

MarginsEditorWidget::MarginsEditorWidget(const QString& title, const Options &opts) : QGroupBox(title)
{
    L = makeSpinBox(0, 1000);
    T = makeSpinBox(0, 1000);
    R = makeSpinBox(0, 1000);
    B = makeSpinBox(0, 1000);

    if (opts.layoutInLine)
    {
        LayoutH({
            new QLabel("Left:"), L, Space(10),
            new QLabel("Top:"), T, Space(10),
            new QLabel("Right:"), R, Space(10),
            new QLabel("Bottom:"), B, Stretch(),
//                    new QLabel("←"), L, Space(10),
//                    new QLabel("↑"), T, Space(10),
//                    new QLabel("→"), R, Space(10),
//                    new QLabel("↓"), B, Stretch(),
        }).useFor(this);
    }
    else
    {
        auto layout = new QGridLayout(this);
        layout->setSpacing(0);
        layout->addWidget(T, 0, 1);
        layout->addWidget(L, 1, 0);
        layout->addWidget(R, 1, 2);
        layout->addWidget(B, 2, 1);
    }
}

void MarginsEditorWidget::setValue(const QMargins& m)
{
    T->setValue(m.top());
    R->setValue(m.right());
    B->setValue(m.bottom());
    L->setValue(m.left());
}

QMargins MarginsEditorWidget::value() const
{
    return {L->value(), T->value(), R->value(), B->value()};
}

//---------------------------------------------------------------------
//                              Helpers
//---------------------------------------------------------------------

QPixmap makePenIcon(const QPen& pen, const QSize &sz)
{
    int w, h;
    if (sz.isEmpty())
        w = 24, h = 24;
    else
        w = sz.width(), h = sz.height();
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    QPen pn = pen;
    pn.setJoinStyle(Qt::MiterJoin);
    p.setPen(pn);

    int pw = qCeil(pen.widthF()/2.0);
    if (pw == 0) pw = 1;
    p.drawRect(pw, pw, w-2*pw, h-2*pw);

    return pixmap;
}

//---------------------------------------------------------------------
//                         PenEditorWidget
//---------------------------------------------------------------------

PenEditorWidget::PenEditorWidget(PenEditorWidgetOptions opts, QWidget *parent) : QWidget(parent)
{
    _style = new Ori::Widgets::MenuToolButton;
    _style->setIconSize({ opts.narrow || opts.noLabels ? penIconWidthNarrow : penIconWidthNormal, iconHeight });
    if (opts.enableNoPen)
        createPenAction(Qt::NoPen, "No line");
    createPenAction(Qt::SolidLine, "Solid");
    createPenAction(Qt::DashLine, "Dashed"),
    createPenAction(Qt::DotLine, "Dotted");
    createPenAction(Qt::DashDotLine, "Dash-dotted");
    createPenAction(Qt::DashDotDotLine, "Dash-dot-dotted");

    _width = new QSpinBox;

    _color = new Ori::Widgets::ColorButton;
    _color->drawIconFrame = false;
    _color->setIconSize({ opts.narrow || opts.noLabels ? colorIconWidthNarrow : colorIconWidthNormal, iconHeight });

    QLabel *labelStyle, *labelWidth, *labelColor;
    if (!opts.noLabels)
    {
        labelStyle = new QLabel(opts.labelStyle.isEmpty() ? tr("Style:") : opts.labelStyle);
        labelWidth = new QLabel(tr("Width:"));
        labelColor = new QLabel(tr("Color:"));
    }

    if (opts.gridLayout and !opts.noLabels)
    {
        opts.gridLayout->addWidget(labelStyle, opts.gridRow, 0);
        opts.gridLayout->addWidget(_style, opts.gridRow, 1);
        opts.gridLayout->addWidget(labelWidth, opts.gridRow, 2);
        opts.gridLayout->addWidget(_width, opts.gridRow, 3);
        opts.gridLayout->addWidget(labelColor, opts.gridRow, 4);
        opts.gridLayout->addWidget(_color, opts.gridRow, 5);
    }
    else if (opts.narrow and !opts.noLabels)
    {
        LayoutH({
            LayoutV({ labelStyle, _style }).setSpacing(3).setMargin(0),
            LayoutV({ labelWidth, _width }).setSpacing(3).setMargin(0),
            LayoutV({ labelColor, _color }).setSpacing(3).setMargin(0),
        }).setSpacing(20).setMargin(0).useFor(this);
    }
    else if (opts.noLabels)
    {
        LayoutH({ _style, _width, _color }).setSpacing(20).setMargin(0).useFor(this);
    }
    else
    {
        LayoutH({
            labelStyle,
            _style,
            Space(10),
            labelWidth,
            _width,
            Space(10),
            labelColor,
            _color,
        }).setMargin(0).useFor(this);
    }
}

void PenEditorWidget::createPenAction(Qt::PenStyle style, const QString& title)
{
    auto sz = _style->iconSize();

    if (style == Qt::NoPen)
    {
        _style->addAction(style, new QAction(QIcon(":/qcpl_images/style_empty"), title, this));
        return;
    }

    QPixmap pixmap(sz);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    QPen pen;
    pen.setWidth(3);
    pen.setStyle(style);
    p.setPen(pen);
    p.drawLine(0, sz.height()/2, sz.width(), sz.height()/2);
    _style->addAction(style, new QAction(pixmap, title, this));
}

void PenEditorWidget::setValue(const QPen& p)
{
    _pen = p;
    _color->setSelectedColor(p.color());
    _width->setValue(p.width());
    _style->setSelectedId(p.style() == Qt::CustomDashLine ? Qt::SolidLine : p.style());
}

QPen PenEditorWidget::value() const
{
    QPen p(_pen);
    p.setColor(_color->selectedColor());
    p.setWidth(_width->value());
    p.setStyle(Qt::PenStyle(_style->selectedId()));
    p.setJoinStyle(Qt::MiterJoin);
    return p;
}

} // namespace QCPL
