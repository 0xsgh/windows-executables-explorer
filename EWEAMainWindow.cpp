
#include "EWEAMainWindow.h"
#include "EXEFile.h"
#include "EXEViewer.h"

#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QListWidget>
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

    m_loadedFilesList = new QListWidget;
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
                 not pathOfExecutableFile.endsWith( ".dll" ) )
            {
                continue;
            }

            auto newListItem = new QListWidgetItem( pathOfExecutableFile );
            newListItem->setToolTip( pathOfExecutableFile );

            if ( pathOfExecutableFile.endsWith( ".exe" ) or
                 pathOfExecutableFile.endsWith( ".dll" ) )
            {
                auto loadedEXEFile = loadEXEFile( pathOfExecutableFile.toStdString() );
                m_artifactViewersStack->addWidget( new EXEViewer( std::move( loadedEXEFile ) ) );
            }

            m_loadedFilesList->addItem( newListItem );
        }
    }
}