
#include "EXEViewer.h"

EXEViewer::EXEViewer( QWidget* parentWidget )
: QTabWidget( parentWidget )
{
    addTab( new QWidget, "Raw Data" );
}