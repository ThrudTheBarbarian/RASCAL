#include <QPainter>

#include "constants.h"
#include "limiters.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Limiters::Limiters(QWidget *parent) : QWidget(parent)
	{

	}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Limiters::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);

	QPainter painter(this);

	int width = size().width();
	int height = size().height();

	painter.fillRect(0, 0, width, height, QColor(110,220,220));
	painter.drawText(10, 10, "Limiters");
	}
