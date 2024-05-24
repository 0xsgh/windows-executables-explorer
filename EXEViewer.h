
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

    void
    setUpFileHeadersTab();

    void
    setUpSectionHeadersTab();

private:
    EXEFile    m_loadedEXEFile;
};

#endif // EXEVIEWER_H