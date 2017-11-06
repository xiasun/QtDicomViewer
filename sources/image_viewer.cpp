#include "image_viewer.h"

using namespace std;
using namespace cv;

ImageViewer::ImageViewer(QWidget *parent) :
	QOpenGLWidget(parent)
{
	mBgColor = QColor::fromRgb(150, 150, 150);
	movedFlag = false;
	zoomFactor = 1.0f;
}

void ImageViewer::initializeGL()
{
	makeCurrent();
	initializeOpenGLFunctions();

	float r = ((float)mBgColor.darker().red()) / 255.0f;
	float g = ((float)mBgColor.darker().green()) / 255.0f;
	float b = ((float)mBgColor.darker().blue()) / 255.0f;
	glClearColor(r, g, b, 1.0f);
}

void ImageViewer::resizeGL(int width, int height)
{
	makeCurrent();
	glViewport(0, 0, (GLint)width, (GLint)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, width, -height, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);

	recalculatePosition();

	emit imageSizeChanged(mRenderWidth, mRenderHeight);

	updateScene();
}

void ImageViewer::updateScene()
{
	if (this->isVisible()) update();
}

void ImageViewer::paintGL()
{
	makeCurrent();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderImage();
}

void ImageViewer::renderImage()
{

	drawMutex.lock();
	makeCurrent();

	glClear(GL_COLOR_BUFFER_BIT);

	if (!mRenderQtImg.isNull())
	{
		glLoadIdentity();

		glPushMatrix();
		{
			if (mResizedImg.width() <= 0)
			{
				if (mRenderWidth == mRenderQtImg.width() && mRenderHeight == mRenderQtImg.height())
					mResizedImg = mRenderQtImg;
				else
					mResizedImg = mRenderQtImg.scaled(QSize(mRenderWidth, mRenderHeight),
						Qt::IgnoreAspectRatio,
						Qt::SmoothTransformation);
			}

			// ---> Centering image in draw area

			glViewport(mRenderPosX, mRenderPosY, mRenderWidth, mRenderHeight);
			glRasterPos2i(0, 0); // no negative value allowed

			// zooming
			// mRenderWidth *= 0.1 * zoomFactor; mRenderHeight *= 0.1 * zoomFactor;

			glPixelZoom(zoomFactor, - zoomFactor);
			// glPixelZoom(1, -1);

			glDrawPixels(mResizedImg.width(), mResizedImg.height(), GL_RGBA, GL_UNSIGNED_BYTE, mResizedImg.bits());

		}
		glPopMatrix();

		// end
		glFlush();
	}

	drawMutex.unlock();
}

void ImageViewer::recalculatePosition()
{
	mImgRatio = (float)mOrigImage.cols / (float)mOrigImage.rows;

	mRenderWidth = this->size().width();
	mRenderHeight = floor(mRenderWidth / mImgRatio);

	if (mRenderHeight > this->size().height())
	{
		mRenderHeight = this->size().height();
		mRenderWidth = floor(mRenderHeight * mImgRatio);
	}

	if (!movedFlag) {
		mRenderPosX = floor((this->size().width() - mRenderWidth) / 2);
		mRenderPosY = -floor((this->size().height() - mRenderHeight) / 2);
	}

	mResizedImg = QImage();
}

bool ImageViewer::showImage(const cv::Mat& image)
{
	/*if (!zoomedFlag) {
		zoomFactor = 1.0f;
	}*/

	drawMutex.lock();
	// convert all cv data type into CV_8U

	cv::normalize(image, mOrigImage, 255.0, 0, cv::NORM_MINMAX);
	mOrigImage.convertTo(mOrigImage, CV_8U);

	auto imageSize = image.size();
	imageWidth = imageSize.width;
	imageHeight = imageSize.height;

	if (mOrigImage.channels() == 3)
		cvtColor(mOrigImage, mOrigImage, CV_BGR2RGBA);
	else if (mOrigImage.channels() == 1)
		cvtColor(mOrigImage, mOrigImage, CV_GRAY2RGBA);
	else if (mOrigImage.channels() == 4)
		mOrigImage = mOrigImage;
	else return false;

	mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
		mOrigImage.cols, mOrigImage.rows,
		mOrigImage.step1(), QImage::Format_ARGB32);

	recalculatePosition();

	updateScene();
	drawMutex.unlock();
	return true;
}

void ImageViewer::mousePressEvent(QMouseEvent * event) {
	// middle button for move image
	if (event->button() == Qt::MiddleButton) {
		lastPosX = event->x();
		lastPosY = event->y();
		return;
	}

	scaleX = float(imageWidth) / float(mRenderWidth * zoomFactor);
	scaleY = float(imageHeight) / float(mRenderHeight * zoomFactor);

	int xAtPress = event->x();
	int yAtPress = event->y();

	// convert window coordinates to cv::Mat coordinates
	int xPos = (xAtPress - mRenderPosX) * scaleX;
	int yPos = (yAtPress + mRenderPosY) * scaleY;

	emit clickInfo(xPos, yPos, event->buttons());
}

void ImageViewer::mouseReleaseEvent(QMouseEvent * event) {
	/*if (event->button() == Qt::MiddleButton) {
		drawMutex.lock();

		mRenderPosX += event->x() - lastPosX;
		mRenderPosY -= event->y() - lastPosY;

		updateScene();

		drawMutex.unlock();
		return;
	}*/
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent * event) {
	if (event->buttons() & Qt::LeftButton)
	{
		;
	}
	else if (event->buttons() & Qt::RightButton)
	{
		;
	}
}

void ImageViewer::mouseMoveEvent(QMouseEvent * event) {
	return; // disable this method

	if (event->buttons() != Qt::MiddleButton) { // buttons not button here
		return;
	} 

	drawMutex.lock();

	mRenderPosX += event->x() - lastPosX;
	mRenderPosY -= event->y() - lastPosY;

	lastPosX = event->x();
	lastPosY = event->y();

	updateScene();

	drawMutex.unlock();

	movedFlag = true;
}

void ImageViewer::wheelEvent(QWheelEvent *event) {
	return; // disable this method

	drawMutex.lock();

	int numDegrees = event->delta() / 8; 
	int numSteps = numDegrees / 15;
	if (numSteps > 0) {
		zoomFactor *= numSteps * 0.75f;
	} else {
		zoomFactor /= - numSteps * 0.75f;
	}

	updateScene();

	drawMutex.unlock();
}
