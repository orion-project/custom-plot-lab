#include "qcpl_format_editors.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriMenuToolButton.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QSpinBox>
#include <QToolButton>

namespace QCPL {

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

static QPixmap makeColorIcon(const QBrush &b)
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(b.color());
    p.setBrush(b);
    p.drawRect(5, 5, 14, 14);
    return pixmap;
}

//---------------------------------------------------------------------
//                         PenEditorWidget
//---------------------------------------------------------------------

PenEditorWidget::PenEditorWidget(QWidget *parent) : QWidget(parent)
{
    // TODO: add style icons
    _btnStyle = new Ori::Widgets::MenuToolButton;
    _btnStyle->addAction(Qt::NoPen, "No line");
    _btnStyle->addAction(Qt::SolidLine, "Solid");
    _btnStyle->addAction(Qt::DashLine, "Dashed"),
    _btnStyle->addAction(Qt::DotLine, "Dotted");
    _btnStyle->addAction(Qt::DashDotLine, "Dash-dotted");
    _btnStyle->addAction(Qt::DashDotDotLine, "Dash-dot-dotted");

    _width = new QSpinBox;

    _btnColor = new QToolButton;
    connect(_btnColor, &QToolButton::clicked, this, &PenEditorWidget::selectColor);

    Ori::Layouts::LayoutH({
                              new QLabel(tr("Style:")),
                              _btnStyle,
                              Ori::Layouts::Space(10),
                              new QLabel(tr("Width:")),
                              _width,
                              Ori::Layouts::Space(10),
                              new QLabel(tr("Color:")),
                              _btnColor,
                          }).useFor(this);
}

void PenEditorWidget::setValue(const QPen& p)
{
    _pen = p;
    setColor(p.color());
    _width->setValue(p.width());
    _btnStyle->setSelectedId(p.style() == Qt::CustomDashLine ? Qt::SolidLine : p.style());
}

QPen PenEditorWidget::value() const
{
    QPen p(_pen);
    p.setColor(_color);
    p.setWidth(_width->value());
    p.setStyle(Qt::PenStyle(_btnStyle->selectedId()));
    return p;
}

void PenEditorWidget::selectColor()
{
    QColorDialog dlg;
    dlg.setCurrentColor(_color);
    if (dlg.exec())
        setColor(dlg.selectedColor());
}

void PenEditorWidget::setColor(QColor c)
{
    _color = c;
    _btnColor->setIcon(QIcon(makeColorIcon(c)));
}

} // namespace QCPL
