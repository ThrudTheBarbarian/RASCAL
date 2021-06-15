#ifndef MSGIO_H
#define MSGIO_H

#include <QObject>
#include <QList>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

#include "fftaggregator.h"

class MsgIO: public QObject
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Typedefs and enums
		\**********************************************************************/

	private:
		/**********************************************************************\
		|* Private variables
		\**********************************************************************/
		QWebSocketServer *		_server;		// Handle the connection
		QList<QWebSocket *>		_clients;		// List of connected clients


	private slots:
		/**********************************************************************\
		|* Private slots - generally for WebSocket operation
		\**********************************************************************/
		void onNewConnection(void);
		void socketDisconnected();
		void processTextMessage(const QString &message);
		void processBinaryMessage(QByteArray message);

	public:
		/**********************************************************************\
		|* Constructor / Destructor
		\**********************************************************************/
		explicit MsgIO(QObject *parent = nullptr);
		~MsgIO() override;

		/**********************************************************************\
		|* Initialise the server
		\**********************************************************************/
		void init(int port);

	public slots:
		/**********************************************************************\
		|* Receive data ready to send out, from the aggregator
		\**********************************************************************/
		void newData(FFTAggregator::DataType type, int64_t bufferId);

	};

#endif // MSGIO_H
