#include <QGridLayout>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "constants.h"
#include "datamgr.h"
#include "events.h"
#include "graph.h"
#include "limiters.h"
#include "msgio.h"
#include "sky.h"
#include "vcr.h"
#include "waterfall.h"

Q_LOGGING_CATEGORY(log_gui, "rascal.ui    ")
#define LOG qDebug(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_gui) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor
\******************************************************************************/
MainWindow::MainWindow(Config *cfg, QWidget *parent)
		   :QMainWindow(parent)
		   ,_cfg(cfg)
		   ,ui(new Ui::MainWindow)
	{
	ui->setupUi(this);\
	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
MainWindow::~MainWindow()
	{
	delete ui;
	}

/******************************************************************************\
|* Programmatically create the UI
\******************************************************************************/
void MainWindow::createUI(Msgio *io)
	{
	_io			= io;
	_events		= new Events(this);
	_graph		= new Graph(this);
	_limits		= new Limiters(this);
	_sky		= new Sky(this);
	_vcr		= new Vcr(this);
	_waterfall	= new Waterfall(this);

	QGridLayout *grid = new QGridLayout;
	grid->addWidget(_graph, 0, 0, 16, 44);
	grid->addWidget(_sky, 0, 44, 16, 32);
	grid->addWidget(_limits, 16, 0, 1, 44);
	grid->addWidget(_waterfall, 17, 0, 15, 44);
	grid->addWidget(_events, 16, 44, 12, 32);
	grid->addWidget(_vcr, 28, 44, 4, 32);

	QWidget *top = findChild<QWidget *>("top");
	top->setLayout(grid);
	setWindowTitle("RASCAL");

	/**************************************************************************\
	|* Connect up the data-flow from io->* : update
	\**************************************************************************/
	connect(_io, &Msgio::updateReceived,
			this, &MainWindow::updateReceived);
	connect(this, &MainWindow::updateReady,
			_waterfall, &Waterfall::updateReceived);
	connect(this, &MainWindow::updateReady,
			_graph, &Graph::updateReceived);


	/**************************************************************************\
	|* Connect up the data-flow from io->* : sample
	\**************************************************************************/
	connect(_io, &Msgio::sampleReceived,
			this, &MainWindow::sampleReceived);
	connect(this, &MainWindow::sampleReady,
			_waterfall, &Waterfall::sampleReceived);
	connect(this, &MainWindow::sampleReady,
			_graph, &Graph::sampleReceived);
	}

/******************************************************************************\
|* Distribute the update data, handling the retain/release correctly
\******************************************************************************/
void MainWindow::updateReceived(int64_t bufferId)
	{
	DataMgr& dmgr		= DataMgr::instance();

	/**********************************************************************\
	|* Buffer comes to us with a retain-count of 1, so make sure it is sent
	|* to all destinations before we release the bufferId
	\**********************************************************************/
	emit updateReady(bufferId);
	dmgr.release(bufferId);
	}


/******************************************************************************\
|* Distribute the sample data, handling the retain/release correctly
\******************************************************************************/
void MainWindow::sampleReceived(int64_t bufferId)
	{
	DataMgr& dmgr		= DataMgr::instance();

	/**********************************************************************\
	|* Buffer comes to us with a retain-count of 1, so make sure it is sent
	|* to all destinations before we release the bufferId
	\**********************************************************************/
	emit sampleReady(bufferId);
	dmgr.release(bufferId);
	}

/******************************************************************************\
|* Menu action: We want to begin calibration
\******************************************************************************/
void MainWindow::on_actionBegin_calibration_triggered()
	{
	LOG << "Beginning calibration";
	_io->sendTextMessage("CALIBRATION BEGIN");
	}

/**********************************************************************\
|* Menu action: We want to stop calibration
\**********************************************************************/
void MainWindow::on_actionStop_calibration_triggered()
	{
	LOG << "Stopping calibration";
	_io->sendTextMessage("CALIBRATION END");
	}

/**********************************************************************\
|* Menu action: We want to load the calibration
\**********************************************************************/
void MainWindow::on_actionLoad_calibration_triggered()
	{
	LOG << "Loading calibration";
	_io->sendTextMessage("CALIBRATION LOAD");
	}

