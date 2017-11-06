#ifndef QT_DICOM_VIEWER_H
#define QT_DICOM_VIEWER_H

#include <QMainWindow>
#include <QFileDialog>
#include <qfuture.h>
#include <qfuturewatcher.h>
#include <QtConcurrent/qtconcurrentrun.h>

#include <gdcmImageReader.h>
#include <gdcmImage.h>
#include <gdcmGlobal.h>
#include <gdcmDicts.h>
#include <gdcmDict.h>
#include <gdcmAttribute.h>
#include <gdcmStringFilter.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include <dirent.h> // for directory searching
#include <locale> // below three for utf8
#include <codecvt>
#include <string>

// int seg_downward = 1, seg_upward = 2, seg_both = 3;

namespace Ui {
class QtDicomViewer;
}

class QtDicomViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtDicomViewer(QWidget *parent = 0);
    ~QtDicomViewer();

private: 
    Ui::QtDicomViewer *ui;
	// attributes
    int rows; // rows and numbers for dicom files, assume same among series
    int columns;
	int sliceId;
	std::string toothID; // ID for the tooth being segmented
	std::string dicomDir; // directory for dicom files
	std::string dicomdirDir; // directory for DICOMDIR file
	std::vector<cv::Mat> slices; // image slices, 0-255
	std::vector<std::vector<std::vector<cv::Point>>> segResults; // segmentation results of each slice
	cv::Mat currentSlice;
	std::vector<cv::Point> contour; // one contour stored by points
	std::vector<std::vector<cv::Point>> contours;
	double segValueRange[3];

	// methods
	void switchSlice(int id);
	void generateContour();
	cv::Mat getMask();
	std::string formatString(std::string inputString, int index);

private slots:
	void resizeEvent(QResizeEvent* event);
	void on_actionOpenFile_triggered();
	void on_ScrollBarIV1_valueChanged(int position);
	void on_SpinBoxIV1_valueChanged(int position);

	void on_ImageVewer1_clicked(int, int, Qt::MouseButtons);
};

#endif
