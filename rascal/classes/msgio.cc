#include <libra.h>

#include "msgio.h"

#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
Msgio::Msgio(QString host, int port, QObject *parent)
	 : QObject(parent)
	 ,_host(host)
	 ,_port(port)
	 ,_isConnected(false)
	{
	_url.setHost(host);
	_url.setPort(port);
	}
/******************************************************************************\
|* Constructor
\******************************************************************************/
Msgio::Msgio(QUrl url, QObject *parent)
	  :QObject(parent)
	  ,_url(url)
	  ,_isConnected(false)
	{
	}

/******************************************************************************\
|* Connect to the remote service, or fail in the attempt
\******************************************************************************/
void Msgio::connectToServer(void)
	{
	_isConnected = false;

	_url.setScheme("ws");

	LOG << "Connecting to server at " << _url;
	connect(&_socket, &QWebSocket::connected,
			this, &Msgio::onConnected);
	_socket.open(_url);
	}


/******************************************************************************\
|* Handle connection
\******************************************************************************/
void Msgio::onConnected(void)
	{
	LOG << "Connected to server";

	connect(&_socket, &QWebSocket::binaryMessageReceived,
			this, &Msgio::onBinaryMessageReceived);
	connect(&_socket, &QWebSocket::textMessageReceived,
			this, &Msgio::onTextMessageReceived);
	connect(&_socket, &QWebSocket::disconnected,
			this, &Msgio::closed);

	_isConnected = true;
	}

/******************************************************************************\
|* Handle disconnection
\******************************************************************************/
void Msgio::closed(void)
	{
	LOG << "Server connection closed";
	_isConnected = false;
	}

/******************************************************************************\
|* Handle text messages
\******************************************************************************/
void Msgio::onTextMessageReceived(const QString msg)
	{
	LOG << "Got text message " << msg;
	}

/******************************************************************************\
|* Handle binary data
\******************************************************************************/
void Msgio::onBinaryMessageReceived(const QByteArray &msg)
	{
	char * ptr = const_cast<char *>(msg.data());
	Preamble *hdr = reinterpret_cast<Preamble*>(ptr);
	if (hdr == nullptr)
		{
		ERR << "Somehow managed to get a nil pointer to the byte array!";
		exit(-1);
		}

	if (hdr->isByteSwapped())
		{
		ERR << "Unimplemented: need to byte-swap incoming message data";
		return;
		}

	DataMgr& dmgr		= DataMgr::instance();
	int block			= dmgr.blockFor(hdr->extent);
	uint8_t *dst		= dmgr.asUint8(block);
	if (dst != nullptr)
		{
		memcpy(dst, ptr + hdr->offset, hdr->extent);
		switch (hdr->type)
			{
			case TYPE_UPDATE:
				LOG << "Update";
				emit updateReceived(block);
				break;

			case TYPE_SAMPLE:
				LOG << "Sample";
				emit sampleReceived(block);
				break;

			default:
				LOG << "Uh ?";
				dmgr.release(block);
				break;
			}
		}
	else
		ERR << "Cannot get data for block " << block;
	}
