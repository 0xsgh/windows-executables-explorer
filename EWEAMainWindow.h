
#ifndef EWEAMAINWINDOW_H
#define EWEAMAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

#include <map>
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

    void
    setUpLoadedFilesList();

    void
    unloadSelectedArtifacts();

private:
    QPointer<QListWidget>                m_loadedFilesList;
    QPointer<QStackedWidget>             m_artifactViewersStack;
    std::map<std::string, QTabWidget*>   m_artifactPathToViewerMap;
};

#endif // EWEAMAINWINDOW_H