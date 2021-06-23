#include <QFileInfo>
#include <QtWebSockets>
#include <QWebSocketServer>

#include <libra.h>

#include "config.h"
#include "msgio.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_net, "rascal.net   ")

#define LOG qDebug(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")

#define CALIBRATION_FILE "/calib.dat"

/******************************************************************************\
|* Helper function: Create an identifier for a connection
\******************************************************************************/
static QString getIdentifier(QWebSocket *peer)
	{
	return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
									   QString::number(peer->peerPort()));
	}

/******************************************************************************\
|* Constructor
\******************************************************************************/
MsgIO::MsgIO(QObject *parent)
	  :QObject(parent)
	  ,_isCalibrating(false)
	  ,_calibration(-1)
	  ,_calibrationPasses(0)
	  ,_useCalibration(false)
	  ,_calData(-1)
	  ,_calNum(0)
	{
	DataMgr &dmgr	= DataMgr::instance();
	/**************************************************************************\
	|* Check to see if there is any calibration data, if so, use it
	\**************************************************************************/
	QString appDir	= Config::instance().saveDir();
	QString file	= appDir + CALIBRATION_FILE;

	QFileInfo check(file);
	if (check.exists())
		{
		_calNum			= check.size() / sizeof(float);
		_calData		= dmgr.blockFor(_calNum, sizeof(float));
		float * data	= dmgr.asFloat(_calData);
		if (data != nullptr)
			{
			FILE *fp = fopen(qPrintable(file), "rb");
			if (fp != nullptr)
				{
				fread(data, sizeof(float), _calNum, fp);
				fclose(fp);
				_useCalibration = true;
				}
			else
				{
				ERR << "Cannot read calibration data";
				// Be explicit
				dmgr.release(_calNum);
				_calNum			= -1;
				_useCalibration = false;
				}
			}
		else
			ERR << "Cannot allocate calibration data space";
		}
	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
MsgIO::~MsgIO(void)
	{
	_server->close();
	}


/******************************************************************************\
|* Initialise
\******************************************************************************/
void MsgIO::init(int port)
	{
	_server = new QWebSocketServer(QStringLiteral("Data-Source"),
								   QWebSocketServer::NonSecureMode,
								   this);
	if (_server->listen(QHostAddress::Any, port))
		{
		LOG << "Starting network transport on port" << port;
		connect(_server, &QWebSocketServer::newConnection,
				this, &MsgIO::onNewConnection);
		}
	else
		ERR << "Cannot start network transport on port" << port;
	}

/******************************************************************************\
|* Handle a client connecting
\******************************************************************************/
void MsgIO::onNewConnection(void)
	{
	auto socket = _server->nextPendingConnection();
	LOG << "New connection: " << getIdentifier(socket);

	socket->setParent(this);

	connect(socket, &QWebSocket::textMessageReceived,
			this, &MsgIO::processTextMessage);
	connect(socket, &QWebSocket::disconnected,
			this, &MsgIO::socketDisconnected);
	connect(socket, &QWebSocket::binaryMessageReceived,
			this, &MsgIO::processBinaryMessage);

	_clients << socket;
	}


/******************************************************************************\
|* Handle a client command message
\******************************************************************************/
void MsgIO::processTextMessage(const QString& msg)
	{
	if (msg == "CALIBRATION BEGIN")
		_beginCalibration();
	else if (msg == "CALIBRATION END")
		_stopCalibration();
	else if (msg == "CALIBRATION LOAD")
		_loadCalibration();
	}

/******************************************************************************\
|* Handle a client binary message
\******************************************************************************/
void MsgIO::processBinaryMessage(QByteArray msg)
	{
	LOG << "WebSocket got binary. Length : " << msg.length();
	}

/******************************************************************************\
|* Client disconnected
\******************************************************************************/
void MsgIO::socketDisconnected(void)
	{
	QWebSocket *client = qobject_cast<QWebSocket *>(sender());

	if (client)
		{
		LOG << "Disconnection: " << getIdentifier(client);
		_clients.removeAll(client);
		client->deleteLater();
		}
	}


/******************************************************************************\
|* Send a text message
\******************************************************************************/
void MsgIO::sendTextMessage(const QString &message)
	{
	for (QWebSocket *client : qAsConst(_clients))
		client->sendTextMessage(message);
	}

/******************************************************************************\
|* Send a binary message
\******************************************************************************/
void MsgIO::sendBinaryMessage(const QByteArray &data)
	{
	for (QWebSocket *client : qAsConst(_clients))
		client->sendBinaryMessage(data);
	}

/******************************************************************************\
|* We have new smoothed data, send it off to all the clients. This comes in
|* as a buffer of floats, _fftSize long
\******************************************************************************/
void MsgIO::newData(PreambleType type, int64_t bufferId)
	{
	QMutexLocker guard(&_lock);
	DataMgr &dmgr	= DataMgr::instance();

	if ((type == TYPE_UPDATE) && _isCalibrating)
		_appendToCalibration(bufferId);

	size_t extent	= dmgr.extent(bufferId);
	float *src		= dmgr.asFloat(bufferId);

	if (_useCalibration)
		{
		int num = extent / sizeof(float);
		if (num == _calNum)
			{
			float *calValues = dmgr.asFloat(_calData);
			for (int i=0; i<num; i++)
				src[i] -= calValues[i];
			}
		else
			ERR << "Calibration range" << _calNum << " mismatch to " <<num;
		}

	int dstId		= dmgr.blockFor(extent+sizeof(Preamble));
	char *dst		= reinterpret_cast<char *>(dmgr.asUint8(dstId));

	if ((src == nullptr) || (dst == nullptr))
		{
		ERR << "Cannot get src(" << src <<":" << bufferId
			<< ") or dst(" << dst << ":" << dstId << ") in send";
		}
	else
		{
		Preamble hdr;
		hdr.extent	= (uint32_t)extent;
		hdr.type	= (uint16_t)type;
		::memcpy(dst, &hdr, sizeof(Preamble));
		::memcpy(dst+sizeof(Preamble), src, extent);

		const char * buffer = const_cast<char *>(dst);
		QByteArray msg(buffer, extent + sizeof(Preamble));
		for (QWebSocket *client : qAsConst(_clients))
			client->sendBinaryMessage(msg);

		dmgr.release(dstId);
		}

	dmgr.release(bufferId);
	}

/******************************************************************************\
|* Set up for starting a calibration run
\******************************************************************************/
void MsgIO::_beginCalibration(void)
	{
	LOG << "Beginning calibration";
	QMutexLocker guard(&_lock);
	if (_calibration >= 0)
		{
		DataMgr &dmgr = DataMgr::instance();
		dmgr.release(_calibration);
		}
	_calibration		= -1;
	_calibrationPasses	= 0;
	_isCalibrating		= true;
	}

/******************************************************************************\
|* Add calibration data. Note: does not need the lock, since it's only called
|* from within newData() which already has the lock
\******************************************************************************/
void MsgIO::_appendToCalibration(int64_t buffer)
	{
	LOG << "Appending calibration data";
	DataMgr &dmgr = DataMgr::instance();
	float * src   = dmgr.asFloat(buffer);
	int extent	  = dmgr.extent(buffer);
	int count	  = extent / sizeof(float);

	if (_calibration < 0)
		{
		LOG << "Creating calibration storage";
		_calibration = dmgr.blockFor(count, sizeof(double));
		double *dst  = dmgr.asDouble(_calibration);
		if (dst == nullptr)
			{
			ERR << "Cannot allocate calibration array";
			_calibration = -1;
			return;
			}
		memset(dst, 0, sizeof(double) * count);
		}

	double *dst  = dmgr.asDouble(_calibration);
	for (int i=0; i<count; i++)
		dst[i] += src[i];
	_calibrationPasses ++;
	}

/******************************************************************************\
|* Stop calibrating and write the calibration to storage
\******************************************************************************/
void MsgIO::_stopCalibration(void)
	{
	LOG << "Stopping calibration";
	if (_calibrationPasses == 0)
		ERR << "Need more data before we can save calibration";
	else if (_calibration > 0)
		{
		QMutexLocker guard(&_lock);
		DataMgr &dmgr	= DataMgr::instance();
		QString appDir	= Config::instance().saveDir();
		QString file	= appDir + CALIBRATION_FILE;

		double * data  = dmgr.asDouble(_calibration);
		int extent	  = dmgr.extent(_calibration);

		FILE *fp = fopen(qPrintable(file), "wb");
		if (fp != nullptr)
			{
			int num = extent / sizeof(double);
			float vals[num];
			for (int i=0; i<num; i++)
				vals[i] = data[i] / _calibrationPasses;
			fwrite(vals, sizeof(float), num, fp);
			fclose(fp);
			}
		else
			ERR << "Cannot open" << file << "for output";

		dmgr.release(_calibration);
		_calibration	= -1;
		_isCalibrating	= false;
		}
	else
		ERR << "Cannot save non-existent calibration data!";
	}

/******************************************************************************\
|* Stop calibrating and write the calibration to storage
\******************************************************************************/
void MsgIO::_loadCalibration(void)
	{
	QMutexLocker guard(&_lock);

	}
