
#include "EXEViewer.h"

#include <QApplication>
#include <QPlainTextEdit>

EXEViewer::EXEViewer( EXEFile&& loadedEXEFile,
                      QWidget* parentWidget )
: QTabWidget( parentWidget )
, m_loadedEXEFile( std::move( loadedEXEFile ) )
{
    setUpRawDataTab();
}

void
EXEViewer::setUpRawDataTab()
{
    auto rawDataViewer = new QPlainTextEdit;
    addTab( rawDataViewer, "Raw Data" );

    rawDataViewer->setReadOnly( true );

    auto rawDataAsText = QString();
    for ( auto i = 0; i < m_loadedEXEFile.rawBytes.size(); i++ )
    {
        rawDataAsText += QString( " %1" ).arg( m_loadedEXEFile.rawBytes[ i ],
                                               2, 16, QChar( '0' ) ).toUpper();

        if ( ( i + 1 ) % 24 == 0 )
        {
            rawDataViewer->appendPlainText( rawDataAsText );
            rawDataAsText.clear();
        }

        if ( ( i % 1024 ) == 0 )
        {
            QApplication::processEvents();
        }
    }
}