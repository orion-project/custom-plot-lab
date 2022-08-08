#include "qcpl_format_editors.h"

#include "widgets/OriLabels.h"
#include "helpers/OriLayouts.h"
#include "widgets/OriMenuToolButton.h"

#include <QAction>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>
#include <QToolButton>
#include <QtMath>

namespace QCPL {

//---------------------------------------------------------------------
//                         Helpers
//---------------------------------------------------------------------

QWidget* makeLabelSeparator(const QString& title)
{
    auto s = new Ori::Widgets::LabelSeparator(title);
    QFont f = s->font();
    f.setBold(true);
    s->flat = true;
    s->setFont(f);
    return s;
}

QWidget* makeParamLabel(const QString& name, const QString& hint)
{
    // TODO: take some color from palette, don't use hardcoded 'gray'
    return new QLabel(QStringLiteral("%1 <span style='color:gray'>(%2)</span>  ").arg(name, hint));
}

//---------------------------------------------------------------------
//                         MarginsEditorWidget
//---------------------------------------------------------------------

MarginsEditorWidget::MarginsEditorWidget(const QString& title) : QGroupBox(title)
{
    L = new QSpinBox;
    T = new QSpinBox;
    R = new QSpinBox;
    B = new QSpinBox;

    auto layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->addWidget(T, 0, 1);
    layout->addWidget(L, 1, 0);
    layout->addWidget(R, 1, 2);
    layout->addWidget(B, 2, 1);
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

QPixmap makeSolidColorIcon(const QBrush &b, const QSize &sz)
{
    int w, h;
    if (sz.isEmpty())
        w = 24, h = 24;
    else
        w = sz.width(), h = sz.height();
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(b.color());
    p.setBrush(b);
    p.drawRect(5, 5, w-10, h-10);
    return pixmap;
}

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
//                            ColorButton
//---------------------------------------------------------------------

ColorButton::ColorButton(QWidget* parent) : QToolButton(parent)
{
    connect(this, &QToolButton::clicked, this, &ColorButton::selectColor);
}

void ColorButton::setValue(const QColor& c)
{
    _color = c;
    setIcon(makeSolidColorIcon(c, iconSize()));
}

void ColorButton::selectColor()
{
    QColorDialog dlg;
    dlg.setCurrentColor(_color);
    if (dlg.exec())
        setValue(dlg.selectedColor());
}

//---------------------------------------------------------------------
//                         PenEditorWidget
//---------------------------------------------------------------------

PenEditorWidget::PenEditorWidget(PenEditorWidgetOptions opts, QWidget *parent) : QWidget(parent)
{
    _style = new Ori::Widgets::MenuToolButton;
    _style->setIconSize({50, 16});
    createPenAction(Qt::NoPen, "No line");
    createPenAction(Qt::SolidLine, "Solid");
    createPenAction(Qt::DashLine, "Dashed"),
    createPenAction(Qt::DotLine, "Dotted");
    createPenAction(Qt::DashDotLine, "Dash-dotted");
    createPenAction(Qt::DashDotDotLine, "Dash-dot-dotted");

    _width = new QSpinBox;

    _color = new ColorButton;
    _color->setIconSize({40, 16});

    auto labelStyle = new QLabel(opts.labelStyle.isEmpty() ? tr("Style:") : opts.labelStyle);
    auto labelWidth = new QLabel(tr("Width:"));
    auto labelColor = new QLabel(tr("Color:"));

    if (opts.gridLayout)
    {
        opts.gridLayout->addWidget(labelStyle, opts.gridRow, 0);
        opts.gridLayout->addWidget(_style, opts.gridRow, 1);
        opts.gridLayout->addWidget(labelWidth, opts.gridRow, 2);
        opts.gridLayout->addWidget(_width, opts.gridRow, 3);
        opts.gridLayout->addWidget(labelColor, opts.gridRow, 4);
        opts.gridLayout->addWidget(_color, opts.gridRow, 5);
    }
    else
    {
        Ori::Layouts::LayoutH({
                                  labelStyle,
                                  _style,
                                  Ori::Layouts::Space(10),
                                  labelWidth,
                                  _width,
                                  Ori::Layouts::Space(10),
                                  labelColor,
                                  _color,
                              }).setMargin(0).useFor(this);
    }
}

void PenEditorWidget::createPenAction(Qt::PenStyle style, const QString& title)
{
    auto sz = _style->iconSize();
    int w = sz.width();
    int h = sz.height();

    QPixmap pixmap(sz);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    if (style == Qt::NoPen)
    {
        auto p1 = QIcon(":/qcpl_images/style_empty").pixmap(h, h);
        p.drawPixmap(QRect((w-h)/2, 0, h, h), p1, QRect(0, 0, h, h));
    }
    else
    {
        QPen pen;
        pen.setWidth(3);
        pen.setStyle(style);
        p.setPen(pen);
        p.drawLine(0, h/2, w, h/2);
    }
    _style->addAction(style, new QAction(pixmap, title, this));
}

void PenEditorWidget::setValue(const QPen& p)
{
    _pen = p;
    _color->setValue(p.color());
    _width->setValue(p.width());
    _style->setSelectedId(p.style() == Qt::CustomDashLine ? Qt::SolidLine : p.style());
}

QPen PenEditorWidget::value() const
{
    QPen p(_pen);
    p.setColor(_color->value());
    p.setWidth(_width->value());
    p.setStyle(Qt::PenStyle(_style->selectedId()));
    p.setJoinStyle(Qt::MiterJoin);
    return p;
}

} // namespace QCPL
