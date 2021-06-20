#ifndef SOURCERTLSDR_H
#define SOURCERTLSDR_H

#include <QObject>

#include "properties.h"
#include "sourcebase.h"
#include "rtl-sdr.h"

class SourceRtlSdr : public SourceBase
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GETSET(bool, isActive, IsActive);		// Size of the block in bytes

	private:
		/**********************************************************************\
		|* Private instance variables
		\**********************************************************************/
		rtlsdr_dev_t *		_dev;			// Device structure
		int64_t				_bufId;			// Buffer Id for IQ data
		int					_sampleRate;	// Sampling frequency in Hz

	public:
		/**********************************************************************\
		|* Constructor
		\**********************************************************************/
		explicit SourceRtlSdr(QObject *parent = nullptr);
		virtual ~SourceRtlSdr(void);

		/**********************************************************************\
		|* Return the information on how this source reports data
		\**********************************************************************/
		virtual StreamInfo streamInfo(void);

		/**********************************************************************\
		|* Return whether we managed to open a given instance
		\**********************************************************************/
		virtual bool open(int deviceId);

		/**********************************************************************\
		|* Set the sample-rate
		\**********************************************************************/
		virtual bool setSampleRate(int sampleRate);

		/**********************************************************************\
		|* Set the center-frequency
		\**********************************************************************/
		virtual bool setFrequency(int frequency);

		/**********************************************************************\
		|* Set the gain in dB
		\**********************************************************************/
		virtual bool setGain(double gain);

		/**********************************************************************\
		|* Set the antenna to use
		\**********************************************************************/
		virtual bool setAntenna(QString antenna);

		/**********************************************************************\
		|* Set the tuner bandwidth to use
		\**********************************************************************/
		virtual bool setBandwidth(int bandwidth);

		/**********************************************************************\
		|* Get a list of available antennas
		\**********************************************************************/
		virtual QList<QString> listAntennas(void);

		/**********************************************************************\
		|* Get a list of available bandwidth settings
		\**********************************************************************/
		virtual QList<QString> listBandwidths(void);

		/**********************************************************************\
		|* Get the number of available channels for RX and TX
		\**********************************************************************/
		virtual ChannelInfo numberOfChannels(void);

		/**********************************************************************\
		|* Get a list of frequency ranges, in Hz
		\**********************************************************************/
		virtual QList<Range> listFrequencyRanges(void);

		/**********************************************************************\
		|* Get a list of gains in dB
		\**********************************************************************/
		virtual QList<double> listGains(void);

		/**********************************************************************\
		|* Get the ranges within which you can sample via the ADC
		\**********************************************************************/
		virtual QList<SourceBase::Range> listSampleRateRanges(void);




		/**********************************************************************\
		|* Signal handler, has to be public to be invokable from 'C'
		\**********************************************************************/
		void _handleSignal(int signum);

		/**********************************************************************\
		|* Callback handler, has to be public to be invokable from 'C'
		\**********************************************************************/
		void _dataIncoming(uint8_t *data, uint32_t len);

	public slots:
		/**********************************************************************\
		|* Start sampling from the source
		\**********************************************************************/
		 virtual void startSampling(void);

		/**********************************************************************\
		|* Stop sampling from the source
		\**********************************************************************/
		 virtual void stopSampling(void);
	};

#endif // SOURCERTLSDR_H
