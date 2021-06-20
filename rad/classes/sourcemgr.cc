#include <QThread>

#include "config.h"
#include "constants.h"
#include "processor.h"
#include "sourcebase.h"
#include "sourcemgr.h"
#include "sourcertlsdr.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_dsp, "rascal.dsp   ")
Q_LOGGING_CATEGORY(log_src, "rascal.src   ")

#define LOG  qDebug(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define WARN qWarning(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR	 qCritical(log_dsp) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
SourceMgr::SourceMgr(QObject *parent)
		  :QObject(parent)
		  ,_src(nullptr)
	{
	_findMatchingSource();
	}


/******************************************************************************\
|* Destructor
\******************************************************************************/
SourceMgr::~SourceMgr(void)
	{
	if (_src)
		delete _src;
	}


/******************************************************************************\
|* Return if we have a source
\******************************************************************************/
bool SourceMgr::foundSource(void)
	{
	return (_src != nullptr);
	}

/******************************************************************************\
|* Initialise the parameters for the source
\******************************************************************************/
bool SourceMgr::initialiseSource(void)
	{
	bool ok = false;
	if (_src != nullptr)
		{
		ok = _src->setSampleRate(Config::instance().sampleRate());
		if (ok)
			ok = _src->setFrequency(Config::instance().centerFrequency());
		if (ok)
			ok = _src->setGain(Config::instance().gain());
		if (ok)
			ok = _src->setAntenna(Config::instance().antenna());
		}

	return ok;
	}

/******************************************************************************\
|* Determine the radio to use by probing and filtering the results
\******************************************************************************/
void SourceMgr::_findMatchingSource(void)
	{
	QString devFilter	= Config::instance().radioDriverFilter();
	QString modeFilter	= Config::instance().radioModeFilter();
	int idFilter		= Config::instance().radioIdFilter();

	/**************************************************************************\
	|* Create instances of all the sources we know about
	\**************************************************************************/
	QList<SourceBase *> srcs;
	SourceRtlSdr *rtlsdr = new SourceRtlSdr();
	srcs.append(rtlsdr);

	/**************************************************************************\
	|* Find the first entry in the list that matches the criteria
	\**************************************************************************/
	_src = nullptr;
	for (SourceBase *src : srcs)
		{
		bool driverFound			= true;
		bool modeFound				= true;

		SourceBase::StreamInfo info	= src->streamInfo();

		if ((devFilter.length() > 0) && (info.name.length() > 0))
			if (!info.name.contains(devFilter))
				driverFound = false;

		if ((modeFilter.length() > 0) && (info.mode.length() > 0))
			if (!info.mode.contains(modeFilter))
					modeFound = false;

		bool found = driverFound & modeFound;
		if (found && src->open(idFilter))
			{
			_src = src;
			break;
			}
		}

	/**************************************************************************\
	|* Delete all the instances that we didn't match
	\**************************************************************************/
	for (SourceBase *src : srcs)
		if (src != _src)
			delete src;

	/**************************************************************************\
	|* If we're still null, then show an error
	\**************************************************************************/
	if (_src == nullptr)
		{
		QString msg = QString("Cannot match device using {driver:%1, mode:%2}").
				arg(devFilter,modeFilter);
		ERR << msg;
		}
	}

/******************************************************************************\
|* Start the source sampling
\******************************************************************************/
bool SourceMgr::start(Processor *processor)
	{
	bool ok = false;
	if (_src != nullptr)
		{
		_thread = new QThread(this);
		QObject *instance = dynamic_cast<QObject *>(_src);
		instance->moveToThread(_thread);

		connect(this, &SourceMgr::startSourceSampling,
				_src, &SourceBase::startSampling);
		connect(this, &SourceMgr::stopSourceSampling,
				_src, &SourceBase::stopSampling);
		connect(_src, &SourceBase::dataAvailable,
				processor, &Processor::dataReceived);
		_thread->start();

		emit startSourceSampling();
		ok = true;
		}

	return ok;
	}
