#include <QPainter>

#include "constants.h"
#include "sky.h"


/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Sky::Sky(QWidget *parent) : QWidget(parent)
	{

	}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Sky::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);

	QPainter painter(this);

	int width = size().width();
	int height = size().height();

	painter.fillRect(0, 0, width, height, QColor(220,110,220));
	painter.drawText(10, 10, "Sky");
	}
