#ifndef EVENTS_H
#define EVENTS_H

#include <QWidget>

class Events : public QWidget
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Events(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;

	signals:

	};

#endif // EVENTS_H
