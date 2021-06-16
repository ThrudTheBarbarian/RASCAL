#include <QtWebSockets>
#include <QWebSocketServer>

#include <libra.h>

#include "msgio.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
Q_LOGGING_CATEGORY(log_net, "rascal.net   ")

#define LOG qDebug(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_net) << QTime::currentTime().toString("hh:mm:ss.zzz")

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
MsgIO::MsgIO(QObject *parent) : QObject(parent)
	{}

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
	LOG << "WebSocket got: " << msg;
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
|* We have new smoothed data, send it off to all the clients. This comes in
|* as a buffer of floats, _fftSize long
\******************************************************************************/
void MsgIO::newData(PreambleType type, int64_t bufferId)
	{
	DataMgr &dmgr	= DataMgr::instance();

	size_t extent	= dmgr.extent(bufferId);
	uint8_t *src	= dmgr.asUint8(bufferId);

	int dstId		= dmgr.blockFor(extent+sizeof(Preamble));
	char *dst		= reinterpret_cast<char *>(dmgr.asUint8(dstId));

	if ((src == nullptr) || (dst == nullptr))
		{
		ERR << "Cannot get src(" << src <<") or dst(" << dst <<") in send";
		}
	else
		{
		Preamble hdr;
		hdr.extent	= (uint32_t)extent;
		hdr.type	= (uint16_t)type;
		memcpy(dst, &hdr, sizeof(Preamble));
		memcpy(dst+sizeof(Preamble), src, extent);

		const char * buffer = const_cast<char *>(dst);
		QByteArray msg(buffer, extent + sizeof(Preamble));
		for (QWebSocket *client : qAsConst(_clients))
			client->sendBinaryMessage(msg);

		dmgr.release(dstId);
		}

	dmgr.release(bufferId);
	}
