
#include "EXEFile.h"
#include "EXEViewer.h"

#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTableWidget>
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
    setUpSectionHeadersTab();
    setUpImportsTab();
    setUpExportsTab();
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

void
EXEViewer::setUpSectionHeadersTab()
{
    auto sectionHeadersTabRootWidget = new QWidget;
    addTab( sectionHeadersTabRootWidget, "Section Headers" );

    auto sectionHeadersTabMainLayout = new QGridLayout( sectionHeadersTabRootWidget );

    auto const& sectionHeadersNameToInfo = m_loadedEXEFile.sectionHeadersNameToInfo;
    auto currentRow = 0;
    auto currentColumn = 0;

    for ( auto const& [sectionName, sectionHeader] : sectionHeadersNameToInfo )
    {
        auto sectionHeaderWidgetsContainer =
            new QGroupBox( QString::fromStdString( sectionName ) );
        sectionHeadersTabMainLayout->addWidget( sectionHeaderWidgetsContainer,
                                                currentRow,
                                                currentColumn );
        currentColumn++;
        if ( currentColumn >= 2 )
        {
            currentColumn = 0;
            currentRow++;
        }

        auto sectionHeaderWidgetsLayout = new QVBoxLayout( sectionHeaderWidgetsContainer );

        auto sectionSizeLabel =
            new QLabel( QString( "Size in memory: %1" )
                            .arg( sectionHeader.sectionSizeInBytesInMemory ) );
        sectionHeaderWidgetsLayout->addWidget( sectionSizeLabel );

        auto const sectionBaseAddressAsHexString =
            QString( "%1" ).arg( sectionHeader.sectionBaseAddressInMemory,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto sectionBaseAddressLabel =
            new QLabel( QString( "Base address in memory: 0x%1" )
                            .arg( sectionBaseAddressAsHexString ) );
        sectionHeaderWidgetsLayout->addWidget( sectionBaseAddressLabel );

        auto sizeOfRawDataLabel =
            new QLabel( QString( "Size of raw data: %1" )
                            .arg( sectionHeader.sizeOfRawDataInBytes ) );
        sectionHeaderWidgetsLayout->addWidget( sizeOfRawDataLabel );

        auto const pointerToRawDataAsHexString =
            QString( "%1" ).arg( sectionHeader.pointerToRawData,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto pointerToRawDataLabel =
            new QLabel( QString( "Pointer to raw data: 0x%1" )
                            .arg( pointerToRawDataAsHexString ) );
        sectionHeaderWidgetsLayout->addWidget( pointerToRawDataLabel );

        auto const pointerToRelocationsAsHexString =
            QString( "%1" ).arg( sectionHeader.pointerToRelocations,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto pointerToRelocationsLabel =
            new QLabel( QString( "Pointer to relocations: 0x%1" )
                            .arg( pointerToRelocationsAsHexString ) );
        sectionHeaderWidgetsLayout->addWidget( pointerToRelocationsLabel );

        auto const pointerToLineNumbersAsHexString =
            QString( "%1" ).arg( sectionHeader.pointerToLineNumbers,
                                 8, 16, QChar( '0' ) ).toUpper();
        auto pointerToLineNumbersLabel =
            new QLabel( QString( "Pointer to line numbers: 0x%1" )
                            .arg( pointerToLineNumbersAsHexString ) );
        sectionHeaderWidgetsLayout->addWidget( pointerToLineNumbersLabel );

        auto numberOfRelocationsLabel =
            new QLabel( QString( "Number of relocations: %1" )
                            .arg( sectionHeader.numberOfRelocations ) );
        sectionHeaderWidgetsLayout->addWidget( numberOfRelocationsLabel );

        auto numberOfLineNumberEntriesLabel =
            new QLabel( QString( "Number of line number entries: %1" )
                            .arg( sectionHeader.numberOfLineNumberEntries ) );
        sectionHeaderWidgetsLayout->addWidget( numberOfLineNumberEntriesLabel );
    }
}

void
EXEViewer::setUpImportsTab()
{
    auto importsTabRootWidget = new QWidget;
    addTab( importsTabRootWidget, "Imports" );

    auto importsTabMainLayout = new QVBoxLayout( importsTabRootWidget );

    auto importedDLLsViewerContainer = new QGroupBox( "Imported DLLs" );
    importsTabMainLayout->addWidget( importedDLLsViewerContainer );

    auto importedFunctionsViewerContainer = new QGroupBox( "Imported Functions" );
    importsTabMainLayout->addWidget( importedFunctionsViewerContainer );

    auto importedDLLsViewerLayout = new QVBoxLayout( importedDLLsViewerContainer );
    auto importedFunctionsViewerLayout = new QVBoxLayout( importedFunctionsViewerContainer );

    auto importedDLLsViewer = new QListWidget;
    importedDLLsViewerLayout->addWidget( importedDLLsViewer );

    auto importedFunctionsViewer = new QListWidget;
    importedFunctionsViewerLayout->addWidget( importedFunctionsViewer );

    for ( auto const& [importedDLLName, importedFunctions] : m_loadedEXEFile.importedDLLToImportedFunctions )
    {
        importedDLLsViewer->addItem( QString::fromStdString( importedDLLName ) );

        for ( auto const& importedFunction : importedFunctions )
        {
            importedFunctionsViewer->addItem( QString::fromStdString( importedFunction ) );
        }
    }
}

void
EXEViewer::setUpExportsTab()
{
    auto exportedFunctionsViewerContainer = new QGroupBox( "Exported Functions" );
    addTab( exportedFunctionsViewerContainer, "Exports" );

    auto exportedFunctionsViewerLayout = new QVBoxLayout( exportedFunctionsViewerContainer );

    auto exportedFunctionsViewer = new QTableWidget;
    exportedFunctionsViewerLayout->addWidget( exportedFunctionsViewer );

    exportedFunctionsViewer->setColumnCount( 1 );
    exportedFunctionsViewer->setHorizontalHeaderLabels( { "Function Name" } );

    exportedFunctionsViewer->setRowCount( m_loadedEXEFile.exportedFunctions.size() );

    for ( auto row = 0; auto const& exportedFunction : m_loadedEXEFile.exportedFunctions )
    {
        auto tableEntry = new QTableWidgetItem( QString::fromStdString( exportedFunction.name ) );
        tableEntry->setFlags( tableEntry->flags() & ~Qt::ItemIsEditable );

        exportedFunctionsViewer->setItem( row++, 0, tableEntry );
    }

    exportedFunctionsViewer->resizeColumnsToContents();
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

        auto numberOfSectionsLabel =
            new QLabel( QString( "Number of sections: %1" )
                            .arg( ntFileHeader.numberOfSections ) );
        ntFileHeaderWidgetsLayout->addWidget( numberOfSectionsLabel );
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

        auto numberOfDataDirectoriesLabel =
            new QLabel( QString( "Number of data directories: %1" )
                            .arg( optionalHeader.numberOfDataDirectories ) );
        ntOptionalHeaderWidgetsLayout->addWidget( numberOfDataDirectoriesLabel );
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