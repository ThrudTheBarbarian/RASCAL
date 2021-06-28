#ifndef SOURCESDRPLAY_H
#define SOURCESDRPLAY_H

#include <QObject>

#include "properties.h"
#include "sourcebase.h"
#include "sdrplay_api.h"

class SourceSdrPlay : public SourceBase
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Constants, enums and typedefs
		\**********************************************************************/
		static const int MAX_DEV = 6;

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GETSET(bool, isActive, IsActive);				// Are we active

	private:
		/**********************************************************************\
		|* Private instance variables
		\**********************************************************************/
		sdrplay_api_DeviceT				_devs[MAX_DEV];	// Possible devices
		sdrplay_api_DeviceT *			_dev;			// Device structure
		sdrplay_api_DeviceParamsT *		_params;		// Device parameters
		sdrplay_api_RxChannelParamsT *	_rxParams;		// Receive params
		sdrplay_api_CallbackFnsT		_cbfns;			// Callbacks
		int64_t							_bufId;			// buf-Id for IQ data
		QString							_antenna;		// Which antenna to use
		int								_sampleRate;	// Sampling Freq in Hz
		int								_frequency;		// Center Freq in Hz
		int								_bandwidth;		// IF bandwidth
		int								_gain;			// IF gain

	public:
		/**********************************************************************\
		|* Constructor
		\**********************************************************************/
		explicit SourceSdrPlay(QObject *parent = nullptr);
		virtual ~SourceSdrPlay(void);

		/**********************************************************************\
		|* Return the information on how this source reports data
		\**********************************************************************/
		virtual StreamInfo streamInfo(void);

		/**********************************************************************\
		|* Return whether we managed to open a given instance
		\**********************************************************************/
		virtual bool open(int deviceId = 0);


		/**********************************************************************\
		|* Callback methods
		\**********************************************************************/
		void streamA(short *xi,
					 short *xq,
					 sdrplay_api_StreamCbParamsT *params,
					 unsigned int numSamples,
					 unsigned int reset);

		void streamB(short *xi,
					 short *xq,
					 sdrplay_api_StreamCbParamsT *params,
					 unsigned int numSamples,
					 unsigned int reset);

		void eventCb(sdrplay_api_EventT eventId,
					 sdrplay_api_TunerSelectT tuner,
					 sdrplay_api_EventParamsT *params);
	};

#endif // SOURCESDRPLAY_H
