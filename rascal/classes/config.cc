#include <QCoreApplication>
#include <QObject>
#include <QSettings>

#include "config.h"

#define NETWORK_GROUP		"network"
#define NET_ADDR_KEY		"network-address"
#define NET_PORT_KEY		"network-port"

#define DEFAULT_HOST		"shed.gornall.net"

/******************************************************************************\
|* These are the commandline args we're managing
\******************************************************************************/
Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_help,
		({"h", "help"}, "Show this useful help"))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_ipaddress,
		({"a",NET_ADDR_KEY}, "Address (ip or name) to connect to)", DEFAULT_HOST))

Q_GLOBAL_STATIC_WITH_ARGS(const QCommandLineOption,
		_port,
		({"p",NET_PORT_KEY}, "Address (ip or name) to connect to)", "5417"))

/******************************************************************************\
|* Constructor
\******************************************************************************/
Config::Config()
	{
	_parser.setApplicationDescription("RASCAL");
	_parser.addOption(*_ipaddress);
	_parser.addOption(*_help);
	_parser.addOption(*_port);

	_parser.parse(QCoreApplication::arguments());

	if (_parser.isSet(*_help))
		{
		QString help = _parser.helpText();
		fprintf(stderr, "%s\n\n"
			,qUtf8Printable(help)
			);
		exit(0);
		}
	}

/******************************************************************************\
|* Get the port to use
\******************************************************************************/
int Config::networkPort(void)
	{
	if (_parser.isSet(*_port))
		return _parser.value(*_port).toInt();

	QSettings s;
	s.beginGroup(NETWORK_GROUP);
	QString port = s.value(NET_PORT_KEY, "5417").toString();
	s.endGroup();
	return port.toInt();
	}

/******************************************************************************\
|* Get the host to connect to
\******************************************************************************/
QString Config::networkHost(void)
	{
	if (_parser.isSet(*_ipaddress))
		return _parser.value(*_ipaddress);

	QSettings s;
	s.beginGroup(NETWORK_GROUP);
	QString host = s.value(NET_PORT_KEY, DEFAULT_HOST).toString();
	s.endGroup();
	return host;
	}
