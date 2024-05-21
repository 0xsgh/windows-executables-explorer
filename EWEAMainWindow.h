
#ifndef EWEAMAINWINDOW_H
#define EWEAMAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

#include <vector>

class QDragEnterEvent;
class QListWidget;
class QStackedWidget;
class QTabWidget;

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
    QPointer<QListWidget>                m_loadedFilesList;
    std::vector<QPointer<QTabWidget>>    m_executableArtifactViewers;
    QPointer<QStackedWidget>             m_artifactViewersStack;
};

#endif // EWEAMAINWINDOW_H