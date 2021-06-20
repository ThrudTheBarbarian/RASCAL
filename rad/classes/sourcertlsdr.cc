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
	info.max	= 128;
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
		sampleRate = rtlsdr_get_sample_rate(_dev);
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

			double nVal = ((double)nearest);
			double gVal = ((double)gain);

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
|* Set the bandwidth for the tuner
\******************************************************************************/
bool SourceRtlSdr::setBandwidth(int requestedBw)
	{
	int ok = -1;

	if (requestedBw < 0)
		requestedBw = _sampleRate;
	else
		requestedBw *= 1000;
	if (requestedBw > _sampleRate)
		requestedBw = _sampleRate;


	QList<QString> bandwidths = listBandwidths();
	int count = bandwidths.size();
	if (count <= 0)
		{
		ERR << "Cannot get list of bandwidths to use";
		return false;
		}

	int bw[count];
	memset(bw, 0, sizeof(int) * count);

	int idx = 0;
	for (QString &rep : bandwidths)
		bw[idx++] = rep.toInt() * 1000;

	int err1, err2;
	int nearest = bw[0];

	for (int i=0; i<count; i++)
		{
		err1 = abs(requestedBw - nearest);
		err2 = abs(requestedBw - bw[i]);
		if (err2 < err1)
			nearest = bw[i];
		}

	double nVal = ((double)nearest);
	double bVal = ((double)requestedBw);

	if (nearest != requestedBw)
		LOG << name()
			<< "setting bandwidth to nearest value ["
			<< (int)nVal << "] to " << (int)bVal;

	ok = rtlsdr_set_tuner_bandwidth(_dev, nearest);
	if (ok < 0)
		ERR << "Failed to set gain to " << nVal;
	else
		LOG << name() << "bandwidth now set to " << (int)nVal;
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
|* Stop sampling : FIXME - this probably doesn't actually work...
\******************************************************************************/
void SourceRtlSdr::stopSampling(void)
	{
	_isActive = false;
	rtlsdr_cancel_async(_dev);
	}


/******************************************************************************\
|* Get a list of antennas - there's only one on an RTL-SDR
\******************************************************************************/
QList<QString> SourceRtlSdr::listAntennas(void)
	{
	QList<QString> list;
	list.append("RX");
	return list;
	}

/******************************************************************************\
|* Get the number of channels in each direction - only 1 RX on RTL-SDR
\******************************************************************************/
SourceBase::ChannelInfo SourceRtlSdr::numberOfChannels(void)
	{
	SourceBase::ChannelInfo info;
	info.rx = 1;
	info.tx = 0;
	return info;
	}

/******************************************************************************\
|* Get a list of available bandwidth settings, in kHz
\******************************************************************************/
QList<QString> SourceRtlSdr::listBandwidths(void)
	{
	QList<QString> list;
	list.append("290");
	list.append("375");
	list.append("420");
	list.append("470");
	list.append("600");
	list.append("860");
	list.append("950");
	list.append("1100");
	list.append("1300");
	list.append("1500");
	list.append("1600");
	list.append("1750");
	list.append("1950");
	list.append("2048");
	list.append("2560");
	return list;
	}

/******************************************************************************\
|* Get a list of frequency ranges, in MHz
\******************************************************************************/
QList<SourceBase::Range> SourceRtlSdr::listFrequencyRanges(void)
	{
	QList<SourceBase::Range> list;
	list.append({.from=24, .to=1799});
	return list;
	}

/******************************************************************************\
|* Get a list of sample rates, in MHz
\******************************************************************************/
QList<SourceBase::Range> SourceRtlSdr::listSampleRateRanges(void)
	{
	QList<SourceBase::Range> list;
	list.append({.from=0, .to=8});
	return list;
	}


/******************************************************************************\
|* Get a list of available gains, in dB
\******************************************************************************/
QList<double> SourceRtlSdr::listGains(void)
	{
	QList<double> list;

	int count = rtlsdr_get_tuner_gains(_dev, NULL);
	if (count <= 0)
		ERR << "Cannot get list of gains from tuner";
	else
		{
		int *gains = new int[count];
		count = rtlsdr_get_tuner_gains(_dev, gains);

		for (int i=0; i<count; i++)
			{
			double gain = ((double)(gains[i]))/10.0;
			list.append(gain);
			}
		}

	return list;
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
