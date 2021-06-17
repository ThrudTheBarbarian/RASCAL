#ifndef WATERFALL_H
#define WATERFALL_H

#include <QWidget>

#include "properties.h"

class Waterfall : public QWidget
	{
	Q_OBJECT
	/**************************************************************************\
	|* Typedefs and enums
	\**************************************************************************/
	typedef QList<QColor>	ColourList;

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GETSET(ColourList, gradient, Gradient);
	GET(double, maxVal);
	GET(double, minVal);
	GET(bool, haveData);

	private:
		/**********************************************************************\
		|* Return the Colour to use, interpolated between those in _gradient
		\**********************************************************************/
		QColor _getGradientColour(double at);


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
