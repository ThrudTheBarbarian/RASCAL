#ifndef SOURCEMGR_H
#define SOURCEMGR_H

#include <QObject>
QT_FORWARD_DECLARE_CLASS(SourceBase)
QT_FORWARD_DECLARE_CLASS(Processor)
QT_FORWARD_DECLARE_CLASS(QThread)

class SourceMgr : public QObject
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Typedefs and enums
		\**********************************************************************/
		enum
			{
			AUTOMATIC_GAIN		= -1
			};

	private:
		/**********************************************************************\
		|* Private variables
		\**********************************************************************/
		SourceBase *	_src;					// The actual source we got
		QThread *		_thread;				// The background thread

		/**********************************************************************\
		|* Private methods
		\**********************************************************************/
		void _findMatchingSource(void);

	public:
		/**********************************************************************\
		|* Constructor etc
		\**********************************************************************/
		explicit SourceMgr(QObject *parent = nullptr);
		~SourceMgr(void);

		/**********************************************************************\
		|* Did we find a source based on the criteria
		\**********************************************************************/
		bool foundSource(void);

		/**********************************************************************\
		|* Initialise the source
		\**********************************************************************/
		bool initialiseSource(void);

		/**********************************************************************\
		|* Initialise the source
		\**********************************************************************/
		bool start(Processor *processor);

	signals:
		/**********************************************************************\
		|* Start/Stop a source sampling
		\**********************************************************************/
		void startSourceSampling(void);
		void stopSourceSampling(void);
	};

#endif // SOURCEMGR_H
