#include <QPainter>

#include <libra.h>

#include "mainwindow.h"
#include "waterfall.h"

#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

#define MAX_SAMPLES			(2048)
#define SEPARATOR			(3)
#define MIN_SAMPLES			(32)

static unsigned char __map [][3] =
	{	/* These values were originally calculated for a dynamic range of 180dB. */
		{	255,	255,	255	},
		{	240,	254,	216	},
		{	242,	251,	185	},
		{	253,	245,	143	},
		{	253,	200,	102	},
		{	252,	144,	66	},
		{	252,	75,		32	},
		{	237,	28,		41	},
		{	214,	3,		64	},
		{	183,	3,		101	},
		{	157,	3,		122	},
		{	122,	3,		126	},
		{	80,		2,		110	},
		{	45,		2,		89	},
		{	19,		2,		70	},
		{	1,		3,		53	},
		{	1,		3,		37	},
		{	1,		2,		19	},
		{	0,		0,		0	},
	};

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Waterfall::Waterfall(QWidget *parent)
		  :QWidget(parent)
		  ,_redrawImage(true)
		  ,_sampleSecs(300)
		  ,_updateSecs(5)
		  ,_updateMax(-MAXFLOAT)
		  ,_updateMin(MAXFLOAT)
		  ,_sampleMax(-MAXFLOAT)
		  ,_sampleMin(MAXFLOAT)
		  ,_haveData(false)
		  ,_binLo(-1)
		  ,_binHi(-1)
		  ,_img(nullptr)
	{
	/**************************************************************************\
	|* Create the colour gradient
	\**************************************************************************/
	for (int i=18; i>=0; i--)
		{
		int r = __map[i][0];
		int g =	__map[i][1];
		int b = __map[i][2];

		_gradient.append(QColor::fromRgb(r,g,b));;
		}
	}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Waterfall::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);
	if (_img == nullptr)
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

		return;
		}

	/**************************************************************************\
	|* Create the rectangle to draw the graph within
	\**************************************************************************/
	QRect R(MainWindow::DRAW_L,
			MainWindow::DRAW_T,
			size().width()  - MainWindow::DRAW_L - MainWindow::DRAW_R,
			size().height() - MainWindow::DRAW_T - MainWindow::DRAW_B);

	QPainter qp(this);
	qp.drawImage(R, *_img);
	}


/******************************************************************************\
|* We got an update message
\******************************************************************************/
void Waterfall::updateReceived(int64_t idx)
	{
	DataMgr& dmgr		= DataMgr::instance();
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
	int center = num/2;
	for (int i=_binLo; i<_binHi; i++)
		{
		// Ignore the DC peak
		if ((i >= center-1) && (i <= center+1))
			continue;

		_updateMax = (_updateMax > data[i]) ? _updateMax : data[i];
		_updateMin = (_updateMin < data[i]) ? _updateMin : data[i];
		}

	/**************************************************************************\
	|* Update the backing data
	\**************************************************************************/
	dmgr.retain(idx);
	_updates.insert(0, idx);
	int limit = _updatesHeight();
	while (_updates.size() > limit)
		{
		int64_t last = _updates.last();
		dmgr.release(last);
		_updates.removeLast();
		}

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
void Waterfall::sampleReceived(int64_t idx)
	{
	DataMgr& dmgr		= DataMgr::instance();
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
	int center = num/2;
	for (int i=_binLo; i<_binHi; i++)
		{
		// Ignore the DC peak
		if ((i >= center-1) && (i <= center+1))
			continue;

		_sampleMax = (_sampleMax > data[i]) ? _sampleMax : data[i];
		_sampleMin = (_sampleMin < data[i]) ? _sampleMin : data[i];
		}

	/**************************************************************************\
	|* Update the backing data
	\**************************************************************************/
	dmgr.retain(idx);
	_samples.insert(0, idx);
	while (_updates.size() > MAX_SAMPLES)
		{
		int64_t last = _samples.last();
		dmgr.release(last);
		_samples.removeLast();
		}

	/**************************************************************************\
	|* Add a black line into the updates count, so we can tell where one sample
	|* starts and another ends
	\**************************************************************************/
	int64_t bufId	= dmgr.blockFor(num, sizeof(float));
	float *dummy	= dmgr.asFloat(bufId);
	memset(dummy, 0, num*sizeof(float));
	_updates.insert(0, bufId);

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
|* Private Method - return a colour along the gradient
\******************************************************************************/
QColor Waterfall::_getGradientColour(float at)
	{
	float stepbase = 1.0/(_gradient.count()-1);
	int interval	= _gradient.count()-1;

	/**************************************************************************\
	|* Determine which two colours we're between
	\**************************************************************************/
	for (int i=1; i<_gradient.count();i++)
		if (at <= i*stepbase )
			{
			interval=i;
			break;
			}

	float percent	= (at - stepbase * (interval-1)) / stepbase;
	QColor stt		= _gradient[interval];
	QColor end		= _gradient[interval-1];
	int r			= (int)(percent * stt.red()   + (1-percent) * end.red());
	int g			= (int)(percent * stt.green() + (1-percent) * end.green());
	int b			= (int)(percent * stt.blue()  + (1-percent) * end.blue());

	// We can ask for out-of-range values due to scaling, so constrain to the
	// correct 0..255 range
	r				= (r < 0) ? 0 : (r > 255) ? 255 : r;
	g				= (g < 0) ? 0 : (g > 255) ? 255 : g;
	b				= (b < 0) ? 0 : (b > 255) ? 255 : b;

	return QColor::fromRgb(r,g,b);
	}

/******************************************************************************\
|* Private Method - Update the backing image
\******************************************************************************/
void Waterfall::_updateImage(void)
	{
	DataMgr& dmgr		= DataMgr::instance();
	int updates			= _updatesHeight();

	/**************************************************************************\
	|* Check to see if we have an image, if not, create it
	\**************************************************************************/
	if (_img == nullptr)
		{
		int height	= size().height();
		if (height  < updates + SEPARATOR + MIN_SAMPLES)
			height  = updates + SEPARATOR + MIN_SAMPLES;

		_img = new QImage(_binMax, height, QImage::Format_ARGB32);
		}

	/**************************************************************************\
	|* Draw the background
	\**************************************************************************/
	QPainter painter(_img);
	QRect bounds = _img->rect();
	painter.fillRect(bounds, QColor::fromRgb(200,200,200));

	/**************************************************************************\
	|* Draw the updates
	\**************************************************************************/
	for (int i=0; i<_updates.size(); i++)
		{
		int idx		 = _updates.at(i);
		float * data = dmgr.asFloat(idx);
		if (data != nullptr)
			{
			float scale = 1.0f / (_updateMax - _updateMin);
			for (int j=_binLo; j<_binHi; j++)
				{
				float value = (data[j] - _updateMin) * scale;
				QColor rgb = _getGradientColour(value);
				painter.setPen(rgb);
				painter.drawPoint(j,i);
				}
			}
		}

	/**************************************************************************\
	|* Draw the white line to separate updates from samples
	\**************************************************************************/
	QPen pen(QColor::fromRgb(255,255,255));
	pen.setWidth(SEPARATOR);
	painter.setPen(pen);
	painter.drawLine(0, updates, _binMax, updates);

	/**************************************************************************\
	|* Draw the samples
	\**************************************************************************/
	int max = _img->rect().height() - updates - SEPARATOR;
	max		= (max > MAX_SAMPLES) ? MAX_SAMPLES			: max;
	max		= (max > _samples.size()) ? _samples.size() : max;

	for (int i=0; i<max; i++)
		{
		int y		 = i + updates + SEPARATOR;
		int idx		 = _samples.at(i);
		float * data = dmgr.asFloat(idx);
		if (data != nullptr)
			{
			float scale = 1.0f / (_sampleMax - _sampleMin);
			for (int j=_binLo; j<_binHi; j++)
				{
				float value = (data[j] - _sampleMin) * scale;
				QColor rgb = _getGradientColour(value);
				painter.setPen(rgb);
				painter.drawPoint(j,y);
				}
			}
		}
	}

/******************************************************************************\
|* Private Method - Return the height of the fast-flowing waterfall part
\******************************************************************************/
int Waterfall::_updatesHeight(void)
	{
	return 1 + _sampleSecs / _updateSecs;
	}
