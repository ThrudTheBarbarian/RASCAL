#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QString>

class SourceBase
	{
	public:
		/**********************************************************************\
		|* typedefs and enums
		\**********************************************************************/
		typedef enum
			{
			STREAM_S8C		= 0,
			STREAM_S16C
			} StreamFormat;

		struct StreamInfo
			{
			StreamFormat format;
			QString name;
			QString mode;
			};

		/**********************************************************************\
		|* Destructor
		\**********************************************************************/
		virtual ~SourceBase(void)	{};

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
		virtual void dataAvailable(void) = 0;

	};

#endif // SOURCEBASE_H
