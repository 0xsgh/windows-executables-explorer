
#include "EWEAMainWindow.h"

#include <QApplication>

int
main( int argCount, char** args )
{
    auto app = QApplication( argCount, args );

    auto programWindow = EWEAMainWindow();
    programWindow.show();

    return app.exec();
}
