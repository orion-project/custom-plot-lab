#ifndef QCPL_CURSOR_PANEL_H
#define QCPL_CURSOR_PANEL_H

#include <QTextBrowser>

QT_BEGIN_NAMESPACE
class QToolBar;
QT_END_NAMESPACE

namespace QCPL {

class Cursor;

class CursorPanel : public QTextBrowser
{
    Q_OBJECT

public:
    enum Mode { Both, Vertical, Horizontal };
    Q_ENUM(Mode)

public:
    explicit CursorPanel(Cursor *cursor);

    void update();
    void update(const QString& info);
    void placeIn(QToolBar* toolbar);
    void fillMenu(QMenu *menu);

    void setNumberPrecision(int value, bool update);
    QString formatLinkX(const QString& x) const;
    QString formatLinkY(const QString& y) const;

    bool autoUpdateInfo() const { return _autoUpdateInfo; }
    void setAutoUpdateInfo(bool v) { _autoUpdateInfo = v; }

    Mode mode() const;
    void setMode(Mode mode);
    bool enabled() const;
    void setEnabled(bool on);

protected:
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private:
    Cursor *_cursor;
    QAction *actnCursorFollow, *actnCursorSetX, *actnCursorSetY,
        *actnShowCursor, *actnCursorVert, *actnCursorHorz, *actnCursorBoth;
    bool _autoUpdateInfo = true;
    int _numberPrecision = 6;

    void createActions();
    QString formatCursorInfo() const;

private slots:
    void linkClicked(const class QUrl&);
    void cursorPositionCanged();
    void setCursorX();
    void setCursorY();
    void setCursorShape();
};

} // namespace QCPL

#endif // QCPL_CURSOR_PANEL_H
