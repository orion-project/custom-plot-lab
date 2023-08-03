#ifndef QCPL_FORMAT_TITLE_H
#define QCPL_FORMAT_TITLE_H

#include <QWidget>

class QCPTextElement;

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGridLayout;
class QSpinBox;
QT_END_NAMESPACE

namespace QCPL {

class MarginsEditorWidget;
class TextEditorWidget;
struct TitleFormatDlgProps;

class TitleFormatWidget : public QWidget
{
    Q_OBJECT

public:
    TitleFormatWidget(QCPTextElement* title, const TitleFormatDlgProps& props);

public slots:
    void apply();

private:
    QCPTextElement *_title;
    TextEditorWidget *_textProps;
    QCheckBox *_visible, *_saveDefault;
    MarginsEditorWidget *_margins;
    std::function<void()> _onSaveDefault;
};

} // namespace QCPL

#endif // QCPL_FORMAT_TITLE_H
