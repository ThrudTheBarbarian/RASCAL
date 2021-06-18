#ifndef WATERFALL_H
#define WATERFALL_H

#include <QWidget>

#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QImage)


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
	GETSET(ColourList, gradient, Gradient);		// Colour map to use
	GETSET(bool, redrawImage, RedrawImage);		// Need to redraw  backing img?
	GETSET(int, sampleSecs, SampleSecs);		// Seconds between samples
	GETSET(int, updateSecs, UpdateSecs);		// Seconds between updates
	GET(double, updateMax);						// Max value in update data
	GET(double, updateMin);						// Min value in update data
	GET(bool, haveData);						// Can draw something
	GET(int, binLo);							// Smallest bin number to show
	GET(int, binHi);							// Largest bin number to show
	GET(int, binMax);							// Largest bin available

	private:
		/**********************************************************************\
		|* Return the Colour to use, interpolate those in _gradient
		\**********************************************************************/
		QColor _getGradientColour(float at);

		/**********************************************************************\
		|* Update the image from the backing data
		\**********************************************************************/
		void _updateImage(void);

		/**********************************************************************\
		|* Variables
		\**********************************************************************/
		QImage *_img;							// The backing image
		QVector<int64_t> _updates;				// The backing 'update' data
		QVector<int64_t> _samples;				// The backing 'sample' data

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
