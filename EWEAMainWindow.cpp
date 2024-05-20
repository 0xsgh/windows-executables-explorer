
#include "EWEAMainWindow.h"

#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMimeData>
#include <QSplitter>

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
        for ( const auto& url : dropEvent->mimeData()->urls() )
        {
            m_loadedFilesList->addItem( url.toLocalFile() );
        }
    }
}