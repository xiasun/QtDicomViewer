#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_0>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <mutex>
#include <QMouseEvent> 
#include <QWheelEvent>

class ImageViewer : public QOpenGLWidget, protected QOpenGLFunctions_2_0
{
	Q_OBJECT
public:
	explicit ImageViewer(QWidget *parent = 0);

signals:
	void imageSizeChanged(int outW, int outH); /// Used to resize the image outside the widget
	void clickInfo(int, int, Qt::MouseButtons);

public slots:
	bool showImage(const cv::Mat& image); /// Used to set the image to be viewed, for external call only

protected:
	void initializeGL(); /// OpenGL initialization
	void paintGL(); /// OpenGL Rendering
	void resizeGL(int width, int height);        /// Widget Resize Event
	void mousePressEvent(QMouseEvent * event); // 
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent * event); // double click to finish
	void mouseMoveEvent(QMouseEvent * event);
	void wheelEvent(QWheelEvent *event);

	void updateScene();
	void renderImage();

private:

	QImage mRenderQtImg;           /// Qt image to be rendered
	QImage mResizedImg;
	cv::Mat mOrigImage;             /// original OpenCV image to be shown

	QColor mBgColor;		/// Background color

	float mImgRatio;             /// height/width ratio

	int mRenderWidth;
	int mRenderHeight;
	int mRenderPosX; // position of image relative to render window
	int mRenderPosY;
	int imageWidth;
	int imageHeight;
	float scaleX; // between image matrix size and window size
	float scaleY;

	bool movedFlag;

	float zoomFactor;

	int lastPosX; // for calculate moving
	int lastPosY;

	std::vector<cv::Point> contour; // one contour stored by points
	std::vector<std::vector<cv::Point>> contours;

	void recalculatePosition();

	std::mutex drawMutex;
};

#endif // IMAGEVIEWER_H
