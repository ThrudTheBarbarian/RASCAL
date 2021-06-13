#ifndef CONFIG_H
#define CONFIG_H

#include <QCommandLineParser>

#include "singleton.h"

class Config : public Singleton<Config>
	{
	private:
		QCommandLineParser		_parser;

	public:
		/**********************************************************************\
		|* Constructor
		\**********************************************************************/
		Config();

		/******************************************************************\
		|* Return the Host to connect to
		\******************************************************************/
		QString networkHost(void);

		/******************************************************************\
		|* Return the port to connect to
		\******************************************************************/
		int networkPort(void);

	};

#endif // CONFIG_H
