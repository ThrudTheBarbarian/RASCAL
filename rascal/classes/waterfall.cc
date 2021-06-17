#include <QPainter>

#include <libra.h>

#include "waterfall.h"

#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

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
Waterfall::Waterfall(QWidget *parent) : QWidget(parent)
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

	QPainter qp(this);
	for (int i=0; i<100; i++)
		{
		QPen pen(_getGradientColour(((float)i)/100.0), 2, Qt::SolidLine);
		qp.setPen(pen);
		qp.drawLine(20, 10+2*i, 250, 10+2*i);
		}
	}


/******************************************************************************\
|* We got an update message
\******************************************************************************/
void Waterfall::updateReceived(int64_t idx)
	{
	DataMgr& dmgr		= DataMgr::instance();
	int num				= dmgr.extent(idx) / sizeof(float);
	float *data			= dmgr.asFloat(idx);

	fprintf(stderr, "%7lld: %d floats (%p)\n", idx, num, data);
	}

/******************************************************************************\
|* Private Method - return a colour along the gradient
\******************************************************************************/
QColor Waterfall::_getGradientColour(double at)
	{
	double stepbase = 1.0/(_gradient.count()-1);
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

	double percent	= (at - stepbase * (interval-1)) / stepbase;
	QColor stt		= _gradient[interval];
	QColor end		= _gradient[interval-1];
	int r			= (int)(percent * stt.red()   + (1-percent) * end.red());
	int g			= (int)(percent * stt.green() + (1-percent) * end.green());
	int b			= (int)(percent * stt.blue()  + (1-percent) * end.blue());

	return QColor::fromRgb(r,g,b);
	}
