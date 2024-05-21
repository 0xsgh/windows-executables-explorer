
#ifndef EXEVIEWER_H
#define EXEVIEWER_H

#include "EXEFile.h"

#include <QTabWidget>

class EXEViewer : public QTabWidget
{
    Q_OBJECT

public:
    EXEViewer( EXEFile&& loadedEXEFile,
               QWidget* parentWidget = nullptr );

private:
    void
    setUpRawDataTab();

private:
    EXEFile    m_loadedEXEFile;
};

#endif // EXEVIEWER_H