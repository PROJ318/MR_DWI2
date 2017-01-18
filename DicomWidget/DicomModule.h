#ifndef DicomModule_h
#define DicomModule_h


// MITK
// #include "DicomEventHandler.h"
// #include "QmitkDicomDataEventPublisher.h"
// #include "QmitkDicomDirectoryListener.h"
// #include "QmitkStoreSCPLauncher.h"
// #include "QmitkStoreSCPLauncherBuilder.h"



// Qt
#include <QHash>
#include <QLabel>
#include <QProgressDialog>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWidget>

namespace Ui{
	class DicomDialog;
}

/**
* \brief DicomModule is an editor providing functionality for dicom storage and import and query retrieve functionality.
*
*/
class DicomModule : public QWidget
{
	// this is needed for all Qt objects that should have a Qt meta-object
	// (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:

	/**
	* \brief DicomModule constructor.
	*/
	DicomModule(QWidget *parent);

	/**
	* \brief DicomModule destructor.
	*/
	virtual ~DicomModule();

	/**
	* \brief Init initialize the editor.
	*/

	//void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;

	//void SetFocus() override;
	//void DoSave() override {}
	//void DoSaveAs() override {}
	//bool IsDirty() const override { return false; }
	//bool IsSaveAsAllowed() const override { return false; }

	//virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

signals:

	/**
	* \brief SignalStartDicomImport is enitted when dicom directory for import was selected.
	*/
	//void SignalStartDicomImport(const QString&);
	
	/**
	* \brief emitted when view button is clicked.
	*/
	void SignalDicomRead(const QStringList&);
	protected slots:

	/// \brief Called when import is finished.
	void OnDicomImportFinished();

	///// \brief Called when Query Retrieve or Import Folder was clicked.
	//void OnTabChanged(int);

	///// \brief Called when view button is clicked. Sends out an event for adding the current selected file to the mitkDataStorage.
	//void OnViewButtonAddToDataManager(QHash<QString, QVariant> eventProperties);

	///// \brief Called when status of dicom storage provider changes.
	//void OnStoreSCPStatusChanged(const QString& status);

	///// \brief Called when dicom storage provider emits a network error.
	//void OnDicomNetworkError(const QString& status);

	/// \brief pass the signal to public
	void onSignalDicomToDataManager(const QStringList& pathlist);

protected:

	///// \brief StartStoreSCP starts  dicom storage provider.
	//void StartStoreSCP();

	///// \brief StopStoreSCP stops dicom storage provider.
	//void StopStoreSCP();

	///// \brief TestHandler initializes event handler.
	//void TestHandler();

	///// \brief CreateTemporaryDirectory creates temporary directory in which temorary dicom objects are stored.
	//void CreateTemporaryDirectory();

	///// \brief StartDicomDirectoryListener starts dicom directory listener.
	//void StartDicomDirectoryListener();


	/**
	* \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_DicomModuleControls.h.
	*
	* \param parent is a pointer to the parent widget
	*/
	void CreateQtPartControl(QWidget *parent);

	/// \brief SetPluginDirectory Sets plugin directory.
	//void SetPluginDirectory();

	//Events::Types GetPartEventTypes() const override;

	//ctkFileDialog* m_ImportDialog;
	Ui::DicomDialog* m_Controls;
	//QmitkDicomDirectoryListener* m_DicomDirectoryListener;
	//QmitkStoreSCPLauncherBuilder m_Builder;
	//QmitkStoreSCPLauncher* m_StoreSCPLauncher;
	//DicomEventHandler* m_Handler;
	//QmitkDicomDataEventPublisher* m_Publisher;
	QString m_PluginDirectory;
	QString m_TempDirectory;
	QString m_DatabaseDirectory;
};

#endif // DicomModule_h
