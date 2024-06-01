
#include "OBJViewer.h"

#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
    void
    setUpNTFileHeaderWidgets( PE::NTFileHeader const& ntFileHeader,
                              QGroupBox* ntFileHeaderWidgetsContainer );
}

OBJViewer::OBJViewer( OBJFile&& loadedOBJFile,
                      QWidget* parentWidget )
: QTabWidget( parentWidget )
, m_loadedOBJFile( std::move( loadedOBJFile ) )
{
    setUpFileHeadersTab();
    setUpSectionHeadersTab();
}

void
OBJViewer::setUpFileHeadersTab()
{
    auto headersTabRootWidget = new QWidget;
    addTab( headersTabRootWidget, "File Headers" );

    auto headersTabMainLayout = new QVBoxLayout( headersTabRootWidget );

    auto ntFileHeaderWidgetsContainer = new QGroupBox( "NT File Header" );
    headersTabMainLayout->addWidget( ntFileHeaderWidgetsContainer );

    headersTabMainLayout->addStretch();

    setUpNTFileHeaderWidgets( m_loadedOBJFile.ntFileHeader, ntFileHeaderWidgetsContainer );
}

void
OBJViewer::setUpSectionHeadersTab()
{
    auto sectionHeadersScrollWidget = new QScrollArea;
    addTab( sectionHeadersScrollWidget, "Section Headers" );

    auto sectionHeadersTabRootWidget = new QWidget;
    auto sectionHeadersTabMainLayout = new QGridLayout( sectionHeadersTabRootWidget );

    auto const& sectionHeadersNameToInfo = m_loadedOBJFile.sectionHeaders;
    auto currentRow = 0;
    auto currentColumn = 0;

    for ( auto const& [sectionName, sectionHeaders] : sectionHeadersNameToInfo )
    {
        for ( auto const& sectionHeader : sectionHeaders )
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

    sectionHeadersScrollWidget->setWidget( sectionHeadersTabRootWidget );
}

namespace
{
    void
    setUpNTFileHeaderWidgets( PE::NTFileHeader const& ntFileHeader,
                              QGroupBox* ntFileHeaderWidgetsContainer )
    {
        auto ntFileHeaderWidgetsLayout = new QVBoxLayout( ntFileHeaderWidgetsContainer );

        auto optionalHeaderSizeLabel =
            new QLabel( QString( "Size of optional header: %1" )
                            .arg( ntFileHeader.sizeOfOptionalHeader ) );
        ntFileHeaderWidgetsLayout->addWidget( optionalHeaderSizeLabel );

        auto const targetMachineArchitectureHexString =
            QString( "%1" ).arg( ntFileHeader.targetMachineArchitecture,
                                 4, 16, QChar( '0' ) ).toUpper();
        auto const targetMachineArchitectureName =
            QString::fromStdString( PE::getMachineArchitectureName( ntFileHeader.targetMachineArchitecture ) );
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
}