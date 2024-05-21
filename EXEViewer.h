
#ifndef EXEVIEWER_H
#define EXEVIEWER_H

#include <QTabWidget>

class EXEViewer : public QTabWidget
{
    Q_OBJECT

public:
    EXEViewer( QWidget* parentWidget = nullptr );
};

#endif // EXEVIEWER_H