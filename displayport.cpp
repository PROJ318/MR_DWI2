#include "displayport.h"
#include "qdebug.h"

DisplayPort::DisplayPort(QWidget *grid)
{
	//this->DC_gridlayout = grid;
}

DisplayPort::~DisplayPort()
{
	//delete DC_LayoutMap;
}

void DisplayPort::index2Pos(int i, int& row, int& col)
{
	int Tf = floor(sqrt(i));
	int Tc = ceil(sqrt(i));
	if (Tf == Tc)
	{
		row = Tf - 1;
		col = Tf - 1;
	}
	else{
		if (i < (Tf*Tf + Tc))
		{
			row = i - Tf*Tf - 1;
			col = Tc - 1;
		}
		else
		{
			row = Tc - 1;
			col = Tc - Tc*Tc + i - 1;
		}
	}
}

void DisplayPort::pos2Index(int& i, int row, int col)
{
	if (row >= col)
	{
		i = row*row + row + col + 1;
	}
	else
	{
		i = col*col + row + 1;
	}
}

void DisplayPort::insertWindow(QWidget* wdw, const QString imageLabel)
{
	DC_LayoutMap.push_back(imageLabel);
	int index = DC_LayoutMap.size();

	//qDebug()<<"index = "<<index;
	int row(-9), col(-9);
	index2Pos(index, row, col);
	//qDebug()<<"row = "<<row<<" col ="<<col<<endl;
	this->addWidget(wdw, row, col);
	this->update();
}

QWidget* DisplayPort::getWindow(const QString imageLabel)
{
	int counter = getWidgetInd(imageLabel);
	if (counter > 0) //imageLabel exists
	{
		int row(-1), col(-1);
		index2Pos(counter, row, col);
		qDebug() << "[DISPLAYPORT] Retrieving widget " << imageLabel << "from <" << row << "-" << col << ">" << endl;
		QLayoutItem *Item = this->itemAtPosition(row, col);
		QWidget * Widget = Item->widget();
		return Widget;
	}
	else
	{
		return NULL;
	}
}

QHash <const QString, QWidget * >  DisplayPort::getAllWindow()
{
	QHash <const QString, QWidget * > curWidgets;
	std::vector<const QString>::iterator it;
	int index;
	for (it = DC_LayoutMap.begin(), index = 1; it != DC_LayoutMap.end(); it++, index++)
	{
		int row, col;
		index2Pos(index, row, col);
		QLayoutItem *Item = this->itemAtPosition(row, col);
		QWidget * Widget = Item->widget();
		if (Widget != NULL) {
			curWidgets.insert(*it, Widget);
		}
	}
	return curWidgets;
}

int DisplayPort::getWidgetInd(const QString imageLabel)
{
	int flag(-1), counter(-1); //the first one cannot be removed
	for (counter = 1; (counter < DC_LayoutMap.size() + 1) && flag < 0;)
	{
		//qDebug() << "[DISPLAYPORT]" << DC_LayoutMap[counter - 1].c_str() << "_" << imageLabel.c_str() << "_" << endl;
		if (DC_LayoutMap[counter-1] == imageLabel)//if equal
		{
			flag = 1;
			//qDebug() << "Found widget at counter = " << counter << endl;
		}
		else{
			counter++;			
		}
	}

	if (flag > 0)
	{
		return counter;
	}
	else
	{
		return -1;
	}

}

void DisplayPort::removeWindow(const QString imageLabel)
{
	
	int counter = getWidgetInd(imageLabel);

	//2. If found.

	if (counter > 0) //imageLabel exists
	{
		//2.1, Adjust the LayoutMap.
		int rowF(-1), colF(-1), rowT(-1), colT(-1);
		//qDebug() << "[DISPLAYPORT] counter = " << counter << endl;

		index2Pos(counter, rowT, colT); //deleting pos

		index2Pos(DC_LayoutMap.size(), rowF, colF); //last pos




		//qDebug() << "[DISPLAYPORT]moving window from " << rowF << ":" << colF << " to " << rowT << ":" << colT << endl;
		QLayoutItem *ItemF = this->itemAtPosition(rowF, colF);
		QWidget * WidgetF = ItemF->widget();
		QLayoutItem *ItemT = this->itemAtPosition(rowT, colT);
		QWidget * WidgetT = ItemT->widget();

		if (WidgetF != NULL) {
			this->addWidget(WidgetF, rowT, colT); //move F to T pos.
		}
		//qDebug << "There is a widget ? " << (existWidget != NULL) << std::endl;
		if (WidgetT != NULL) {
			//this->removeWidget(WidgetT);
			WidgetT->setParent(NULL);
			delete WidgetT;
		}

		//STEP2, modify the DC_Windoes accordingly
		std::iter_swap(DC_LayoutMap.begin()+counter-1, DC_LayoutMap.end() - 1);
		DC_LayoutMap.pop_back();
		//this->PrintWdwLayout();
	}

}

void DisplayPort::PrintWdwLayout()
{
	qDebug() << "[DISPLAYPORT] TOTAL " << DC_LayoutMap.size() << " WINDOWS ARE RENDERING" << endl;
	std::vector<const QString >::iterator it;
	int index;
	for (it = DC_LayoutMap.begin(), index = 1; it != DC_LayoutMap.end(); it++, index++)
	{
		int row, col;
		index2Pos(index, row, col);
		qDebug() << "[DISPLAYPORT] <" << row << " , " << col << "> renders->" << (*it) << endl;
	}
}
