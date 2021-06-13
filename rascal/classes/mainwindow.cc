#include <QGridLayout>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "constants.h"
#include "events.h"
#include "graph.h"
#include "limiters.h"
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
	_createUI();
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
void MainWindow::_createUI(void)
	{
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
	}
