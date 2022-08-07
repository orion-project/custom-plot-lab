#ifndef QCPL_FORMAT_EDITORS_H
#define QCPL_FORMAT_EDITORS_H

#include <QGroupBox>
#include <QPen>

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QSpinBox;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
}}

namespace QCPL {

class MarginsEditorWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit MarginsEditorWidget(const QString& title);

    void setValue(const QMargins& m);
    QMargins value() const;

private:
    QSpinBox *L, *T, *R, *B;
};

struct PenEditorWidgetOptions
{
    QString labelStyle;
    QGridLayout *gridLayout = nullptr;
    int gridRow = 0;
};

class PenEditorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit PenEditorWidget(PenEditorWidgetOptions opts = PenEditorWidgetOptions(), QWidget *parent = nullptr);

    void setValue(const QPen& p);
    QPen value() const;

private:
    QPen _pen;
    QSpinBox *_width;
    QColor _color;
    QToolButton *_btnColor;
    Ori::Widgets::MenuToolButton *_btnStyle;

    void createPenAction(Qt::PenStyle style, const QString& title);

    void selectColor();
    void setColor(QColor c);
};

QWidget* makeLabelSeparator(const QString& title);
QWidget* makeParamLabel(const QString& name, const QString& hint);
QPixmap makeSolidColorIcon(const QBrush &b, const QSize &sz = QSize());
QPixmap makePenIcon(const QPen& pen, const QSize& sz = QSize());

} // namespace QCPL

#endif // QCPL_FORMAT_EDITORS_H
