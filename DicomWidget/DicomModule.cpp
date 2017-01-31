#include "DicomModule.h"
#include "ui_DicomModule.h"



DicomModule::DicomModule(QWidget *parent)
	//: m_DicomDirectoryListener(new QmitkDicomDirectoryListener())
	//, m_StoreSCPLauncher(new QmitkStoreSCPLauncher(&m_Builder))
	//, m_Publisher(new QmitkDicomDataEventPublisher())
	: QWidget(parent)
	, m_Controls(nullptr)
{
	CreateQtPartControl(this);
}

DicomModule::~DicomModule()
{
	//delete m_DicomDirectoryListener;
	//delete m_StoreSCPLauncher;
	//delete m_Handler;
	//delete m_Publisher;
}

void DicomModule::CreateQtPartControl(QWidget *parent)
{
	if (!m_Controls)
	{
		m_Controls = new Ui::DicomDialog;
		m_Controls->setupUi(parent);
		//m_Controls.StoreSCPStatusLabel->setTextFormat(Qt::RichText);
		//m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-offline_16.png'>");
		//TestHandler();
		//OnPreferencesChanged(nullptr);
		//CreateTemporaryDirectory();
		//StartDicomDirectoryListener();

		m_Controls->m_ctkDICOMQueryRetrieveWidget->useProgressDialog(true);

		connect(m_Controls->externalDataWidget, SIGNAL(SignalStartDicomImport(const QStringList&)),
			m_Controls->internalDataWidget, SLOT(OnStartDicomImport(const QStringList&)));

		connect(m_Controls->internalDataWidget, SIGNAL(SignalFinishedImport()), this, SLOT(OnDicomImportFinished()));

		connect(m_Controls->internalDataWidget, SIGNAL(SignalDicomToDataManager(QStringList)),
			this, SLOT(onSignalDicomToDataManager(QStringList)));

		connect(m_Controls->externalDataWidget, SIGNAL(SignalDicomToDataManager(QStringList)),
			this, SLOT(onSignalDicomToDataManager(QStringList)));
	}
}


//void DicomModule::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
//{
//	this->SetSite(site);
//	this->SetInput(input);
//}

void DicomModule::onSignalDicomToDataManager(const QStringList& path)
{
	std::cout << "paths signal sent" << std::endl;
	emit SignalDicomRead(path);
}

//void DicomModule::SetFocus()
//{
//}

//berry::IPartListener::Events::Types DicomModule::GetPartEventTypes() const
//{
//	return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
//}

//void DicomModule::OnTabChanged(int page)
//{
//	if (page == 2)//Query/Retrieve is selected
//	{
//		QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
//		QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
//		if (!((m_Builder.GetAETitle()->compare(storageAET, Qt::CaseSensitive) == 0) &&
//			(m_Builder.GetPort()->compare(storagePort, Qt::CaseSensitive) == 0)))
//		{
//			StopStoreSCP();
//			StartStoreSCP();
//		}
//	}
//}

void DicomModule::OnDicomImportFinished()
{
	m_Controls->tabWidget->setCurrentIndex(0);
}

//void DicomModule::StartDicomDirectoryListener()
//{
//	m_DicomDirectoryListener->SetDicomListenerDirectory(m_TempDirectory);
//	m_DicomDirectoryListener->SetDicomFolderSuffix(TEMP_DICOM_FOLDER_SUFFIX);
//	connect(m_DicomDirectoryListener, SIGNAL(SignalStartDicomImport(const QStringList&)), m_Controls.internalDataWidget, SLOT(OnStartDicomImport(const QStringList&)), Qt::DirectConnection);
//}


//void DicomModule::TestHandler()
//{
//	m_Handler = new DicomEventHandler();
//	m_Handler->SubscribeSlots();
//}

//void DicomModule::OnViewButtonAddToDataManager(QHash<QString, QVariant> eventProperties)
//{
//	ctkDictionary properties;
//	//    properties["PatientName"] = eventProperties["PatientName"];
//	//    properties["StudyUID"] = eventProperties["StudyUID"];
//	//    properties["StudyName"] = eventProperties["StudyName"];
//	//    properties["SeriesUID"] = eventProperties["SeriesUID"];
//	//    properties["SeriesName"] = eventProperties["SeriesName"];
//	properties["FilesForSeries"] = eventProperties["FilesForSeries"];
//	if (eventProperties.contains("Modality"))
//	{
//		properties["Modality"] = eventProperties["Modality"];
//	}
//
//	m_Publisher->PublishSignals(mitk::PluginActivator::getContext());
//	m_Publisher->AddSeriesToDataManagerEvent(properties);
//}


//void DicomModule::StartStoreSCP()
//{
//	QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
//	QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
//	m_Builder.AddPort(storagePort)->AddAETitle(storageAET)->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(m_TempDirectory);
//	m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&m_Builder);
//	connect(m_StoreSCPLauncher, SIGNAL(SignalStatusOfStoreSCP(const QString&)), this, SLOT(OnStoreSCPStatusChanged(const QString&)));
//	connect(m_StoreSCPLauncher, SIGNAL(SignalStartImport(const QStringList&)), m_Controls.internalDataWidget, SLOT(OnStartDicomImport(const QStringList&)));
//	connect(m_StoreSCPLauncher, SIGNAL(SignalStoreSCPError(const QString&)), m_DicomDirectoryListener, SLOT(OnDicomNetworkError(const QString&)), Qt::DirectConnection);
//	connect(m_StoreSCPLauncher, SIGNAL(SignalStoreSCPError(const QString&)), this, SLOT(OnDicomNetworkError(const QString&)), Qt::DirectConnection);
//	m_StoreSCPLauncher->StartStoreSCP();
//}
//
//void DicomModule::OnStoreSCPStatusChanged(const QString& status)
//{
//	m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-idle_16.png'> " + status);
//}
//
//void DicomModule::OnDicomNetworkError(const QString& status)
//{
//	m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-error_16.png'> " + status);
//}
//
//void DicomModule::StopStoreSCP()
//{
//	delete m_StoreSCPLauncher;
//}

//void DicomModule::SetPluginDirectory()
//{
//	m_PluginDirectory = mitk::PluginActivator::getContext()->getDataFile("").absolutePath();
//	m_PluginDirectory.append("/database");
//}
//
//void DicomModule::CreateTemporaryDirectory()
//{
//	QDir tmp;
//	QString tmpPath = QDir::tempPath();
//	m_TempDirectory.clear();
//	m_TempDirectory.append(tmpPath);
//	m_TempDirectory.append(QString("/"));
//	m_TempDirectory.append(TEMP_DICOM_FOLDER_SUFFIX);
//	m_TempDirectory.append(QString("."));
//	m_TempDirectory.append(QTime::currentTime().toString("hhmmsszzz"));
//	m_TempDirectory.append(QString::number(QCoreApplication::applicationPid()));
//	tmp.mkdir(QDir::toNativeSeparators(m_TempDirectory));
//}

//void DicomModule::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
//{
//	SetPluginDirectory();
//	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
//	m_DatabaseDirectory = prefService->GetSystemPreferences()->Node("/org.mitk.views.dicomreader")->Get("default dicom path", m_PluginDirectory);
//	m_Controls.internalDataWidget->SetDatabaseDirectory(m_DatabaseDirectory);
//}
