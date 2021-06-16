#include <QPainter>

#include <libra.h>

#include "waterfall.h"


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

	}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Waterfall::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);

	QPainter painter(this);

	int width = size().width();
	int height = size().height();

	painter.fillRect(0, 0, width, height, QColor(220,110,110));
	painter.drawText(10, 10, "Waterfall");
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
