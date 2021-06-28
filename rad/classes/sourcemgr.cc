#include <QThread>

#include "config.h"
#include "constants.h"
#include "processor.h"
#include "sourcebase.h"
#include "sourcemgr.h"
#include "sourcertlsdr.h"
#include "sourcesdrplay.h"

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

	/**************************************************************************\
	|* Handle commandline options to list out stats etc.
	\**************************************************************************/
	bool listed = false;
	if (_src)
		{
		if (Config::instance().listAntennas())
			{
			listed = true;
			QList<QString> antennas = _src->listAntennas();
			_showList("Antennas", "\n", "  ", antennas);
			}

		if (Config::instance().listChannels())
			{
			listed = true;
			SourceBase::ChannelInfo info = _src->numberOfChannels();
			printf("Channel count: rx=%d, tx=%d\n", info.rx, info.tx);
			}

		if (Config::instance().listBandwidths())
			{
			listed = true;
			QList<QString> bw = _src->listBandwidths();
			_showList("Bandwidths (kHz)", ",", " ", bw);
			}

		if (Config::instance().listFrequencyRanges())
			{
			listed = true;
			QList<SourceBase::Range> ranges = _src->listFrequencyRanges();
			QList<QString> info;
			for (SourceBase::Range& range : ranges)
				info.append(QString("%1 -> %2").arg(range.from).arg(range.to));
			_showList("Frequencies (MHz)", "\n", "  ", info);
			}

		if (Config::instance().listSampleRates())
			{
			listed = true;
			QList<SourceBase::Range> ranges = _src->listSampleRateRanges();
			QList<QString> info;
			for (SourceBase::Range& range : ranges)
				info.append(QString("%1 -> %2").arg(range.from).arg(range.to));
			_showList("Sample rates (MHz)", "\n", "  ", info);
			}

		if (Config::instance().listNativeFormat())
			{
			listed = true;
			SourceBase::StreamInfo info = _src->streamInfo();
			printf("Native format: %s [max=%f]\n", _src->formatName(), info.max);
			}

		if (Config::instance().listGains())
			{
			listed = true;
			QList<double> gains = _src->listGains();
			QList<QString> info;
			for (double gain : gains)
				info.append(QString("%1").arg(gain));
			_showList("Gains (dB)", ",", " ", info);
			}
		}

	if (listed)
		{
		printf("Exiting on request\n");
		fflush(stdout);
		exit(0);
		}
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
		if (ok)
			ok = _src->setBandwidth(Config::instance().bandwidth());
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
	srcs.append(new SourceRtlSdr());
	srcs.append(new SourceSdrPlay());

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
		_src->moveToThread(_thread);

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

/******************************************************************************\
|* Private method - show a list with a title
\******************************************************************************/
void SourceMgr::_showList(QString title,
						  const char *sep,
						  const char *prefix,
						  QList<QString>list)
	{
	int num = list.length();

	printf("%s:", qPrintable(title));
	if (num == 0)
		printf(" none\n");
	else if (num == 1)
		printf(" %s\n", qPrintable(list.at(0)));
	else
		{
		if (sep[strlen(sep)-1] == '\n')
			printf("\n");

		for (int i=0; i<num; i++)
			{
			printf("%s%s", prefix, qPrintable(list.at(i)));
			if (i < num-1)
				printf("%s", sep);
			}

		if (sep[strlen(sep)-1] != '\n')
			printf("\n");
		}
	}
