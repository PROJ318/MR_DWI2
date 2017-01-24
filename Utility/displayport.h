#ifndef DISPLAYPORT_H
#define DISPLAYPORT_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>

class DisplayPort : public QGridLayout
{
	Q_OBJECT
public:

	//static const std::string Widget_ID;

	/**
	* \brief DisplayPort(QWidget *parent) constructor. DisplayPort is an expansion of QGridlayout
	*
	*/
	explicit DisplayPort(QWidget * parent = 0);

	/**
	* \brief DiffusionCore destructor.
	*/
	virtual ~DisplayPort();

	/**
	* \brief insertWindow(QWidget* wdw, std::string imageLabel) insert wdw widget whose name is imageLabel
	*
	* \param wdw is any QWidget, imageLabel is one unique name of this QWidget.
	*/
	void insertWindow(QWidget* wdw, const QString imageLabel);

	/**
	* \brief removeWindow(std::string imageLabel) remove wdw widget whose name is imageLabel
	*
	* \param imageLabel is one unique name of the QWidget going to be removed.
	*/
	void removeWindow(const QString imageLabel);

	/**
	* \brief PrintWdwLayout() print the widget names in current displayport
	*/
	void PrintWdwLayout();

	/**
	* \brief QWidget* getWindow(std::string imageLabel) returns the widget whose name is imageLabel.
	* \param imageLabel is one unique name of the QWidget going to be retrieved.
	*/
	QWidget* getWindow(const QString imageLabel);

	/**
	* \brief QHash < const std::string, QWidget * > getAllWindow(); get all widgets currently exist in display port
	*/
	QHash < const QString, QWidget * > getAllWindow();

	///**
	//* \brief std::vector<QWidget* > getAllWindowNames() get all widgets' name currently exist in display port
	//*/
	//std::vector< const QString > getAllWindowNames();

protected:

	void index2Pos(int index, int& row, int& col);
	void pos2Index(int& index, int row, int col);

private:
	//QGridLayout* DC_gridlayout;
	int getWidgetInd(const QString imageLabel);
	std::vector< const QString > DC_LayoutMap;
};

#endif // DISPLAYPORT_H
