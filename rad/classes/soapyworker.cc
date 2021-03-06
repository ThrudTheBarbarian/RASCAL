#include <unistd.h>

#include <SoapySDR/Device.hpp>

#include <libra.h>

#include "soapyio.h"
#include "soapyworker.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG  qDebug(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define WARN qWarning(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR	 qCritical(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
SoapyWorker::SoapyWorker(QObject *parent)
			:QObject(parent)
			,_isActive(false)
	{}

/******************************************************************************\
|* Start sampling from the SOAPY device
\******************************************************************************/
void SoapyWorker::startSampling(void)
	{
	_isActive = true;

	/**************************************************************************\
	|* Configure the stream
	\**************************************************************************/
	SoapySDR::Stream *rx = _sdr->rxStream();
	if (rx == nullptr)
		{
		ERR << "Cannot connect to the SDR. Exiting";
		exit(-1);
		}

	/**************************************************************************\
	|* Get the MTU for the stream, so we know how much we can request
	\**************************************************************************/
	int mtu = _sdr->dev()->getStreamMTU(rx);

	/**************************************************************************\
	|* Obtain a sample buffer from the data manager
	\**************************************************************************/
	int complexFactor	= _sdr->isComplexStream() ? 2 : 1;
	int occupancy		= _sdr->sampleValueBytes() * complexFactor;
	int64_t ping		= DataMgr::instance().blockFor(mtu, occupancy);
	int64_t pong		= DataMgr::instance().blockFor(mtu, occupancy);

	void *pingBuffers[]		= {DataMgr::instance().asUint8(ping)};
	void *pongBuffers[]		= {DataMgr::instance().asUint8(pong)};


	/**************************************************************************\
	|* Enter the loop
	\**************************************************************************/
	bool isPing = true;
	while (_isActive)
		{
		void **buffers = isPing ? pingBuffers : pongBuffers;
		int flags = 0;
		long long time_ns = 0;

		// Read the data
		int samples = _sdr->waitForData(rx, buffers, mtu, flags, time_ns);
		if (samples < 0)
			ERR << "waitForData() returned" << samples;
		else
			emit dataAvailable(isPing ? ping : pong,
							   samples,
							   _sdr->maxValue(),
							   _sdr->sampleValueBytes(),
							   _sdr->isComplexStream());

		// Cycle around with the next buffer
		isPing = !isPing;
		}
	}

/******************************************************************************\
|* Stop sampling from the SOAPY device
\******************************************************************************/
void SoapyWorker::stopSampling(void)
	{

	}
