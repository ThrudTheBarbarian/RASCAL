#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QObject>
#include <QString>

class SourceBase : public QObject
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* typedefs and enums
		\**********************************************************************/
		typedef enum
			{
			STREAM_S8C		= 0,		// Signed, 8-bit data
			STREAM_S16C					// Signed, 16-bit data
			} StreamFormat;

		struct StreamInfo
			{
			StreamFormat format;		// Native format for dats
			double max;					// Maximum value in stream
			QString name;				// Name of the driver
			QString mode;				// Operating mode
			};

		struct ChannelInfo
			{
			int rx;						// Number of RX channels
			int tx;						// Number of TX channels
			};

		struct Range
			{
			double from;				// Low part of range
			double to;					// Top part of range
			};

		/**********************************************************************\
		|* Destructor
		\**********************************************************************/
		explicit SourceBase(QObject *parent = nullptr) : QObject(parent) {};
		virtual ~SourceBase(void) {};

		/**********************************************************************\
		|* Return the information on how this source reports data
		\**********************************************************************/
		virtual StreamInfo streamInfo(void) = 0;

		/**********************************************************************\
		|* Return whether we managed to open a given instance
		\**********************************************************************/
		virtual bool open(int deviceId) = 0;

		/**********************************************************************\
		|* Set the sample-rate
		\**********************************************************************/
		virtual bool setSampleRate(int sampleRate) = 0;

		/**********************************************************************\
		|* Set the center-frequency
		\**********************************************************************/
		virtual bool setFrequency(int frequency) = 0;

		/**********************************************************************\
		|* Set the gain in dB
		\**********************************************************************/
		virtual bool setGain(double gain) = 0;

		/**********************************************************************\
		|* Set the antenna to use
		\**********************************************************************/
		virtual bool setAntenna(QString antenna) = 0;

		/**********************************************************************\
		|* Set the tuner bandwidth to use
		\**********************************************************************/
		virtual bool setBandwidth(int bandwidth) = 0;

		/**********************************************************************\
		|* Get a list of available antennas
		\**********************************************************************/
		virtual QList<QString> listAntennas(void) = 0;

		/**********************************************************************\
		|* Get a list of available bandwidth settings, in kHz
		\**********************************************************************/
		virtual QList<QString> listBandwidths(void) = 0;

		/**********************************************************************\
		|* Get a list of frequency ranges, in MHz
		\**********************************************************************/
		virtual QList<Range> listFrequencyRanges(void) = 0;

		/**********************************************************************\
		|* Get a list of gains in dB
		\**********************************************************************/
		virtual QList<double> listGains(void) = 0;

		/**********************************************************************\
		|* Get the number of channels in each direction
		\**********************************************************************/
		virtual ChannelInfo numberOfChannels(void) = 0;

		/**********************************************************************\
		|* Get the number of channels in each direction
		\**********************************************************************/
		virtual QList<SourceBase::Range> listSampleRateRanges(void) = 0;

		/**********************************************************************\
		|* Convenience method to return the name
		\**********************************************************************/
		virtual QString name()
			{
			StreamInfo info = streamInfo();
			return info.name;
			};

		/**********************************************************************\
		|* Convenience method to return the mode
		\**********************************************************************/
		virtual QString mode()
			{
			StreamInfo info = streamInfo();
			return info.mode;
			};

		/**********************************************************************\
		|* Convenience method to return the format
		\**********************************************************************/
		virtual StreamFormat format()
			{
			StreamInfo info = streamInfo();
			return info.format;
			};

		/**********************************************************************\
		|* Convenience method to return the format name
		\**********************************************************************/
		virtual const char * formatName()
			{
			StreamInfo info = streamInfo();
			switch (info.format)
				{
				case STREAM_S8C:
					return "8-bit, signed, complex";
					break;

				case STREAM_S16C:
					return "16-bit, signed, complex";
					break;

				default:
					return "undefined";
					break;
				}
			};

	public slots:
		/**********************************************************************\
		|* Start sampling from the source
		\**********************************************************************/
		 virtual void startSampling(void) = 0;

		/**********************************************************************\
		|* Stop sampling from the source
		\**********************************************************************/
		 virtual void stopSampling(void) = 0;


	signals:
		/**********************************************************************\
		|* We have new data
		\**********************************************************************/
		void dataAvailable(int64_t bufId,
						   int samples,
						   int max,
						   StreamFormat fmt);

	};

#endif // SOURCEBASE_H
