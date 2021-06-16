#include <QApplication>

#include <libra.h>

#include "config.h"
#include "mainwindow.h"
#include "msgio.h"


int main(int argc, char *argv[])
	{
	QApplication a(argc, argv);


	/**************************************************************************\
	|* Set up the settings for the application
	\**************************************************************************/
	QCoreApplication::setOrganizationName(ORG_NAME);
	QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
	QCoreApplication::setApplicationName(RASCAL_NAME);
	QCoreApplication::setApplicationVersion(RASCAL_VERSION);

	/**************************************************************************\
	|* Set up the datablock management
	\**************************************************************************/
	DataMgr::instance();

	/**************************************************************************\
	|* Set up the configuration from both settings and commandline
	\**************************************************************************/
	Config &cfg = Config::instance();

	/**************************************************************************\
	|* Set up a websocket connection to the server
	\**************************************************************************/
	Msgio  io(cfg.networkHost(), cfg.networkPort());
	io.connectToServer();

	/**************************************************************************\
	|* Show the window
	\**************************************************************************/
	MainWindow w(&cfg);
	w.createUI(&io);
	w.show();

	/**************************************************************************\
	|* Hand over to event processing
	\**************************************************************************/
	return a.exec();
	}
