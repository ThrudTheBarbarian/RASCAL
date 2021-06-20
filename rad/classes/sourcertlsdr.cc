#include <signal.h>

#include <QCoreApplication>

#include "constants.h"
#include "datamgr.h"
#include "sourcemgr.h"
#include "sourcertlsdr.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG  qDebug(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define WARN qWarning(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR	 qCritical(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Function declarations
\******************************************************************************/
static void _sighandler(int signum);
static void rtlsdr_callback(uint8_t *buf, uint32_t len, void *ctx);
static SourceRtlSdr * _self;

/******************************************************************************\
|* Constructor
\******************************************************************************/
SourceRtlSdr::SourceRtlSdr(QObject *parent)
			 :SourceBase(parent)
			 ,_isActive(false)
			 ,_dev(nullptr)
			 ,_sampleRate(0)
	{
	_self = this;
	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
SourceRtlSdr::~SourceRtlSdr(void)
	{
	}

/******************************************************************************\
|* Return the information on how this source reports data
\******************************************************************************/
SourceBase::StreamInfo SourceRtlSdr::streamInfo(void)
	{
	StreamInfo info;
	info.format = STREAM_S8C;
	info.name	= "rtlsdr";
	info.mode	= "";
	return info;
	}

/******************************************************************************\
|* Attempt to open the given device id
\******************************************************************************/
bool SourceRtlSdr::open(int deviceId)
	{
	bool ok = false;
	int count = rtlsdr_get_device_count();

	// Default is to choose the first device
	if (deviceId < 0)
		deviceId = 0;

	if (deviceId < count)
		{
		int status = rtlsdr_open(&_dev, deviceId);
		if (status < 0)
			{
			ERR << "Failed to open device" << name() << ":" << deviceId;
			ok = false;
			}
		else
			ok = true;
		}

	return ok;
	}

/******************************************************************************\
|* Set the sample rate
\******************************************************************************/
bool SourceRtlSdr::setSampleRate(int sampleRate)
	{
	int ok = rtlsdr_set_sample_rate(_dev, sampleRate);
	if (ok < 0)
		ERR << "Failed to set sample rate of  " << sampleRate;
	else
		{
		LOG << name() << "sample rate is now" << sampleRate;
		_sampleRate = sampleRate;
		}
	return (ok >= 0);
	}

/******************************************************************************\
|* Set the frequency
\******************************************************************************/
bool SourceRtlSdr::setFrequency(int frequency)
	{
	int ok = rtlsdr_set_center_freq(_dev, frequency);
	if (ok < 0)
		ERR << "Failed to tune to" << frequency;
	else
		LOG << name() << "now tuned to" << frequency << "Hz";
	return (ok >= 0);
	}

/******************************************************************************\
|* Set the gain to the nearest supported gain
\******************************************************************************/
bool SourceRtlSdr::setGain(double gain)
	{
	int ok = -1;
	if (gain == SourceMgr::AUTOMATIC_GAIN)
		{
		ok = rtlsdr_set_tuner_gain_mode(_dev, 0);
		if (ok < 0)
			ERR << "Failed to set automatic gain";
		else
			LOG << name() << "now set to automatic gain";
		}
	else
		{
		ok = rtlsdr_set_tuner_gain_mode(_dev, 1);
		if (ok < 0)
			ERR << "Failed to set manual gain";
		else
			LOG << name() << "now set to manual gain";

		int count = rtlsdr_get_tuner_gains(_dev, NULL);
		if (count <= 0)
			{
			ERR << "Cannot get list of gains from tuner";
			ok = -1;
			}
		else
			{
			int err1, err2;
			int *gains = new int[count];
			count = rtlsdr_get_tuner_gains(_dev, gains);
			int nearest = gains[0];

			for (int i=0; i<count; i++)
				{
				err1 = abs(gain - nearest);
				err2 = abs(gain - gains[i]);
				if (err2 < err1)
					nearest = gains[i];
				}

			double nVal = ((double)nearest) / 10.0f;
			double gVal = ((double)gain) / 10.0f;

			if (nearest != gain)
				LOG << name()
					<< "setting gain to nearest value ["
					<< nVal << "] to " << gVal;

			ok = rtlsdr_set_tuner_gain(_dev, nearest);
			if (ok < 0)
				ERR << "Failed to set gain to " << nVal;
			else
				LOG << name() << "gain now set to " << nVal;
			}
		}
	return (ok >= 0);
	}

/******************************************************************************\
|* Set the antenna - a NOP on the rtl-sdr
\******************************************************************************/
bool SourceRtlSdr::setAntenna(QString antenna)
	{
	Q_UNUSED(antenna);
	return true;
	}

/******************************************************************************\
|* Start sampling
\******************************************************************************/
void SourceRtlSdr::startSampling(void)
	{
	/**************************************************************************\
	|* Handle any signals
	\**************************************************************************/
	struct sigaction sigact;
	sigact.sa_handler = _sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);

	/**************************************************************************\
	|* Allocate a data buffer
	\**************************************************************************/
	DataMgr &dmgr = DataMgr::instance();
	_bufId = dmgr.blockFor(_sampleRate * 2);

	/**************************************************************************\
	|* Reset the buffers
	\**************************************************************************/
	bool ok = (rtlsdr_reset_buffer(_dev) >= 0);
	if (!ok)
		ERR << "WARNING: Failed to reset buffers";

	/**************************************************************************\
	|* Signal that we're now running
	\**************************************************************************/
	_isActive = ok;

	/**************************************************************************\
	|* Start capturing
	\**************************************************************************/
	if (_isActive)
		{
		int extent = dmgr.extent(_bufId);
		rtlsdr_read_async(_dev, rtlsdr_callback, this, 0, extent);
		}
	}

/******************************************************************************\
|* Stop sampling
\******************************************************************************/
void SourceRtlSdr::stopSampling(void)
	{
	_isActive = false;
	rtlsdr_cancel_async(_dev);
	}

/******************************************************************************\
|* Private method : callback for receiving data
\******************************************************************************/
static void rtlsdr_callback(uint8_t *buf, uint32_t len, void *ctx)
	{
	(void)ctx;
	_self->_dataIncoming(buf, len);
	}

void SourceRtlSdr::_dataIncoming(uint8_t *srcData, uint32_t len)
	{
	DataMgr &dmgr	= DataMgr::instance();
	int64_t bufId	= dmgr.blockFor(len);
	uint8_t *data	= dmgr.asUint8(bufId);

	memcpy(data, srcData, len);
	emit dataAvailable(bufId, len/2, 128, STREAM_S8C);
	}


/******************************************************************************\
|* Private method : signal handling
\******************************************************************************/
static void _sighandler(int signum)
	{
	_self->_handleSignal(signum);
	}

void SourceRtlSdr::_handleSignal(int signum)
	{
	fprintf(stderr, "Signal %d caught, exiting!\n", signum);
	if (_isActive)
		{
		rtlsdr_cancel_async(_dev);
		qApp->quit();
		}
	else
		exit(0);
	}
