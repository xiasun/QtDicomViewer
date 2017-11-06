#include "qt_dicom_viewer.h"
#include "ui_QtDicomViewer.h"

using namespace cv;
using namespace std;
using namespace gdcm;

QtDicomViewer::QtDicomViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QtDicomViewer)
{
    ui->setupUi(this);
    rows = 0;
    columns = 0;
	connect(ui->ImageViewer1, SIGNAL(clickInfo(int, int, Qt::MouseButtons)), this, SLOT(on_ImageVewer1_clicked(int, int, Qt::MouseButtons)));
}

QtDicomViewer::~QtDicomViewer()
{
    delete ui;
}

void QtDicomViewer::resizeEvent(QResizeEvent* event)
{
	int margin = 10, 
		fixedWidth = ui->centralWidget->width() - 75, 
		fixedHeight = ui->centralWidget->height() - 198,
		startPosX = 20, 
		startPosY = 40,
		barHeight = 20,
		spinBoxWidth = 51;

	int IV2Height = (fixedHeight - margin * 2 - barHeight) * 0.5, IV2Width = (fixedHeight - margin * 2 - barHeight) * 0.5;
	int IV1Height = fixedHeight, IV1Width = fixedWidth - IV2Width;

	ui->ImageViewer1->setGeometry(startPosX, startPosY, IV1Width, IV1Height);
	ui->ImageViewer2->setGeometry(startPosX + IV1Width + margin, startPosY, IV2Height, IV2Width);
	ui->ImageViewer3->setGeometry(startPosX + IV1Width + margin, startPosY + IV2Width + margin * 4, IV2Height, IV2Width);

	ui->ScrollBarIV1->setGeometry(startPosX, startPosY + IV1Height + margin, IV1Width - spinBoxWidth - margin, barHeight);
	ui->SpinBoxIV1->setGeometry(startPosX + IV1Width - spinBoxWidth, startPosY + IV1Height + margin, spinBoxWidth, barHeight);
	ui->ScrollBarIV2->setGeometry(startPosX + IV1Width + margin, startPosY + IV2Width + margin, IV2Width, barHeight);
	ui->ScrollBarIV3->setGeometry(startPosX + IV1Width + margin, startPosY + IV2Width * 2 + margin  * 5, IV2Width, barHeight);
	ui->textBrowserLog->setGeometry(startPosX, startPosY + IV1Height + barHeight + margin * 2, fixedWidth + margin, 111);
}

void QtDicomViewer::switchSlice(int id) {
	contour.clear();
	sliceId = id;

	ui->ScrollBarIV1->setSliderPosition(sliceId);
	ui->SpinBoxIV1->setValue(sliceId);

	Mat drawing = slices[sliceId].clone();
	for (int i = 0; i < segResults[sliceId].size(); i++) {
		drawContours(drawing, segResults[sliceId], i, Scalar(0, 0, 255), 1);
	}
	ui->ImageViewer1->showImage(drawing);
}

void QtDicomViewer::generateContour() {
	Mat shownSlice = slices[sliceId].clone();

	// draw the current selecting contour
	for (int i= 0; i < contour.size(); i++) { 
		circle(shownSlice, contour[i], 1, Scalar(0, 255, 0), 2);
		if (i == 0) continue;
		line(shownSlice, contour[i - 1], contour[i], Scalar(0, 0, 255), 1);
	}

	// draw previous selected contours
	for (int i = 0; i < contours.size(); i++) { 
		for (int j = 0; j < contours[i].size(); j++) {
			circle(shownSlice, contours[i][j], 1, Scalar(0, 255, 0), 2);
			if (j == 0) continue;
			line(shownSlice, contours[i][j - 1], contours[i][j], Scalar(0, 0, 255), 1);
		}
	}
	ui->ImageViewer1->showImage(shownSlice);
}

Mat QtDicomViewer::getMask() {
	Mat mask(slices[sliceId].size(), CV_8UC1);
	mask = 2;

	for (int i = 0; i < contours.size(); i++) {
		drawContours(mask, contours, i, Scalar(-2), CV_FILLED);
	}
	contours.clear();

	return mask;
}

string QtDicomViewer::formatString(string inputString, int index) {
	stringstream ss;
	ss << setw(3) << setfill('0') << index;
	string outputString = ss.str();

	outputString = inputString + outputString + ".bmp";
	return outputString;
}

void QtDicomViewer::on_actionOpenFile_triggered() {
	// open folder select in dialog
    dicomDir = QFileDialog::getExistingDirectory(this, tr("Select directory of DCM files."),
        "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toUtf8().constData();

	// read all .dcm files with order and construct a Mat vector accordingly
	// based on dirent.h
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dicomDir.c_str())) != NULL) {
		// get all file selected directory
		while ((ent = readdir(dir)) != NULL) {
			string imageFileName = ent->d_name;
			// filter for file extension with .dcm
			if (imageFileName.substr(imageFileName.find_last_of(".") + 1) == "dcm") {
				ImageReader dcmReader;
				dcmReader.SetFileName((dicomDir + "///////" + imageFileName).c_str());
				dcmReader.Read();

                if (!(rows && columns)) {
                    const DataSet &dataset = dcmReader.GetFile().GetDataSet();
                    Attribute<0x0028, 0x0010> attributeRows;
                    Attribute<0x0028, 0x0011> attributeColumns;
                    attributeRows.Set(dataset);
                    attributeColumns.Set(dataset);
                    rows = attributeRows.GetValue();
                    columns = attributeRows.GetValue();
                }

				Image &dcmImage = dcmReader.GetImage(); ;
                int lenRaw = dcmImage.GetBufferLength(); // use bitwise or to combine two char into one
				char *rawImage = new char[lenRaw];
				dcmImage.GetBuffer(rawImage);
                Mat imageC1(rows, columns, CV_32FC1);
				for (int i = 0; i < lenRaw; i += 2) {
					short int pixelValue = ((unsigned char)rawImage[i + 1] << 8) | (unsigned char)rawImage[i]; // unsigned char not char
					imageC1.at<float>(i / 2) = float(pixelValue);
				}
				Mat imageC3;
				normalize(imageC1, imageC1, 255.0, 0, cv::NORM_MINMAX); // normalize before convert channel type
				cvtColor(imageC1, imageC3, CV_GRAY2RGB); 
				slices.push_back(imageC3);
			}
		}
	}

	segResults.resize(slices.size());

	sliceId = (slices.size() / 2) - 1;
	currentSlice = slices[sliceId];
	ui->ImageViewer1->showImage(currentSlice);
	ui->ScrollBarIV1->setMinimum(0); ui->ScrollBarIV1->setMaximum(slices.size() - 1);
	ui->SpinBoxIV1->setMinimum(0); ui->SpinBoxIV1->setMaximum(slices.size() - 1);
	ui->ScrollBarIV1->setSliderPosition(sliceId);
	ui->SpinBoxIV1->setValue(sliceId);
}

void QtDicomViewer::on_ScrollBarIV1_valueChanged(int position) {
	switchSlice(position);
}

void QtDicomViewer::on_SpinBoxIV1_valueChanged(int position) {
	switchSlice(position);
}

void QtDicomViewer::on_ImageVewer1_clicked(int xPos, int yPos, Qt::MouseButtons mouse) {
	if (mouse & Qt::LeftButton) // left click to add point to current contour
	{
		Point position(xPos, yPos);
		contour.push_back(position);
		generateContour();
	}
	else if (mouse & Qt::RightButton) // right click to finish current contour
	{
		contour.push_back(contour[0]); // close the lines
		generateContour();
		contours.push_back(contour);
		contour.clear();
	}
}
