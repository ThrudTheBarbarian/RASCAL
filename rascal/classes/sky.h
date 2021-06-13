#ifndef SKY_H
#define SKY_H

#include <QWidget>

class Sky : public QWidget
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Sky(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;

	signals:

	};

#endif // SKY_H
