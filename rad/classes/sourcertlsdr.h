#ifndef SOURCERTLSDR_H
#define SOURCERTLSDR_H

#include <QObject>

#include "properties.h"
#include "sourcebase.h"
#include "rtl-sdr.h"

class SourceRtlSdr : public QObject, public SourceBase
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
		int					_bufId;			// Buffer Id for IQ data
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


	public slots:
		/**********************************************************************\
		|* Start sampling from the source
		\**********************************************************************/
		 virtual void startSampling(void);

		/**********************************************************************\
		|* Stop sampling from the source
		\**********************************************************************/
		 virtual void stopSampling(void);


	signals:
		/**********************************************************************\
		|* We have new data
		\**********************************************************************/
		virtual void dataAvailable(void);
	};

#endif // SOURCERTLSDR_H
