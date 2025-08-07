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
    void placeIn(QToolBar* toolbar, const QList<QAction*> &auxActions = {});
    void fillMenu(QMenu *menu);

    void setNumberPrecision(int value, bool update);
    static QString formatLink(const QString& cmd, const QString& val);
    static QString formatLinkX(const QString& x);
    static QString formatLinkY(const QString& y);

    bool autoUpdateInfo() const { return _autoUpdateInfo; }
    void setAutoUpdateInfo(bool v) { _autoUpdateInfo = v; }

    Mode mode() const;
    void setMode(Mode mode);
    bool enabled() const;
    void setEnabled(bool on);

signals:
    void customCommandInvoked(const QString& cmd);

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
