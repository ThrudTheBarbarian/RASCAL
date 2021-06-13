#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "properties.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_FORWARD_DECLARE_CLASS(Config);
QT_FORWARD_DECLARE_CLASS(Events);
QT_FORWARD_DECLARE_CLASS(Graph);
QT_FORWARD_DECLARE_CLASS(Limiters);
QT_FORWARD_DECLARE_CLASS(Sky);
QT_FORWARD_DECLARE_CLASS(Vcr);
QT_FORWARD_DECLARE_CLASS(Waterfall);

class MainWindow : public QMainWindow
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GETSET(Config*, cfg, Cfg);			// Configuration object
	GET(Events *, events);				// Discovered events
	GET(Graph *, graph);				// Frequency plot
	GET(Limiters *, limits);			// Only use part of the freq. range
	GET(Sky *, sky);					// Sky plot
	GET(Vcr *, vcr);					// Controls for playback etc
	GET(Waterfall *, waterfall);		// Waterfall display

	public:
		/***********************************************************************\
		|* Contructor/Destructor
		\**********************************************************************/
		MainWindow(Config * cfg = nullptr, QWidget *parent = nullptr);
		~MainWindow();

	private:
		/***********************************************************************\
		|* Private variables
		\**********************************************************************/
		Ui::MainWindow *ui;				// UI instance

		/***********************************************************************\
		|* Private method: instantiate the UI programmatically
		\**********************************************************************/
		void _createUI(void);
	};
#endif // MAINWINDOW_H