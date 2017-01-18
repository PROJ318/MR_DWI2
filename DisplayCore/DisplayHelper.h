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
	* \brief DisplayPort(QGridLayout *grid) constructor. it manages the input QGridLayout.
	*
	* \param QGridLayout: a QT5.7 class
	*/
	explicit DisplayPort(QWidget * parent = 0);

	DisplayPort::~DisplayPort();
	/**
	* \brief DiffusionCore destructor.
	*/
	//virtual ~DisplayPort();

	/**
	* \brief imageTypeToString Get String of Image Type.
	*
	* \param parent is a pointer to the parent widget
	*/
	//std::string imageTypeToString(std::string imageLabel);
	//std::string imageTypeToString(int row, int col);

	void insertWindow(QWidget* wdw, std::string imageLabel);
	void removeWindow(std::string imageLabel);

	//QVTKWidget* getWindow(std::string imageLabel);

	void PrintWdwLayout();

	/**
	* \brief Initializes the widget. This method has to be called before widget can start.
	*/
	void Initialize();
protected:

	//    void ui_InsertWindow(int& rowInd, int& colInd, QWidget *vtkWindow, imageType imageLabel);
	//    void ui_RemoveWindow(imageType a);
	//    bool ui_IsWdWSquare();

	//    void ui_dumpWindow(int row, int col);
	//    void ui_findWdw(imageType imageLabel, int& row, int& col);

	//    QString ui_ImageName(int imageLabel);
	void index2Pos(int index, int& row, int& col);
	void pos2Index(int& index, int row, int col);

private:
	//QGridLayout* DC_gridlayout;
	//std::vector< std::vector<int> > DC_layoutMap; // Table used for tracing window content
	std::vector< const std::string > DC_LayoutMap;
};

#endif // DISPLAYPORT_H
