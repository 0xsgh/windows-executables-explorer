
#include "EXEViewer.h"

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
    for ( auto i = 0; i < 128; i++ )
    {
        rawDataAsText += QString( " FF" );

        if ( ( i + 1 ) % 24 == 0 )
        {
            rawDataViewer->appendPlainText( rawDataAsText );
            rawDataAsText.clear();
        }
    }
}