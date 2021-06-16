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


	public slots:
		/**********************************************************************\
		|* Receive data ready to show on-screen
		\**********************************************************************/
		void updateReceived(int64_t bufferId);

	};

#endif // WATERFALL_H
