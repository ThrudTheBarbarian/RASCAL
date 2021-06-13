#ifndef GRAPH_H
#define GRAPH_H

#include <QWidget>

class Graph : public QWidget
	{
	Q_OBJECT

	public:
		/**********************************************************************\
		|* Construction
		\**********************************************************************/
		explicit Graph(QWidget *parent = nullptr);

		/**********************************************************************\
		|* Override the paint event
		\**********************************************************************/
		void paintEvent(QPaintEvent *event) override;

	signals:

	};

#endif // GRAPH_H
