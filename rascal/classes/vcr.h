#ifndef VCR_H
#define VCR_H

#include <QWidget>

class Vcr : public QWidget
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Vcr(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;

	signals:

	};

#endif // VCR_H
