#include <QPainter>

#include "constants.h"
#include "events.h"


/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Events::Events(QWidget *parent) : QWidget(parent)
	{

	}

/******************************************************************************\
|* Paint the display
\******************************************************************************/
void Events::paintEvent(QPaintEvent *e)
	{
	Q_UNUSED(e);

	QPainter painter(this);

	int width = size().width();
	int height = size().height();

	painter.fillRect(0, 0, width, height, QColor(220,220,220));
	painter.drawText(10, 10, "Events");
	}
