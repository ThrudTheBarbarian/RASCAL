#ifndef GRAPH_H
#define GRAPH_H

#include <QMutexLocker>
#include <QWidget>

#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QImage)

class Graph : public QWidget
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GETSET(bool, redrawImage, RedrawImage);		// Need to redraw  backing img?
	GET(int, binLo);							// Smallest bin number to show
	GET(int, binHi);							// Largest bin number to show
	GET(int, binMax);							// Largest bin available
	GET(float, updateMax);						// Max value in update data
	GET(float, updateMin);						// Min value in update data
	GET(float, sampleMax);						// Max value in sample data
	GET(float, sampleMin);						// Min value in sample data
	GETSET(int, sampleSecs, SampleSecs);		// Seconds between samples
	GETSET(int, updateSecs, UpdateSecs);		// Seconds between updates

	private:
		/**********************************************************************\
		|* Update the image from the backing data
		\**********************************************************************/
		void _updateImage(void);

		/**********************************************************************\
		|* Return the number of updates per sample
		\**********************************************************************/
		int _updatesPerSample(void);

		/**********************************************************************\
		|* Variables
		\**********************************************************************/
		QImage *_img;							// The backing image
		QVector<int64_t> _updates;				// The backing 'update' data
		int64_t _sample;						// The current 'sample' data
		QMutex _lock;							// Thread safety

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Graph(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;


	public slots:
		/**********************************************************************\
		|* Receive data ready to show on-screen
		\**********************************************************************/
		void updateReceived(int64_t bufferId);

		/**********************************************************************\
		|* Receive data ready to show on-screen
		\**********************************************************************/
		void sampleReceived(int64_t bufferId);

	};

#endif // GRAPH_H
