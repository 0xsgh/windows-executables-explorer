
#include <QApplication>
#include <QMainWindow>

int
main( int argCount, char** args )
{
    auto app = QApplication( argCount, args );

    auto programWindow = QMainWindow();
    programWindow.show();

    return app.exec();
}
