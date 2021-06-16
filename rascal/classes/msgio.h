#ifndef MSGIO_H
#define MSGIO_H

#include <QObject>
#include <QtWebSockets/QWebSocket>

#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(QString)

class Msgio : public QObject
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Typedefs and enums
		\**********************************************************************/
		struct SampleHeader
			{
			uint16_t order;
			uint16_t offset;
			uint32_t extent;
			uint16_t type;
			uint16_t flags;

			SampleHeader(void)
				{
				order	= 0xAA55;
				offset	= sizeof(SampleHeader);
				extent	= 0;
				type	= 0;
				flags	= 0;
				}
			};

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(QString, host);			// Hostname to connect to
	GET(int, port);				// Port to talk to
	GET(QWebSocket, socket);	// Actual socket to use
	GETSET(QUrl, url, setUrl);	// url to specify connection
	GET(bool, isConnected);		// Are we connected to the server

	public:
		explicit Msgio(QString host, int port, QObject *parent = nullptr);
		explicit Msgio(QUrl url, QObject *parent = nullptr);

	/**********************************************************************\
	|* Connect to the remote service, or fail in the attempt
	\**********************************************************************/
	void connectToServer(void);

	private slots:
		void onConnected();
		void onTextMessageReceived(const QString message);
		void onBinaryMessageReceived(const QByteArray &message);
		void closed();

	signals:
		void sampleReceived(int64_t bufferId);
		void updateReceived(int64_t bufferId);

	};

#endif // MSGIO_H
