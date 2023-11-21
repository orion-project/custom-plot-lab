#ifndef QCPL_FORMAT_EDITORS_H
#define QCPL_FORMAT_EDITORS_H

#include <QGroupBox>
#include <QPen>
#include <QToolButton>

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QSpinBox;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
class BaseColorButton;
}}

namespace QCPL {

//---------------------------------------------------------------------
//                         MarginsEditorWidget
//---------------------------------------------------------------------

class MarginsEditorWidget : public QGroupBox
{
    Q_OBJECT

public:
    struct Options
    {
        bool layoutInLine = false;
    };

    explicit MarginsEditorWidget(const QString& title, const Options& opts);

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
    bool narrow = false;
    bool noLabels = false;
};

//---------------------------------------------------------------------
//                         PenEditorWidget
//---------------------------------------------------------------------

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
    Ori::Widgets::BaseColorButton *_color;
    Ori::Widgets::MenuToolButton *_style;

    void createPenAction(Qt::PenStyle style, const QString& title);
};

//---------------------------------------------------------------------
//                              Helpers
//---------------------------------------------------------------------

QSpinBox* makeSpinBox(int min, int max);
QWidget* makeSeparator();
QWidget* makeLabelSeparator(const QString& title);
QWidget* makeDialogHeader();
QWidget* makeParamLabel(const QString& name, const QString& hint, const QString &hintColor);
QPixmap makePenIcon(const QPen& pen, const QSize& sz = QSize());

} // namespace QCPL

#endif // QCPL_FORMAT_EDITORS_H
