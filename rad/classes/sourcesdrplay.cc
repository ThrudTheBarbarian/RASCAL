
#include <QCoreApplication>

#include "constants.h"
#include "datamgr.h"
#include "sourcemgr.h"
#include "sourcesdrplay.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG  qDebug(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define WARN qWarning(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR	 qCritical(log_src) << QTime::currentTime().toString("hh:mm:ss.zzz")


/******************************************************************************\
|* Tuner options
\******************************************************************************/
#define MAX_TUNERS 4
static const char * _tuners[MAX_TUNERS] =
	{
	"TunerA",
	"TunerB",
	"Master/Slave",
	"Slave"
	};

#define TUNER_A_IDX			(0)
#define TUNER_B_IDX			(1)
#define MASTER_SLAVE		(2)
#define SLAVE				(3)

/******************************************************************************\
|* Bandwidth options
\******************************************************************************/
#define MAX_BANDWIDTHS		(8)
static int _bandwidths[MAX_BANDWIDTHS] =
	{
	sdrplay_api_BW_0_200,
	sdrplay_api_BW_0_300,
	sdrplay_api_BW_0_600,
	sdrplay_api_BW_1_536,
	sdrplay_api_BW_5_000,
	sdrplay_api_BW_6_000,
	sdrplay_api_BW_7_000,
	sdrplay_api_BW_8_000,
	};

#define DEFAULT_FREQUENCY		(1420406000)

/******************************************************************************\
|* Static declarations
\******************************************************************************/
static SourceSdrPlay * _self;


/******************************************************************************\
|* Callback definition: Stream A has data
\******************************************************************************/
void StreamACallback(short *xi,
					 short *xq,
					 sdrplay_api_StreamCbParamsT *params,
					 unsigned int numSamples,
					 unsigned int reset,
					 void *cbContext)
	{
	Q_UNUSED(cbContext);
	if (_self)
		_self->streamA(xi, xq, params, numSamples, reset);
	else
		ERR << "Got Stream A callback with no handler!";
	}

/******************************************************************************\
|* Callback definition: Stream B has data
\******************************************************************************/
void StreamBCallback(short *xi,
					 short *xq,
					 sdrplay_api_StreamCbParamsT *params,
					 unsigned int numSamples,
					 unsigned int reset,
					 void *cbContext)
	{
	Q_UNUSED(cbContext);
	if (_self)
		_self->streamB(xi, xq, params, numSamples, reset);
	else
		ERR << "Got Stream B callback with no handler!";
	}

/******************************************************************************\
|* Callback definition: We have an event to handle
\******************************************************************************/
void EventCallback(sdrplay_api_EventT eventId,
				   sdrplay_api_TunerSelectT tuner,
				   sdrplay_api_EventParamsT *params,
				   void *cbContext)
	{
	Q_UNUSED(cbContext);
	if (_self)
		_self->eventCb(eventId, tuner, params);
	else
		ERR << "Got Stream B callback with no handler!";
	}

/******************************************************************************\
|* Constructor
\******************************************************************************/
SourceSdrPlay::SourceSdrPlay(QObject *parent)
			  :SourceBase(parent)
			  ,_isActive(false)
			  ,_dev(nullptr)
			  ,_params(nullptr)
			  ,_bufId(-1)
			  ,_antenna(_tuners[0])
			  ,_sampleRate(0)
			  ,_frequency(DEFAULT_FREQUENCY)
			  ,_bandwidth(_bandwidths[4])
			  ,_gain(40)
	{
	_self = this;
	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
SourceSdrPlay::~SourceSdrPlay(void)
	{
	}

/******************************************************************************\
|* Return the information on how this source reports data
\******************************************************************************/
SourceBase::StreamInfo SourceSdrPlay::streamInfo(void)
	{
	StreamInfo info;
	info.format = STREAM_S16C;
	info.max	= 16384;
	info.name	= "sdrplay";
	info.mode	= "";
	return info;
	}

/******************************************************************************\
|* Attempt to open the API - we ignore the device id for now
\******************************************************************************/
bool SourceSdrPlay::open(int deviceId)
	{
	bool ok = false;

	/**************************************************************************\
	|* Check that we can open the API
	\**************************************************************************/
	sdrplay_api_ErrT err = sdrplay_api_Open();
	if (err != sdrplay_api_Success)
		{
		ERR << "Failed to open SDRPlay API";
		return false;
		}

	/**************************************************************************\
	|* Enable debugging mode
	\**************************************************************************/
	err = sdrplay_api_DebugEnable(NULL, sdrplay_api_DbgLvl_Verbose);
	if (err != sdrplay_api_Success)
		 {
		 ERR << "sdrplay_api_DebugEnable failed:"
			 <<  sdrplay_api_GetErrorString(err);
		}

	/**************************************************************************\
	|* Fetch the API version
	\**************************************************************************/
	float version = 0.0f;
	err = sdrplay_api_ApiVersion(&version);
	if (err != sdrplay_api_Success)
		 {
		 ERR << "sdrplay_api_ApiVersion failed:"
			 <<  sdrplay_api_GetErrorString(err);
		}

	/**************************************************************************\
	|* Check we don't have a version mismatch
	\**************************************************************************/
	if (version != SDRPLAY_API_VERSION)
		{
		ERR << "API version don't match [local:" << SDRPLAY_API_VERSION
			<< ", system:" << version;

		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* Lock the API while we figure out what we're connecting to
	\**************************************************************************/
	sdrplay_api_LockDeviceApi();


	/**************************************************************************\
	|* Fetch the list of available devices
	\**************************************************************************/
	unsigned int ndev;
	err = sdrplay_api_GetDevices(_devs, &ndev, MAX_DEV);
	if (err != sdrplay_api_Success)
		{
		ERR << "sdrplay_api_GetDevices failed:"
			<< sdrplay_api_GetErrorString(err);
		sdrplay_api_UnlockDeviceApi();
		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* If we've not asked for a device in range, return false
	\**************************************************************************/
	if ((ndev <= 0) || (deviceId >= (int)ndev))
		{
		ERR << "Selected device out of range (max: " << ndev << ")";
		sdrplay_api_UnlockDeviceApi();
		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* Choose the device, and play nice with the Duo
	\**************************************************************************/
	_dev = &_devs[deviceId];
	if (_dev->hwVer == SDRPLAY_RSPduo_ID)
		{
		/**********************************************************************\
		|* If the master device is available, choose it
		\**********************************************************************/
		if (_dev->tuner & sdrplay_api_RspDuoMode_Master)
			{
			if (_antenna == _tuners[TUNER_A_IDX])
				{
				_dev->tuner = sdrplay_api_Tuner_A;
				_dev->rspDuoMode = sdrplay_api_RspDuoMode_Single_Tuner;
				}
			else if (_antenna == _tuners[TUNER_B_IDX])
				{
				_dev->tuner = sdrplay_api_Tuner_B;
				_dev->rspDuoMode = sdrplay_api_RspDuoMode_Single_Tuner;
				}
			else
				{
				_dev->rspDuoMode = sdrplay_api_RspDuoMode_Master;
				_dev->rspDuoSampleFreq = 6000000.0;
				}
			}
		}

	/**************************************************************************\
	|* Select the device
	\**************************************************************************/
	err = sdrplay_api_SelectDevice(_dev);
	if (err != sdrplay_api_Success)
		{
		ERR << "Cannot select tuner device : "
			<< sdrplay_api_GetErrorString(err);
		sdrplay_api_UnlockDeviceApi();
		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* Unlock the API, now we have selected the device
	\**************************************************************************/
	sdrplay_api_UnlockDeviceApi();

	/**************************************************************************\
	|* Fetch the current device parameters
	\**************************************************************************/
	err = sdrplay_api_GetDeviceParams(_dev->dev, &_params);
	if (err != sdrplay_api_Success)
		{
		ERR << "Cannot fetch device parameters : "
			<< sdrplay_api_GetErrorString(err);
		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* Check we didn't get a null set of parameters
	\**************************************************************************/
	if (_params == nullptr)
		{
		ERR << "Received nil device parameters!";
		sdrplay_api_Close();
		return false;
		}

	/**************************************************************************\
	|* Configure the device parameters. Only need to update non-default settings
	\**************************************************************************/
	if (_params->devParams != nullptr)
		{
		/**********************************************************************\
		|* This will be NULL for slave devices, only the master can change
		|* these parameters.
		\**********************************************************************/
		if (_antenna == _tuners[MASTER_SLAVE])
			{
			// Change from default Fs  to 8MHz
			_params->devParams->fsFreq.fsHz = 8000000.0;
			}
		}

	/**************************************************************************\
	|* Configure the correct RX parameters
	\**************************************************************************/
	_rxParams = (_dev->tuner == sdrplay_api_Tuner_B)
			  ? _params->rxChannelB
			  : _params->rxChannelA;

	if (_rxParams != nullptr)
		{
		_rxParams->tunerParams.rfFreq.rfHz = _frequency;
		_rxParams->tunerParams.bwType = (sdrplay_api_Bw_MHzT)_bandwidth;
		if (_antenna != _tuners[MASTER_SLAVE])
			_rxParams->tunerParams.ifType = sdrplay_api_IF_Zero;

		_rxParams->tunerParams.gain.gRdB = _gain;
		_rxParams->tunerParams.gain.LNAstate = 5;

		/**********************************************************************\
		|* Disable AGC.
		\**********************************************************************/
		_rxParams->ctrlParams.agc.enable = sdrplay_api_AGC_DISABLE;
		}

	/**************************************************************************\
	|* Configure the callback functions
	\**************************************************************************/
	_cbfns.StreamACbFn = StreamACallback;
	_cbfns.StreamBCbFn = StreamBCallback;
	_cbfns.EventCbFn = EventCallback;
	return ok;
	}


/******************************************************************************\
|* Handle data appearing on stream A
\******************************************************************************/
void SourceSdrPlay::streamA(short *xi,
							short *xq,
							sdrplay_api_StreamCbParamsT *params,
							unsigned int numSamples,
							unsigned int reset)
	{
	Q_UNUSED(xi);
	Q_UNUSED(xq);
	Q_UNUSED(params);
	Q_UNUSED(numSamples);
	Q_UNUSED(reset);
	}

/******************************************************************************\
|* Handle data appearing on stream B
\******************************************************************************/
void SourceSdrPlay::streamB(short *xi,
							short *xq,
							sdrplay_api_StreamCbParamsT *params,
							unsigned int numSamples,
							unsigned int reset)
	{
	Q_UNUSED(xi);
	Q_UNUSED(xq);
	Q_UNUSED(params);
	Q_UNUSED(numSamples);
	Q_UNUSED(reset);
	}

/******************************************************************************\
|* Handle events
\******************************************************************************/
void SourceSdrPlay::eventCb(sdrplay_api_EventT eventId,
							sdrplay_api_TunerSelectT tuner,
							sdrplay_api_EventParamsT *params)
	{
	Q_UNUSED(eventId);
	Q_UNUSED(tuner);
	Q_UNUSED(params);
	}
