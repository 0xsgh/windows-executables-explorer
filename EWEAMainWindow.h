
#ifndef EWEAMAINWINDOW_H
#define EWEAMAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

class QDragEnterEvent;
class QListWidget;

class EWEAMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    EWEAMainWindow( QWidget* parentWidget = nullptr );

private:
    void
    dragEnterEvent( QDragEnterEvent* dragEnterEvent ) override;

    void
    dropEvent( QDropEvent* dropEvent ) override;

private:
    QPointer<QListWidget>    m_loadedFilesList;
};

#endif // EWEAMAINWINDOW_H