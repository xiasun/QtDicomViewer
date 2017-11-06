#include "qt_dicom_viewer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtDicomViewer w;
    w.show();

    return a.exec();
}
