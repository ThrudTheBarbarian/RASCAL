#include <QCoreApplication>

#include "config.h"
#include "constants.h"
#include "datamgr.h"
#include "msgio.h"
#include "processor.h"
#include "soapyio.h"
#include "tester.h"

int main(int argc, char *argv[])
	{
	QCoreApplication a(argc, argv);

	/**************************************************************************\
	|* Set up the settings for the application
	\**************************************************************************/
	QCoreApplication::setOrganizationName(ORG_NAME);
	QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
	QCoreApplication::setApplicationName(DAEMON_NAME);
	QCoreApplication::setApplicationVersion(DAEMON_VERSION);

	/**************************************************************************\
	|* Set up the configuration from both settings and commandline
	\**************************************************************************/
	Config &cfg = Config::instance();

	/**************************************************************************\
	|* Set up the processing hierarchy
	\**************************************************************************/
	Processor processor(cfg, &a);

	/**************************************************************************\
	|* Set up the data stream
	\**************************************************************************/
	SoapyIO sio(&processor);

	/**************************************************************************\
	|* Configure the message-io handler (websocket based)
	\**************************************************************************/
	QThread networkThread;
	MsgIO mio;
	mio.init(Config::instance().networkPort());

	mio.moveToThread(&networkThread);
	networkThread.start();

	/**************************************************************************\
	|* Configure the processor
	\**************************************************************************/
	processor.init(&sio, &mio);

	/**************************************************************************\
	|* Start streaming data in
	\**************************************************************************/
	sio.startWorker();

	return a.exec();
	}
