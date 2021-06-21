#include <QPainter>

#include <libra.h>

#include "constants.h"
#include "graph.h"
#include "mainwindow.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Graph::Graph(QWidget *parent)
	  :QWidget(parent)
	  ,_redrawImage(true)
	  ,_binLo(-1)
	  ,_binHi(-1)
	  ,_binMax(-1)
	  ,_updateMax(-MAXFLOAT)
	  ,_updateMin(MAXFLOAT)
	  ,_sampleMax(-MAXFLOAT)
	  ,_sampleMin(MAXFLOAT)
	  ,_sampleSecs(300)
	  ,_updateSecs(5)
	  ,_img(nullptr)
	  ,_sample(-1)
	{}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Graph::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);

	if (_img != nullptr)
		{
		QPainter qp(this);
		qp.drawImage(rect(), *_img);
		}
	else
		{
		QPainter qp(this);
		QFont font("Helvetica [Cronyx]", 36);
		qp.setFont(font);
		qp.setPen(QColor::fromRgb(128,128,128));

		int W = size().width();
		int H = size().height();
		QFontMetrics qfm(font);
		QString msg = "Awaiting data";
		QRect bounds = qfm.boundingRect(msg);

		int x = W/2 - bounds.width()/2;
		int y = H/2 - bounds.height()/2;
		qp.drawText(x, y, msg);
		}
	}


/******************************************************************************\
|* We got an update message
\******************************************************************************/
void Graph::updateReceived(int64_t idx)
	{
	QMutexLocker guard(&_lock);

	DataMgr& dmgr		= DataMgr::instance();
	dmgr.retain(idx);
	int num				= dmgr.extent(idx) / sizeof(float);
	float * data		= dmgr.asFloat(idx);

	/**************************************************************************\
	|* Now that we have an update, if we don't have the limits on the data to
	|* process, set it to the size of the incoming data
	\**************************************************************************/
	if ((_binLo < 0) || (_binHi < 0))
		{
		_binLo	= 1;
		_binHi	= num-1;
		_binMax	= num;
		}

	/**************************************************************************\
	|* Update the min/max range to include this data
	\**************************************************************************/\
	for (int i=_binLo; i<_binHi; i++)
		{
		_updateMax = (_updateMax > data[i]) ? _updateMax : data[i];
		_updateMin = (_updateMin < data[i]) ? _updateMin : data[i];
		}

	/**************************************************************************\
	|* Update the backing data
	\**************************************************************************/
	if (_updates.size() == _updatesPerSample())
		{
		for (int64_t buffer : _updates)
			dmgr.release(buffer);
		_updates.clear();
		}
	_updates.insert(0, idx);

	/**************************************************************************\
	|* Update the backing image
	\**************************************************************************/
	_updateImage();

	/**************************************************************************\
	|* And issue a repaint
	\**************************************************************************/
	repaint();
	}


/******************************************************************************\
|* We got a sample message
\******************************************************************************/
void Graph::sampleReceived(int64_t idx)
	{
	QMutexLocker guard(&_lock);

	DataMgr& dmgr		= DataMgr::instance();
	dmgr.retain(idx);
	int num				= dmgr.extent(idx) / sizeof(float);
	float * data		= dmgr.asFloat(idx);

	/**************************************************************************\
	|* Now that we have a sample, if we don't have the limits on the data to
	|* process, set it to the size of the incoming data
	\**************************************************************************/
	if ((_binLo < 0) || (_binHi < 0))
		{
		_binLo	= 1;
		_binHi	= num-1;
		_binMax	= num;
		}

	/**************************************************************************\
	|* Reduce the sample data down to one entry
	\**************************************************************************/
	int64_t currentBuffer = _updates[0];
	for (int64_t buffer : _updates)
		if (buffer != currentBuffer)
			dmgr.release(buffer);
	_updates.clear();
	_updates.append(currentBuffer);

	/**************************************************************************\
	|* Update the min/max range to include this data
	\**************************************************************************/\
	for (int i=_binLo; i<_binHi; i++)
		{
		_sampleMax = (_sampleMax > data[i]) ? _sampleMax : data[i];
		_sampleMin = (_sampleMin < data[i]) ? _sampleMin : data[i];
		}

	/**************************************************************************\
	|* Update the backing data
	\**************************************************************************/
	if (_sample >= 0)
		dmgr.release(_sample);
	_sample = idx;

	/**************************************************************************\
	|* Update the backing image
	\**************************************************************************/
	_updateImage();

	/**************************************************************************\
	|* And issue a repaint
	\**************************************************************************/
	repaint();
	}

/******************************************************************************\
|* Private Method - Return the number of updates per sample
\******************************************************************************/
int Graph::_updatesPerSample(void)
	{
	return _sampleSecs / _updateSecs;
	}

/******************************************************************************\
|* Private Method - Update the backing image
\******************************************************************************/
void Graph::_updateImage(void)
	{
	DataMgr& dmgr		= DataMgr::instance();

	/**************************************************************************\
	|* Check to see if we have an image, if not, create it
	\**************************************************************************/
	if (_img == nullptr)
		_img = new QImage(size().width(),
						  size().height(),
						  QImage::Format_ARGB32);

	/**************************************************************************\
	|* Draw the background
	\**************************************************************************/
	QPainter painter(_img);
	QRect bounds = _img->rect();
	painter.fillRect(bounds, QColor::fromRgb(255,255,255));

	/**************************************************************************\
	|* Create the rectangle to draw the graph within
	\**************************************************************************/
	QRect R(MainWindow::DRAW_L,
			MainWindow::DRAW_T,
			size().width()  - MainWindow::DRAW_L - MainWindow::DRAW_R,
			size().height() - MainWindow::DRAW_T - MainWindow::DRAW_B);

	/**************************************************************************\
	|* Data scaling
	\**************************************************************************/
	float maxY	= ((_updateMax > _sampleMax) ? _updateMax : _sampleMax);
	float minY	= ((_updateMin < _sampleMin) ? _updateMin : _sampleMin);

	float ys	= R.height() / (maxY - minY);
	float xs	= R.width()  / ((float)(_binHi - _binLo));
	int	  xo	= R.left();
	int   yo	= R.bottom();

	/**************************************************************************\
	|* Pen for the contributory updates
	\**************************************************************************/
	int uNum	= _updates.size();

	QPen pen = (uNum > 0)
			 ? QPen(QColor::fromRgba(qRgba(0,0,0,128 / uNum)))
			 : QPen(qRgba(0,0,0,255));
	painter.setPen(pen);

	/**************************************************************************\
	|* Draw the updates
	\**************************************************************************/
	for (int i=0; i<uNum; i++)
		{
		int idx		 = _updates.at(i);
		int ox, oy;

		float * data = dmgr.asFloat(idx);
		if (data != nullptr)
			{
			for (int j=_binLo; j<=_binHi; j++)
				{
				int y = (int)(yo - (data[j] - minY) * ys);
				int x = (int)(xo + j * xs);
				if (j == _binLo)
					painter.drawPoint(x,y);
				else
					painter.drawLine(ox, oy, x, y);

				ox = x;
				oy = y;
				}
			}
		}

	/**************************************************************************\
	|* Draw the samples
	\**************************************************************************/
	pen = QPen(qRgba(0,0,255,255));
	painter.setPen(pen);

	if (_sample >= 0)
		{
		int ox, oy;
		float *data = dmgr.asFloat(_sample);
		if (data != nullptr)
			{
			for (int j=_binLo; j<=_binHi; j++)
				{
				int y = (int)(yo - (data[j] - minY) * ys);
				int x = (int)(xo + j * xs);
				if (j == _binLo)
					painter.drawPoint(x,y);
				else
					painter.drawLine(ox, oy, x, y);

				ox = x;
				oy = y;
				}
			}
		}

	/**************************************************************************\
	|* Draw the axes
	\**************************************************************************/
//	QPen pen(QColor::fromRgb(255,255,255));
//	pen.setWidth(SEPARATOR);
//	painter.setPen(pen);
//	painter.drawLine(0, updates, _binMax, updates);

	}
