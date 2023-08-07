#ifndef QCPL_FORMAT_TITLE_H
#define QCPL_FORMAT_TITLE_H

#include <QWidget>
#include <QJsonObject>

class QCPTextElement;

QT_BEGIN_NAMESPACE
class QCheckBox;
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
    void restore();

private:
    QCPTextElement *_title;
    QJsonObject _backup;
    TextEditorWidget *_textProps;
    QCheckBox *_visible, *_saveDefault;
    MarginsEditorWidget *_margins;
};

} // namespace QCPL

#endif // QCPL_FORMAT_TITLE_H
