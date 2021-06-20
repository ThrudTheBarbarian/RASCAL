#include <complex>

#include <QThreadPool>

#include <libra.h>

#include "config.h"
#include "fftaggregator.h"
#include "msgio.h"
#include "processor.h"
#include "soapyio.h"
#include "taskfft.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Processor::Processor(Config& cfg, QObject *parent)
		  : QObject(parent)
		  ,_cfg(cfg)
		  ,_fftSize(0)
		  ,_work(-1)
		  ,_fftIn(-1)
		  ,_fftOut(-1)
		  ,_window(-1)
	{}

/******************************************************************************\
|* Destructor
\******************************************************************************/
Processor::~Processor(void)
	{
	DataMgr &dmgr	= DataMgr::instance();
	ERR << "Destroying processor";

	if (_fftIn >= 0)
		dmgr.release(_fftIn);
	if (_fftOut >= 0)
		dmgr.release(_fftOut);
	if (_work >= 0)
		dmgr.release(_work);
	if (_window >= 0)
		dmgr.release(_window);
	}

/******************************************************************************\
|* We got data back
\******************************************************************************/
void Processor::dataReceived(int64_t buffer,
							 int samples,
							 int max,
							 SourceBase::StreamFormat fmt)
	{
	DataMgr &dmgr	= DataMgr::instance();
	double *work	= dmgr.asDouble(_work);
	double scale	= 1.0 / (double)max;

	/**************************************************************************\
	|* A complex stream has 2x the data
	\**************************************************************************/
	int extent		= samples;

	/**************************************************************************\
	|* Convert the incoming buffer to double values
	\**************************************************************************/
	switch (fmt)
		{
		case SourceBase::STREAM_S8C:
			{
			extent *= 2;
			int8_t * src8 = dmgr.asInt8(buffer);
			for (int i=0; i<extent; i++)
				*work++ = (*src8++) * scale;
			break;
			}

		case SourceBase::STREAM_S16C:
			{
			extent *= 2;
			int16_t * src16 = dmgr.asInt16(buffer);
			for (int i=0; i<extent; i++)
				*work++ = (*src16++) * scale;
			break;
			}

		}
	work = dmgr.asDouble(_work);

	/**************************************************************************\
	|* There are three cases:
	|* 1: The number of elems from before + this batch < fftSize
	\**************************************************************************/
	if (_previous.size() + extent < _fftSize*2)
		{
		for (int i=0; i<extent; i++)
			_previous.enqueue(work[i]);
		}
	else
		{
		while (extent > _fftSize*2)
			{
			TaskFFT *task = nullptr;

			/******************************************************************\
			|* 2: There are entries left over from the last run. Add them in
			|*    first, then fill up with the work buffer items
			\******************************************************************/
			if (_previous.size() > 0)
				{
				int workItems = _fftSize * 2 - _previous.size();

				task = new TaskFFT(_previous.data(),
								   _previous.size(),
								   work,
								   workItems);

				extent -= workItems;
				work   += workItems;
				_previous.clear();
				}
			else
				{
				/**************************************************************\
				|* 3: We just have to iterate through the passed-in data
				\**************************************************************/
				task = new TaskFFT(work, _fftSize*2);

				work   += _fftSize*2;
				extent -= _fftSize*2;
				}

			connect(task, &TaskFFT::fftDone,
					_aggregator, &FFTAggregator::fftReady);

			task->setPlan(_fftPlan);
			task->setWindow(_window);
			QThreadPool::globalInstance()->start(task);
			}

		/**********************************************************************\
		|* If any samples are left over, enqueue them for the next pass
		\**********************************************************************/
		if (extent > 0)
			{
			for (int i=0; i<extent; i++)
				_previous.enqueue(*work ++);
			}
		}
	}




/******************************************************************************\
|* Initialise
\******************************************************************************/
void Processor::init(MsgIO *mio)
	{
	_mio		= mio;

	/**************************************************************************\
	|* Use a background thread for data-aggregation
	\**************************************************************************/
	_aggregator = new FFTAggregator();
	_aggregator->moveToThread(&_bgThread);

	/**************************************************************************\
	|* Connect up the aggregator to the MsgIO class
	\**************************************************************************/
	connect(_aggregator, &FFTAggregator::aggregatedDataReady,
			mio, &MsgIO::newData);

	/**************************************************************************\
	|* Start the background thread
	\**************************************************************************/
	_bgThread.start();

	_fftSize	= _cfg.fftSize();
	_allocate();

	/**************************************************************************\
	|* Create the FFT plan. We won't actually use these buffers, but we can
	|* substitute others as long as they are compatible, so allocate these
	|* in exactly the same way as the ones we will use.
	\**************************************************************************/
	DataMgr &dmgr		= DataMgr::instance();
	fftw_complex *in	= dmgr.asFFT(_fftIn);
	fftw_complex *out	= dmgr.asFFT(_fftOut);
	_fftPlan			= fftw_plan_dft_1d(_fftSize,
										   in,
										   out,
										   FFTW_FORWARD,
										   FFTW_PATIENT);
	LOG << "FFT plan created";

	_populateWindowData();
	}

/******************************************************************************\
|* Set up the buffers
\******************************************************************************/
void Processor::_allocate(void)
	{
	DataMgr &dmgr = DataMgr::instance();

	if (_work >= 0)
		dmgr.release(_work);
	_work	= dmgr.blockFor(Config::instance().sampleRate()*2, sizeof(double));

	if (_fftIn >= 0)
		dmgr.release(_fftIn);
	_fftIn	= dmgr.fftBlockFor(_fftSize);

	if (_fftOut >= 0)
		dmgr.release(_fftOut);
	_fftOut	= dmgr.fftBlockFor(_fftSize);

	if (_window >= 0)
		dmgr.release(_window);
	_window	= dmgr.blockFor(_fftSize, sizeof(double));
	}


/******************************************************************************\
|* Set up the buffers
\******************************************************************************/
void Processor::_populateWindowData(void)
	{
	DataMgr &dmgr		= DataMgr::instance();
	double *win			= dmgr.asDouble(_window);

	switch (Config::instance().fftWindowType())
		{
		case Config::W_RECTANGLE:
			for (int i=0; i<_fftSize; i++)
				win[i] = 1.0f;
			break;

		case Config::W_HAMMING:
			for (int i=0; i<_fftSize; i++)
				win[i] = 0.54 - 0.46 * cos (2 * M_PI * i / _fftSize);
			break;

		case Config::W_HANNING:
			for (int i=0; i<_fftSize; i++)
				win[i] = 0.54 - 0.5 * cos (2 * M_PI * i / _fftSize);
			break;

		case Config::W_BLACKMAN:
			for (int i=0; i<_fftSize; i++)
				win[i] = 0.42
							   - 0.5 * cos (2 * M_PI * i / _fftSize)
							   + 0.08 * cos (4 * M_PI * i / _fftSize);
			break;

		case Config::W_WELCH:
			for (int i=0; i<_fftSize; i++)
				{
				double sizep1	= _fftSize + 1;
				double range	= 2 * i - _fftSize;
				double step		= range / sizep1;
				win[i]	=  1 - step * step;
				}
			break;

		case Config::W_PARZEN:
			for (int i=0; i<_fftSize; i++)
				{
				double sizep1	= _fftSize + 1;
				double range	= 2 * i - _fftSize;

				win[i] = 1 - fabs (range / sizep1);
				}
			break;
		}
	}
