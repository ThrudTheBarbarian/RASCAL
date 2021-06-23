#ifndef MSGIO_H
#define MSGIO_H

#include <QList>
#include <QObject>
#include <QString>
#include <QMutexLocker>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

#include <libra.h>

class MsgIO: public QObject
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(bool, isCalibrating);			// Configuration object
	GET(int, calibration);				// Calibration data
	GET(int, calibrationPasses);		// Number of passes for calibration
	GET(bool, useCalibration);			// Are we using calibration ?
	GET(int, calData);					// Calibration data
	GET(int, calNum);					// Entries in calibration data

	public:
		/**********************************************************************\
		|* Typedefs and enums
		\**********************************************************************/

	private:
		/**********************************************************************\
		|* Begin storing data for calibration
		\**********************************************************************/
		void _beginCalibration(void);

		/**********************************************************************\
		|* Append to the calibration store
		\**********************************************************************/
		void _appendToCalibration(int64_t buffer);

		/**********************************************************************\
		|* Stop calibration
		\**********************************************************************/
		void _stopCalibration(void);

		/**********************************************************************\
		|* Load the calibration
		\**********************************************************************/
		void _loadCalibration(void);


		/**********************************************************************\
		|* Private variables
		\**********************************************************************/
		QWebSocketServer *		_server;		// Handle the connection
		QList<QWebSocket *>		_clients;		// List of connected clients
		QMutex					_lock;			// Thread safety


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

		/**********************************************************************\
		|* Send appropriate message types
		\**********************************************************************/
		void sendTextMessage(const QString &message);
		void sendBinaryMessage(const QByteArray &data);

	public slots:
		/**********************************************************************\
		|* Receive data ready to send out, from the aggregator
		\**********************************************************************/
		void newData(PreambleType type, int64_t bufferId);

	};

#endif // MSGIO_H
