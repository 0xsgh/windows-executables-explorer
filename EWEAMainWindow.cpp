
#include "EWEAMainWindow.h"
#include "EXEViewer.h"
#include "OBJViewer.h"
#include "PEFiles.h"

#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QSplitter>
#include <QStackedWidget>

#include <utility>

EWEAMainWindow::EWEAMainWindow( QWidget* parentWidget )
: QMainWindow( parentWidget )
{
    setCentralWidget( new QWidget( this ) );
    auto mainLayout = new QHBoxLayout( centralWidget() );

    auto topLevelSplitter = new QSplitter( Qt::Horizontal,
                                           centralWidget() );
    mainLayout->addWidget( topLevelSplitter );

    setUpLoadedFilesList();
    topLevelSplitter->addWidget( m_loadedFilesList );

    m_artifactViewersStack = new QStackedWidget;
    topLevelSplitter->addWidget( m_artifactViewersStack );

    topLevelSplitter->setStretchFactor( 1, 1 );

    setAcceptDrops( true );

    setWindowTitle( "EWEA" );

    resize( 1024, 768 );
}

void
EWEAMainWindow::dragEnterEvent( QDragEnterEvent* dragEnterEvent )
{
    if ( dragEnterEvent->mimeData()->hasUrls() )
    {
        dragEnterEvent->acceptProposedAction();
    }
}

void
EWEAMainWindow::dropEvent( QDropEvent* dropEvent )
{
    if ( dropEvent->mimeData()->hasUrls() )
    {
        for ( const auto& fileURL : dropEvent->mimeData()->urls() )
        {
            auto pathOfExecutableFile = fileURL.toLocalFile();

            if ( not pathOfExecutableFile.endsWith( ".exe" ) and
                 not pathOfExecutableFile.endsWith( ".dll" ) and
                 not pathOfExecutableFile.endsWith( ".obj" ) )
            {
                continue;
            }

            auto newListItem = new QListWidgetItem( pathOfExecutableFile );
            newListItem->setToolTip( pathOfExecutableFile );

            if ( pathOfExecutableFile.endsWith( ".exe" ) or
                 pathOfExecutableFile.endsWith( ".dll" ) )
            {
                auto loadedEXEFile = loadEXEFile( pathOfExecutableFile.toStdString() );
                auto artifactViewer = new EXEViewer( std::move( loadedEXEFile ) );

                m_artifactViewersStack->addWidget( artifactViewer );
                m_artifactPathToViewerMap[pathOfExecutableFile.toStdString()] = artifactViewer;
            }
            else if ( pathOfExecutableFile.endsWith( ".obj" ) )
            {
                auto loadedOBJFile = loadOBJFile( pathOfExecutableFile.toStdString() );
                auto artifactViewer = new OBJViewer( std::move( loadedOBJFile ) );

                m_artifactViewersStack->addWidget( artifactViewer );
                m_artifactPathToViewerMap[pathOfExecutableFile.toStdString()] = artifactViewer;
            }

            m_loadedFilesList->addItem( newListItem );
        }
    }
}

void
EWEAMainWindow::setUpLoadedFilesList()
{
    m_loadedFilesList = new QListWidget;

    m_loadedFilesList->setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( m_loadedFilesList, &QListWidget::itemActivated,
             [this]( QListWidgetItem* activatedItem )
             {
                auto pathOfExecutableFile = activatedItem->text().toStdString();

                if ( m_artifactPathToViewerMap.contains( pathOfExecutableFile ) )
                {
                    m_artifactViewersStack->setCurrentWidget( m_artifactPathToViewerMap.at( pathOfExecutableFile ) );
                }
             } );

    m_loadedFilesList->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_loadedFilesList, &QListWidget::customContextMenuRequested,
             [this]( QPoint const& mousePosition )
             {
                auto contextMenu = QMenu( this );

                auto unloadFilesAction = contextMenu.addAction( "Unload selection" );

                connect( unloadFilesAction, &QAction::triggered,
                         [this]()
                         {
                            auto userResponse =
                                QMessageBox::question( this,
                                                       "Unload selection",
                                                       "Are you sure you want to unload the selected files?" );

                            if ( userResponse == QMessageBox::Yes )
                            {
                                unloadSelectedArtifacts();
                            }
                         } );

                if ( m_loadedFilesList->selectedItems().empty() )
                {
                    unloadFilesAction->setEnabled( false );
                }

                contextMenu.exec( m_loadedFilesList->mapToGlobal( mousePosition ) );
             } );
}

void
EWEAMainWindow::unloadSelectedArtifacts()
{
    for ( const auto& selectedItem : m_loadedFilesList->selectedItems() )
    {
        auto pathOfExecutableFile = selectedItem->text().toStdString();

        if ( m_artifactPathToViewerMap.contains( pathOfExecutableFile ) )
        {
            auto artifactViewer =
                m_artifactPathToViewerMap.at( pathOfExecutableFile );

            artifactViewer->deleteLater();
            m_artifactPathToViewerMap.erase( pathOfExecutableFile );
        }

        delete selectedItem;
    }
}