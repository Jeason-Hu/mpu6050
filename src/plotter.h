#ifndef PLOTTER_H
#define PLOTTER_H

#include <QMap>
#include <QPixmap>
#include <QQueue>
#include <QWidget>

class QToolButton;
class PlotSettings;

class Plotter:public QWidget
{
	Q_OBJECT

public:
	Plotter(QWidget *parent =0);

	void setPlotSettings(const PlotSettings &settings);
	void clearCurve(int id);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:
	void setCurveData(int id, QMap<int, QQueue<QPointF>> * curMap);
	void zoomIn();
	void zoomOut();

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);

private:
	void updateRubberBandRegion();
	void refreshPixmap();
	void drawGrid(QPainter *painter);
	void drawCurve(QPainter *painter);

	enum { Margin = 50};

	QToolButton *zoomInButton;
	QToolButton *zoomOutButton;
	QMap<int, QQueue<QPointF>> curveMap;
	QVector<PlotSettings> zoomStack;
	int curZoom;
	int curId;
	bool rubberBandIsShown;
	QRect rubberBandRect;
	QPixmap pixmap;
};

class PlotSettings
{
public:
	PlotSettings();

	void scroll(int dx, int dy);
	void adjust();
	double spanX() const { return maxX - minX;}
	double spanY() const { return maxY - minY;}

	double minX;
	double maxX;
	int numXTicks;
	double minY;
	double maxY;
	int numYTicks;

private:
	static void adjustAxis(double &min, double &max, int &numTicks);
};

#endif