#ifndef WATERFALL_H
#define WATERFALL_H

#include <QWidget>

class Waterfall : public QWidget
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Waterfall(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;

	signals:

	};

#endif // WATERFALL_H
