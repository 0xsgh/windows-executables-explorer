
#ifndef OBJVIEWER_H
#define OBJVIEWER_H

#include "PEFiles.h"

#include <QTabWidget>

class OBJViewer : public QTabWidget
{
    Q_OBJECT

public:
    OBJViewer( OBJFile&& loadedOBJFile,
               QWidget* parentWidget = nullptr );

private:
    void
    setUpFileHeadersTab();

    void
    setUpSectionHeadersTab();

private:
    OBJFile    m_loadedOBJFile;
};

#endif // OBJVIEWER_H