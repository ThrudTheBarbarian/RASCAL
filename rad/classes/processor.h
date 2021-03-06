#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <fftw3.h>
#include "properties.h"

#include "sourcebase.h"

QT_FORWARD_DECLARE_CLASS(Config)
QT_FORWARD_DECLARE_CLASS(FFTAggregator)
QT_FORWARD_DECLARE_CLASS(MsgIO)

class Processor : public QObject
	{
	Q_OBJECT

	private:
		/**********************************************************************\
		|* Private variables
		\**********************************************************************/
		Config&			_cfg;			// Configuration
		MsgIO *			_mio;			// Websocket interface
		int				_fftSize;		// Size of the FFT

		int64_t			_work;			// Working buffer
		QQueue<double>	_previous;		// Data left over from last pass

		fftw_plan		_fftPlan;		// Plan for the FFT
		int64_t			_fftIn;			// FFTW buffer used during planning
		int64_t			_fftOut;		// FFTW buffer used during planning
		int64_t			_window;		// Buffer holding the windowing data

		QThread			_bgThread;		// Background aggregation thread
		FFTAggregator *	_aggregator;	// Collect data and send it off

		/**********************************************************************\
		|* Private method: allocate the RAM buffers we need
		\**********************************************************************/
		void _allocate(void);

		/**********************************************************************\
		|* Private method: Populate the windowing data
		\**********************************************************************/
		void _populateWindowData(void);

	public:
		/**********************************************************************\
		|* Constructor
		\**********************************************************************/
		explicit Processor(Config& cfg, QObject *parent = nullptr);
		~Processor(void);

		/**********************************************************************\
		|* Initialise with the data-stream params
		\**********************************************************************/
		void init(MsgIO *mio);

	public slots:
		void dataReceived(int64_t handle,
						  int samples,
						  int max,
						  SourceBase::StreamFormat fmt);

	};

#endif // PROCESSOR_H
