#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>

#include <libra.h>

#include "config.h"
#include "tester.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_data) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_data) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* These are the keys we look for in the config file
\******************************************************************************/
#define RADIO_GROUP			"radio"
#define DSP_GROUP			"dsp"
#define NETWORK_GROUP		"network"
#define FILE_GROUP			"files"

#define DRIVER_KEY			"filter-driver"
#define MODEL_KEY			"filter-model"
#define ID_KEY				"filter-id"
#define ANTENNA_KEY			"antenna"

#define BANDWIDTH_KEY		"bandwidth"
#define FREQUENCY_KEY		"frequency"
#define GAIN_KEY			"gain"
#define SAMPLE_RATE_KEY		"sample-rate"

#define FFT_WINDOW_TYPE_KEY	"fft-window-type"
#define FFT_SIZE_KEY		"fft-size"
#define UPDATE_TIME_KEY		"fft-update-time"
#define SAMPLE_TIME_KEY		"fft-sample-time"

#define DEFAULT_FFT_SIZE	"1024"

#define NET_PORT_KEY		"network-port"
#define SAVE_DIR_KEY		"save-dir"

/******************************************************************************\
|* These are the commandline args we're managing
\******************************************************************************/
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_help,
		({"h", "help"}, "Show this useful help"))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_antenna,
		(ANTENNA_KEY, "Antenna to use (name or index)", "0"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_bandwidth,
		({"b", "bandwidth"}, "Tuner bandwidth to use", "-1"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_saveDir,
		({"d", SAVE_DIR_KEY}, "Directory to store data to", "~/.rad"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_driverFilter,
		(DRIVER_KEY, "Filter for the driver name", "rtlsdr"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_modeFilter,
		(MODEL_KEY, "Filter for the mode name", ""))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_idFilter,
		(ID_KEY, "Filter for the device-id", ""))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_frequency,
		({"f", "frequency"}, "Center-frequencty to tune to", "1420406000"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_fftSize,
		({"n", "fft-num-bins"}, "Size of the FFT in bins", DEFAULT_FFT_SIZE))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_fftWindow,
		({"w", "fft-window-type"}, "Window-type for FFT", "hamming"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_gain,
		({"g", "gain"}, "Gain to apply"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_networkPort,
		({"p", "network-port"}, "Network port to communicate over", "5417"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_sampleRate,
		({"s", "sample-rate"}, "Baseband Sample rate", "2048000"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_timeSample,
		({"t", "time-between-samples"}, "Time to aggregate data over", "300"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_test,
		("test", "Run self-tests on the code"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_timeUpdate,
		({"u", "time-between-updates"}, "Send an update every ...", "5"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_version,
		({"v", "version"}, "Display the program version"))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listAllInfo,
		("list-all", "List all info and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listAntennas,
		("list-antennas", "List antennas and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listChannels,
		("list-channels", "List channel info and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listGains,
		("list-gains", "List gains and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listFrequencies,
		("list-frequencies", "List frequency-ranges and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listSampleRates,
		("list-sample-rates", "List sample-rates and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listBandwidths,
		("list-bandwidths", "List baseband bandwidths and exit"))
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_listNativeFormat,
		("list-native-format", "List native streaming format and exit"))

/******************************************************************************\
|* Read configuration from both commandline and settings
\******************************************************************************/
Config::Config()
	   :_listAll(false)
	{
	_parser.setApplicationDescription("RASCAL daemon");
	_parser.addOption(*_antenna);
	_parser.addOption(*_bandwidth);
	_parser.addOption(*_saveDir);
	_parser.addOption(*_driverFilter);
	_parser.addOption(*_idFilter);
	_parser.addOption(*_frequency);
	_parser.addOption(*_fftSize);
	_parser.addOption(*_gain);
	_parser.addOption(*_help);
	_parser.addOption(*_listAllInfo);
	_parser.addOption(*_listAntennas);
	_parser.addOption(*_listChannels);
	_parser.addOption(*_listBandwidths);
	_parser.addOption(*_listFrequencies);
	_parser.addOption(*_listGains);
	_parser.addOption(*_listNativeFormat);
	_parser.addOption(*_listSampleRates);
	_parser.addOption(*_modeFilter);
	_parser.addOption(*_networkPort);
	_parser.addOption(*_sampleRate);
	_parser.addOption(*_test);
	_parser.addOption(*_timeSample);
	_parser.addOption(*_timeUpdate);
	_parser.addOption(*_version);
	_parser.addOption(*_fftWindow);

	_parser.parse(QCoreApplication::arguments());
	_listAll = _parser.isSet(*_listAllInfo);

	if (_parser.isSet(*_test))
		{
		Tester tester;
		tester.test();
		exit(0);
		}

	if (_parser.isSet(*_help))
		{
		QString help = _parser.helpText();
		fprintf(stderr, "%s\n\n"
			"Window types for the FFT can be (use name or index):\n"
			" 0: Rectangle     1: Hamming      2: Hanning\n"
			" 3: Blackman      4: Welch        5: Parzen\n\n"
			,qUtf8Printable(help)
			);
		exit(0);
		}


	if (_parser.isSet(*_version))
		_parser.showVersion();
	}

/******************************************************************************\
|* Get the directory to save stuff to, and make sure it exists
\******************************************************************************/
QString Config::saveDir(void)
	{
	if (_parser.isSet(*_saveDir))
		return _parser.value(*_saveDir);

	QSettings s;
	s.beginGroup(FILE_GROUP);
	QString dir = s.value(SAVE_DIR_KEY, "").toString();
	s.endGroup();

	if (dir.length() == 0)
		dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

	QDir directory(dir);
	directory.mkpath(".");
	return dir;
	}

/******************************************************************************\
|* Get the antenna to use, as a string so it can be an index or a name-substring
\******************************************************************************/
QString Config::antenna(void)
	{
	if (_parser.isSet(*_antenna))
		return _parser.value(*_antenna).toLower();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString antenna = s.value(ANTENNA_KEY, "0").toString().toLower();
	s.endGroup();
	return antenna;
	}

/******************************************************************************\
|* Get the driver filter
\******************************************************************************/
QString Config::radioDriverFilter(void)
	{
	if (_parser.isSet(*_driverFilter))
		return _parser.value(*_driverFilter).toLower();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString filter = s.value(DRIVER_KEY, "sdrplay").toString().toLower();
	s.endGroup();
	return filter;
	}


/******************************************************************************\
|* Get the mode filter
\******************************************************************************/
QString Config::radioModeFilter(void)
	{
	if (_parser.isSet(*_modeFilter))
		return _parser.value(*_modeFilter).toLower();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString filter = s.value(MODEL_KEY, "").toString().toLower();
	s.endGroup();
	return filter;
	}

/******************************************************************************\
|* Get the id filter
\******************************************************************************/
int Config::networkPort(void)
	{
	if (_parser.isSet(*_networkPort))
		return _parser.value(*_networkPort).toInt();

	QSettings s;
	s.beginGroup(NETWORK_GROUP);
	QString port = s.value(NET_PORT_KEY, "5417").toString();
	s.endGroup();
	return port.toInt();
	}

/******************************************************************************\
|* Get the id filter
\******************************************************************************/
int Config::radioIdFilter(void)
	{
	if (_parser.isSet(*_idFilter))
		return _parser.value(*_idFilter).toInt();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString filter = s.value(ID_KEY, "-1").toString();
	s.endGroup();
	return filter.toInt();
	}

/******************************************************************************\
|* Get the time between updates
\******************************************************************************/
double Config::secondsBetweenUpdates(void)
	{
	if (_parser.isSet(*_timeUpdate))
		return _parser.value(*_timeUpdate).toDouble();

	QSettings s;
	s.beginGroup(DSP_GROUP);
	double secs = s.value(UPDATE_TIME_KEY, "5.0").toString().toDouble();
	s.endGroup();
	return secs;
	}

/******************************************************************************\
|* Get the time between samples
\******************************************************************************/
double Config::secondsBetweenSamples(void)
	{
	if (_parser.isSet(*_timeSample))
		return _parser.value(*_timeSample).toDouble();

	QSettings s;
	s.beginGroup(DSP_GROUP);
	double secs = s.value(SAMPLE_TIME_KEY, "300.0").toString().toDouble();
	s.endGroup();
	return secs;
	}

/******************************************************************************\
|* Get the frequency to tune to
\******************************************************************************/
int Config::centerFrequency(void)
	{
	if (_parser.isSet(*_frequency))
		return _parser.value(*_frequency).toInt();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString freq = s.value(FREQUENCY_KEY, "1420406000").toString();
	s.endGroup();
	return freq.toInt();
	}

/******************************************************************************\
|* Get the bandwidth for the tuner
\******************************************************************************/
int Config::bandwidth(void)
	{
	if (_parser.isSet(*_bandwidth))
		return _parser.value(*_bandwidth).toInt();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString bw = s.value(BANDWIDTH_KEY, "-1").toString();
	s.endGroup();
	return bw.toInt();
	}

/******************************************************************************\
|* Get the gain to apply
\******************************************************************************/
double Config::gain(void)
	{
	if (_parser.isSet(*_gain))
		return _parser.value(*_gain).toDouble();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString gain = s.value(GAIN_KEY, "49").toString();
	s.endGroup();
	return gain.toDouble();
	}

/******************************************************************************\
|* Return the baseband sample rate to use
\******************************************************************************/
int Config::sampleRate(void)
	{
	if (_parser.isSet(*_sampleRate))
		return _parser.value(*_sampleRate).toInt();

	QSettings s;
	s.beginGroup(RADIO_GROUP);
	QString rate = s.value(SAMPLE_RATE_KEY, "2048000").toString();
	s.endGroup();
	return rate.toInt();
	}

/******************************************************************************\
|* Get the fft-size
\******************************************************************************/
int Config::fftSize(void)
	{
	if (_parser.isSet(*_fftSize))
		return _parser.value(*_fftSize).toInt();

	QSettings s;
	s.beginGroup(DSP_GROUP);
	QString rate = s.value(FFT_SIZE_KEY, DEFAULT_FFT_SIZE).toString();
	s.endGroup();
	return rate.toInt();
	}

/******************************************************************************\
|* Get the fft-windowing function
\******************************************************************************/
Config::WindowType Config::fftWindowType(void)
	{
	QString window = "";
	if (_parser.isSet(*_fftWindow))
		window = _parser.value(*_fftWindow);
	else
		{
		QSettings s;
		s.beginGroup(DSP_GROUP);
		window = s.value(FFT_WINDOW_TYPE_KEY, "hamming").toString();
		s.endGroup();
		}

	QMap<QString,Config::WindowType> map =
		{
			{"0", Config::W_RECTANGLE},
			{"1", Config::W_HAMMING},
			{"2", Config::W_HANNING},
			{"3", Config::W_BLACKMAN},
			{"4", Config::W_WELCH},
			{"5", Config::W_PARZEN},
			{"rectangle", Config::W_RECTANGLE},
			{"hamming", Config::W_HAMMING},
			{"hanning", Config::W_HANNING},
			{"blackman", Config::W_BLACKMAN},
			{"welch", Config::W_WELCH},
			{"parzen", Config::W_PARZEN},
		};

	QString key = window.toLower();
	if (map.contains(key))
		return map[key];

	qWarning() << "Cannot find window function for " << key << " - using Hamming";
	return Config::W_HAMMING;
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listAntennas(void)
	{
	return _listAll || _parser.isSet(*_listAntennas);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listGains(void)
	{
	return _listAll || _parser.isSet(*_listGains);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listFrequencyRanges(void)
	{
	return _listAll || _parser.isSet(*_listFrequencies);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listSampleRates(void)
	{
	return _listAll || _parser.isSet(*_listSampleRates);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listBandwidths(void)
	{
	return _listAll || _parser.isSet(*_listBandwidths);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listNativeFormat(void)
	{
	return _listAll || _parser.isSet(*_listNativeFormat);
	}

/******************************************************************************\
|* Get whether to list out the antennas
\******************************************************************************/
bool Config::listChannels(void)
	{
	return _listAll || _parser.isSet(*_listChannels);
	}

