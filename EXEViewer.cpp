
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

    void
    setUpDataDirectoryWidgets( EXEFile const& loadedEXEFile,
                               QGroupBox* dataDirectoryWidgetsContainer );
}

EXEViewer::EXEViewer( EXEFile&& loadedEXEFile,
                      QWidget* parentWidget )
: QTabWidget( parentWidget )
, m_loadedEXEFile( std::move( loadedEXEFile ) )
{
    setUpRawDataTab();
    setUpFileHeadersTab();
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
EXEViewer::setUpFileHeadersTab()
{
    auto headersTabRootWidget = new QWidget;
    addTab( headersTabRootWidget, "File Headers" );

    auto headersTabMainLayout = new QVBoxLayout( headersTabRootWidget );

    auto dosHeaderWidgetsContainer = new QGroupBox( "DOS Header" );
    headersTabMainLayout->addWidget( dosHeaderWidgetsContainer );

    auto ntFileHeaderWidgetsContainer = new QGroupBox( "NT File Header" );
    headersTabMainLayout->addWidget( ntFileHeaderWidgetsContainer );

    auto ntOptionalHeaderWidgetsContainer = new QGroupBox( "NT Optional Header" );
    headersTabMainLayout->addWidget( ntOptionalHeaderWidgetsContainer );

    auto dataDirectoryWidgetsContainer = new QGroupBox( "Data Directories" );
    headersTabMainLayout->addWidget( dataDirectoryWidgetsContainer );

    headersTabMainLayout->addStretch();

    setUpDOSHeaderWidgets( m_loadedEXEFile, dosHeaderWidgetsContainer );
    setUpNTFileHeaderWidgets( m_loadedEXEFile, ntFileHeaderWidgetsContainer );
    setUpNTOptionalHeaderWidgets( m_loadedEXEFile, ntOptionalHeaderWidgetsContainer );
    setUpDataDirectoryWidgets( m_loadedEXEFile, dataDirectoryWidgetsContainer );
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

        auto const& ntFileHeader = loadedEXEFile.ntFileHeader;

        auto optionalHeaderSizeLabel =
            new QLabel( QString( "Size of optional header: %1" )
                            .arg( ntFileHeader.sizeOfOptionalHeader ) );
        ntFileHeaderWidgetsLayout->addWidget( optionalHeaderSizeLabel );

        auto const targetMachineArchitectureHexString =
            QString( "%1" ).arg( ntFileHeader.targetMachineArchitecture,
                                 4, 16, QChar( '0' ) ).toUpper();
        auto const targetMachineArchitectureName =
            QString::fromStdString( getMachineArchitectureName( ntFileHeader.targetMachineArchitecture ) );
        auto targetMachineArchitectureLabel =
            new QLabel( QString( "Target machine architecture: 0x%1 (%2)" )
                            .arg( targetMachineArchitectureHexString )
                            .arg( targetMachineArchitectureName ) );
        ntFileHeaderWidgetsLayout->addWidget( targetMachineArchitectureLabel );
    }

    void
    setUpNTOptionalHeaderWidgets( EXEFile const& loadedEXEFile,
                                  QGroupBox* ntOptionalHeaderWidgetsContainer )
    {
        auto ntOptionalHeaderWidgetsLayout =
            new QVBoxLayout( ntOptionalHeaderWidgetsContainer );

        auto const& optionalHeader = loadedEXEFile.ntOptionalHeader;

        auto const peSignatureHexString =
            QString( "%1" ).arg( optionalHeader.peSignature,
                                 0, 16, QChar( '0' ) ).toUpper();
        auto const peSignatureName =
            QString::fromStdString( getPESignatureName( optionalHeader.peSignature ) );
        auto peSignatureLabel =
            new QLabel( QString( "PE signature: 0x%1 (%2)" )
                            .arg( peSignatureHexString )
                            .arg( peSignatureName ) );
        ntOptionalHeaderWidgetsLayout->addWidget( peSignatureLabel );

        auto linkerVersionLabel =
            new QLabel( QString( "Linker version: %1.%2" )
                            .arg( optionalHeader.linkerMajorVersion )
                            .arg( optionalHeader.linkerMinorVersion ) );
        ntOptionalHeaderWidgetsLayout->addWidget( linkerVersionLabel );

        auto sizeOfCodeLabel =
            new QLabel( QString( "Size of code: %1" )
                            .arg( optionalHeader.sizeOfCodeInBytes ) );
        ntOptionalHeaderWidgetsLayout->addWidget( sizeOfCodeLabel );

        auto sizeOfInitializedDataLabel =
            new QLabel( QString( "Size of initialized data: %1" )
                            .arg( optionalHeader.sizeOfInitializedDataInBytes ) );
        ntOptionalHeaderWidgetsLayout->addWidget( sizeOfInitializedDataLabel );

        auto sizeOfUninitializedDataLabel =
            new QLabel( QString( "Size of uninitialized data: %1" )
                            .arg( optionalHeader.sizeOfUninitializedDataInBytes ) );
        ntOptionalHeaderWidgetsLayout->addWidget( sizeOfUninitializedDataLabel );

        auto const entryPointAddressHexString =
            QString( "%1" ).arg( optionalHeader.addressOfEntryPoint,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto addressOfEntryPointLabel =
            new QLabel( QString( "Address of entry point: 0x%1" )
                            .arg( entryPointAddressHexString ) );
        ntOptionalHeaderWidgetsLayout->addWidget( addressOfEntryPointLabel );

        auto const baseOfCodeAddressHexString =
            QString( "%1" ).arg( optionalHeader.addressOfBaseOfCode,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto addressOfBaseOfCodeLabel =
            new QLabel( QString( "Address of base of code: 0x%1" )
                            .arg( baseOfCodeAddressHexString ) );
        ntOptionalHeaderWidgetsLayout->addWidget( addressOfBaseOfCodeLabel );

        auto const imageBaseAddressHexString =
            QString( "%1" ).arg( optionalHeader.preferredBaseAddressOfImage,
                                 16, 16, QChar( '0' ) ).toUpper();
        auto preferredBaseAddressOfImageLabel =
            new QLabel( QString( "Preferred base address of image: 0x%1" )
                            .arg( imageBaseAddressHexString ) );
        ntOptionalHeaderWidgetsLayout->addWidget( preferredBaseAddressOfImageLabel );
    }

    void
    setUpDataDirectoryWidgets( EXEFile const& loadedEXEFile,
                               QGroupBox* dataDirectoryWidgetsContainer )
    {
        auto dataDirectoryWidgetsLayout =
            new QVBoxLayout( dataDirectoryWidgetsContainer );

        auto const& dataDirectoryEntries = loadedEXEFile.dataDirectoryEntries;

        for ( auto i = 0; i < dataDirectoryEntries.size(); i++ )
        {
            auto const& dataDirectoryEntry = dataDirectoryEntries[ i ];

            auto const dataDirectoryDescription =
                QString::fromStdString( getImageDataDirectoryDescription( i ) );

            auto const dataDirectoryRVAHexString =
                QString( "%1" ).arg( dataDirectoryEntry.dataDirectoryRVA,
                                     8, 16, QChar( '0' ) ).toUpper();

            auto dataDirectoryEntryLabel =
                new QLabel( QString( "%1: %2 bytes @ 0x%3" )
                                .arg( dataDirectoryDescription )
                                .arg( dataDirectoryEntry.sizeInBytes )
                                .arg( dataDirectoryRVAHexString ) );
            dataDirectoryWidgetsLayout->addWidget( dataDirectoryEntryLabel );

            if (     dataDirectoryEntry.dataDirectoryRVA == 0
                 and dataDirectoryEntry.sizeInBytes == 0 )
            {
                dataDirectoryEntryLabel->setEnabled( false );
            }
        }
    }
}