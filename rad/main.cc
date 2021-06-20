#include <QCoreApplication>
#include <SoapySDR/Logger.hpp>

#include "config.h"
#include "constants.h"
#include "datamgr.h"
#include "msgio.h"
#include "processor.h"
#include "sourcemgr.h"
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
	SourceMgr srcmgr;
	if (!srcmgr.foundSource())
		{
		fprintf(stderr, "Cannot find source. Exiting\n");
		exit(-1);
		}
	srcmgr.initialiseSource();

	//SoapyIO sio(&processor);

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
	//processor.init(&sio, &mio);

	/**************************************************************************\
	|* Start streaming data in
	\**************************************************************************/
	//sio.startWorker();
	srcmgr.start(&processor);

	return a.exec();
	}
