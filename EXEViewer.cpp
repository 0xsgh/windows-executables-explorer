
#include "EXEViewer.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace
{
    void
    setUpDOSHeaderWidgets( EXEFile const& loadedEXEFile,
                           QGroupBox* dosHeaderWidgetsContainer );

    void
    setUpNTFileHeaderWidgets( EXEFile const& loadedEXEFile,
                              QGroupBox* ntFileHeaderWidgetsContainer );

    void
    setUpNTOptionalHeaderWidgets( EXEFile const& loadedEXEFile,
                                  QGroupBox* ntOptionalHeaderWidgetsContainer );
}

EXEViewer::EXEViewer( EXEFile&& loadedEXEFile,
                      QWidget* parentWidget )
: QTabWidget( parentWidget )
, m_loadedEXEFile( std::move( loadedEXEFile ) )
{
    setUpRawDataTab();
    setUpHeadersTab();
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

void
EXEViewer::setUpHeadersTab()
{
    auto headersTabRootWidget = new QWidget;
    addTab( headersTabRootWidget, "Headers" );

    auto headersTabMainLayout = new QVBoxLayout( headersTabRootWidget );

    auto dosHeaderWidgetsContainer = new QGroupBox( "DOS Header" );
    headersTabMainLayout->addWidget( dosHeaderWidgetsContainer );

    auto ntFileHeaderWidgetsContainer = new QGroupBox( "NT File Header" );
    headersTabMainLayout->addWidget( ntFileHeaderWidgetsContainer );

    auto ntOptionalHeaderWidgetsContainer = new QGroupBox( "NT Optional Header" );
    headersTabMainLayout->addWidget( ntOptionalHeaderWidgetsContainer );

    headersTabMainLayout->addStretch();

    setUpDOSHeaderWidgets( m_loadedEXEFile, dosHeaderWidgetsContainer );
    setUpNTFileHeaderWidgets( m_loadedEXEFile, ntFileHeaderWidgetsContainer );
    setUpNTOptionalHeaderWidgets( m_loadedEXEFile, ntOptionalHeaderWidgetsContainer );
}

namespace
{
    void
    setUpDOSHeaderWidgets( EXEFile const& loadedEXEFile,
                           QGroupBox* dosHeaderWidgetsContainer )
    {
        auto dosHeaderWidgetsLayout = new QVBoxLayout( dosHeaderWidgetsContainer );

        auto offsetOfNTFileHeaderLabel =
            new QLabel( QString( "NT File Header offset: %1" )
                            .arg( loadedEXEFile.dosHeader.offsetOfNTFileHeader ) );
        dosHeaderWidgetsLayout->addWidget( offsetOfNTFileHeaderLabel );
    }

    void
    setUpNTFileHeaderWidgets( EXEFile const& loadedEXEFile,
                              QGroupBox* ntFileHeaderWidgetsContainer )
    {
        auto ntFileHeaderWidgetsLayout = new QVBoxLayout( ntFileHeaderWidgetsContainer );

        auto optionalHeaderSizeLabel =
            new QLabel( QString( "Size of optional header: %1" )
                            .arg( loadedEXEFile.ntFileHeader.sizeOfOptionalHeader ) );
        ntFileHeaderWidgetsLayout->addWidget( optionalHeaderSizeLabel );
    }

    void
    setUpNTOptionalHeaderWidgets( EXEFile const& loadedEXEFile,
                                  QGroupBox* ntOptionalHeaderWidgetsContainer )
    {
        auto ntOptionalHeaderWidgetsLayout =
            new QVBoxLayout( ntOptionalHeaderWidgetsContainer );

        auto const peSignatureHexString =
            QString( "%1" ).arg( loadedEXEFile.ntOptionalHeader.peSignature,
                                 0, 16, QChar( '0' ) ).toUpper();
        auto peSignatureLabel =
            new QLabel( QString( "PE signature: 0x%1" ).arg( peSignatureHexString ) );
        ntOptionalHeaderWidgetsLayout->addWidget( peSignatureLabel );
    }
}