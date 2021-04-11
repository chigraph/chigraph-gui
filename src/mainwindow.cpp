#include "mainwindow.hpp"

#include <KActionCollection>
#include <KActionMenu>
#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardAction>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPlainTextEdit>
#include <QPluginLoader>
#include <QProcess>
#include <QScrollArea>
#include <QSplitter>
#include <QTextStream>
#include <chi/Context.hpp>
#include <chi/DataType.hpp>
#include <chi/GraphFunction.hpp>
#include <chi/GraphModule.hpp>
#include <chi/LangModule.hpp>
#include <chi/NodeInstance.hpp>
#include <chi/Support/Result.hpp>
#include <chi/Support/json.hpp>
#include <fstream>

#include "centraltabview.hpp"
#include "chigraphnodemodel.hpp"
#include "chigraphplugin.hpp"
#include "functiondetails.hpp"
#include "functionview.hpp"
#include "launchconfigurationdialog.hpp"
#include "localvariables.hpp"
#include "modulebrowser.hpp"
#include "moduletreemodel.hpp"
#include "structedit.hpp"
#include "subprocessoutputview.hpp"
#include "thememanager.hpp"

MainWindow* MainWindow::mInstance = nullptr;

MainWindow::MainWindow(QWidget* parent) : KXmlGuiWindow(parent) {
	Q_INIT_RESOURCE(chigraphgui);

	mInstance = this;

	// set icon
	setWindowIcon(QIcon(":/icons/chigraphsmall.png"));

	mChigContext   = std::make_unique<chi::Context>(qApp->arguments()[0].toStdString().c_str());
	mLaunchManager = std::make_unique<LaunchConfigurationManager>(*mChigContext);

	mFunctionTabs = new CentralTabView;
	mFunctionTabs->setMovable(true);
	mFunctionTabs->setTabsClosable(true);
	connect(mFunctionTabs, &CentralTabView::dirtied, this, &MainWindow::moduleDirtied);
	connect(this, &MainWindow::workspaceOpened, mFunctionTabs, [this](chi::Context&) {
		while (auto v = mFunctionTabs->currentView()) { mFunctionTabs->closeView(v); }
	});
	insertChildClient(mFunctionTabs);

	setCentralWidget(mFunctionTabs);

	// setup module browser
	mModuleBrowser = new ModuleBrowser(this);
	auto docker    = new QDockWidget(mModuleBrowser->label(), this);
	docker->setObjectName(mModuleBrowser->dockObjectName());
	insertChildClient(mModuleBrowser);
	docker->setWidget(mModuleBrowser);
	addDockWidget(mModuleBrowser->defaultArea(), docker);
	connect(this, &MainWindow::workspaceOpened, mModuleBrowser, &ModuleBrowser::loadWorkspace);
	connect(mModuleBrowser, &ModuleBrowser::functionSelected, &tabView(),
	        &CentralTabView::selectNewFunction);
	connect(mModuleBrowser, &ModuleBrowser::structSelected, &tabView(),
	        &CentralTabView::selectNewStruct);
	connect(this, &MainWindow::newModuleCreated, mModuleBrowser,
	        [this](chi::GraphModule& mod) { mModuleBrowser->loadWorkspace(mod.context()); });
	connect(this, &MainWindow::workspaceOpened, docker, [docker](chi::Context& ctx) {
		docker->setWindowTitle(i18n("Modules") + " - " +
		                       QString::fromStdString(ctx.workspacePath().string()));
	});
	connect(mModuleBrowser, &ModuleBrowser::functionRenamed, mFunctionTabs,
	        &CentralTabView::functionRenamed);
	connect(mModuleBrowser, &ModuleBrowser::structRenamed, mFunctionTabs,
	        &CentralTabView::structRenamed);
	connect(mModuleBrowser, &ModuleBrowser::functionDeleted, mFunctionTabs,
	        &CentralTabView::functionDeleted);
	connect(mModuleBrowser, &ModuleBrowser::structDeleted, mFunctionTabs,
	        &CentralTabView::structDeleted);

	docker = new QDockWidget(i18n("Output"), this);
	docker->setObjectName("Output");
	auto outputView = new QTabWidget();
	outputView->setTabsClosable(true);
	docker->setWidget(outputView);
	addDockWidget(Qt::BottomDockWidgetArea, docker);
	connect(outputView, &QTabWidget::tabCloseRequested, outputView, &QTabWidget::removeTab);

	docker               = new QDockWidget(i18n("Function Details"), this);
	auto functionDetails = new FunctionDetails;
	docker->setObjectName(functionDetails->dockObjectName());
	insertChildClient(functionDetails);
	auto scroll = new QScrollArea;
	scroll->setWidget(functionDetails);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	docker->setWidget(scroll);
	addDockWidget(Qt::RightDockWidgetArea, docker);
	connect(mFunctionTabs, &CentralTabView::functionViewChanged, functionDetails,
	        [docker, functionDetails](FunctionView* view, bool) {
		        functionDetails->setEnabled(true);
		        functionDetails->loadFunction(view);
		        docker->setWindowTitle(i18n("Function Details") + " - " +
		                               QString::fromStdString(view->function()->name()));
	        });
	connect(mFunctionTabs, &CentralTabView::structViewChanged, functionDetails,
	        [docker, functionDetails](StructEdit*, bool) {
		        functionDetails->setEnabled(false);

		        docker->setWindowTitle(i18n("Function Details"));
	        });
	connect(functionDetails, &FunctionDetails::dirtied, this,
	        [this, functionDetails] { moduleDirtied(functionDetails->chiFunc()->module()); });

	/// load plugins
	for (QObject* plugin : QPluginLoader::staticInstances()) {
		if (auto chiPlugin = qobject_cast<ChigraphPlugin*>(plugin)) {
			insertChildClient(chiPlugin);

			for (auto view : chiPlugin->toolViews()) {
				docker = new QDockWidget(view->label(), this);
				docker->setObjectName(view->dockObjectName());
				docker->setWidget(view->toolView());

				insertChildClient(view);

				addDockWidget(view->defaultArea(), docker);
			}
		}
	}

	/// Setup actions
	auto actColl = actionCollection();

	KStandardAction::quit(qApp, SLOT(quit()), actColl);

	QAction* openAction = actColl->addAction(KStandardAction::Open, QStringLiteral("open"));
	openAction->setWhatsThis(QStringLiteral("Open a chigraph workspace"));
	connect(openAction, &QAction::triggered, this, &MainWindow::openWorkspaceDialog);

	mOpenRecentAction = KStandardAction::openRecent(this, &MainWindow::openWorkspace, nullptr);
	actColl->addAction(QStringLiteral("open-recent"), mOpenRecentAction);

	mOpenRecentAction->setToolBarMode(KSelectAction::MenuMode);
	mOpenRecentAction->setToolButtonPopupMode(QToolButton::DelayedPopup);
	mOpenRecentAction->setIconText(i18nc("action, to open an archive", "Open"));
	mOpenRecentAction->setToolTip(i18n("Open an archive"));
	mOpenRecentAction->loadEntries(KSharedConfig::openConfig()->group("Recent Files"));

	auto newAction = actColl->addAction(KStandardAction::New, QStringLiteral("new"));
	newAction->setWhatsThis(i18n("Create a new chigraph workspace"));
	connect(newAction, &QAction::triggered, this, &MainWindow::newWorkspace);

	auto saveAction = actColl->addAction(KStandardAction::Save, QStringLiteral("save"));
	saveAction->setWhatsThis(i18n("Save the chigraph module"));
	connect(saveAction, &QAction::triggered, this, &MainWindow::save);

	auto cancelAction = new QAction(nullptr);
	cancelAction->setEnabled(false);
	cancelAction->setText(i18n("Cancel"));
	cancelAction->setIcon(QIcon::fromTheme("process-stop"));
	actColl->setDefaultShortcut(cancelAction, Qt::CTRL + Qt::Key_Q);
	actColl->addAction(QStringLiteral("cancel"), cancelAction);
	connect(cancelAction, &QAction::triggered, this, [outputView] {
		auto output = dynamic_cast<SubprocessOutputView*>(outputView->currentWidget());

		if (output != nullptr) { output->cancelProcess(); }
	});

	connect(outputView, &QTabWidget::currentChanged, this, [cancelAction, outputView](int) {
		auto view = dynamic_cast<SubprocessOutputView*>(outputView->currentWidget());

		if (view != nullptr) {
			cancelAction->setEnabled(view->running());
		} else {
			cancelAction->setEnabled(false);
		}
	});

	auto runAction = new QAction(nullptr);
	runAction->setText(i18n("&Run"));
	runAction->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
	actColl->setDefaultShortcut(runAction, Qt::CTRL + Qt::Key_R);
	actColl->addAction(QStringLiteral("run"), runAction);

	connect(runAction, &QAction::triggered, this, [this, outputView, cancelAction] {
		if (!launchManager().currentConfiguration().valid()) {
			KMessageBox::error(this, i18n("Select a launch configuration"), i18n("run: error"));
			return;
		}

		auto moduleName = launchManager().currentConfiguration().module();

		if (moduleName.isEmpty()) {
			KMessageBox::error(this, i18n("No module set in launch configuration: ") +
			                             launchManager().currentConfiguration().name());
			return;
		}

		// get the module
		chi::Result       res;
		chi::GraphModule* mod;
		std::tie(res, mod) = loadModule(moduleName);

		if (!res || !mod) {
			KMessageBox::detailedError(this, i18n("Failed to load module"),
			                           QString::fromStdString(res.dump()), i18n("run: error"));
			return;
		}

		auto view = new SubprocessOutputView(mod);
		connect(view, &SubprocessOutputView::processFinished, this,
		        [outputView, view, cancelAction](int exitCode, QProcess::ExitStatus exitStatus) {
			        QString statusStr =
			            QString(" (%1, %2)")
			                .arg(exitStatus == QProcess::NormalExit ? "exited" : "crashed",
			                     QString::number(exitCode));
			        outputView->setTabText(
			            outputView->indexOf(view),
			            QString::fromStdString(view->module()->fullName()) + statusStr);

			        // disable it
			        if (outputView->currentWidget() == view) { cancelAction->setEnabled(false); }
		        });
		// add the tab to the beginning
		int newTabID = outputView->insertTab(
		    0, view, QString::fromStdString(mod->fullName()) + i18n(" (running)"));
		outputView->setCurrentIndex(newTabID);
		cancelAction->setEnabled(true);
	});

	auto runConfigDialogAction = new QAction(this);
	runConfigDialogAction->setText(i18n("Configure Launches"));
	runConfigDialogAction->setIcon(QIcon::fromTheme(QStringLiteral("run-build-configure")));
	actColl->addAction(QStringLiteral("configure-launches"), runConfigDialogAction);
	connect(runConfigDialogAction, &QAction::triggered, this, [this] {
		LaunchConfigurationDialog d(launchManager());

		d.exec();

		updateUsableConfigs();
	});

	mConfigSelectAction = new KSelectAction(QIcon::fromTheme(QStringLiteral("run-build-configure")),
	                                        i18n("Launch Configuration"), this);
	actColl->addAction(QStringLiteral("select-launch-configuration"), mConfigSelectAction);
	connect(mConfigSelectAction,
			&KSelectAction::textTriggered, this,
	        [this](const QString& str) {
		        launchManager().setCurrentConfiguration(launchManager().configByName(str));
	        });

	if (launchManager().currentConfiguration().valid()) {
		mConfigSelectAction->setCurrentAction(launchManager().currentConfiguration().name(),
		                                      Qt::CaseSensitive);
	}

	// theme selector
	auto themeAction = new KActionMenu(i18n("Theme"), this);
	mThemeManager    = std::make_unique<ThemeManager>(themeAction);
	actColl->addAction(QStringLiteral("theme"), themeAction);

	setupGUI(Default, "chigraphguiui.rc");
}

MainWindow::~MainWindow() {
	mOpenRecentAction->saveEntries(KSharedConfig::openConfig()->group("Recent Files"));
}

std::pair<chi::Result, chi::GraphModule*> MainWindow::loadModule(const QString& name) {
	assert(!name.isEmpty() && "Name passed to loadModule should not be empty");

	chi::ChiModule* mod;
	auto            res = context().loadModule(name.toStdString(), &mod);
	if (!res) { return {res, nullptr}; }
	return {res, dynamic_cast<chi::GraphModule*>(mod)};
}

void MainWindow::newWorkspace() {
	// open a dialog
	auto newWorkspaceDir = QFileDialog::getExistingDirectory(this, i18n("New Chigraph Workspace"),
	                                                         QDir::homePath(), {});

	if (newWorkspaceDir.isEmpty()) { return; }

	// create the file
	QFile f{newWorkspaceDir + "/.chigraphworkspace"};
	f.open(QIODevice::ReadWrite);

	auto url = QUrl::fromLocalFile(newWorkspaceDir);
	mOpenRecentAction->addUrl(url);
	openWorkspace(url);
}

void MainWindow::save() {
	auto currentFunc = tabView().currentView();
	if (!currentFunc) { return; }
	auto module = &currentFunc->function()->module();
	if (module != nullptr) {
		chi::Result res = module->saveToDisk();
		if (!res) {
			KMessageBox::detailedError(this, i18n("Failed to save module!"),
			                           QString::fromStdString(res.dump()));
		}
		mModuleBrowser->moduleSaved(*module);
	}
}

void MainWindow::openWorkspaceDialog() {
	QString workspace =
	    QFileDialog::getExistingDirectory(this, i18n("Chigraph Workspace"), QDir::homePath(),
	                                      QFileDialog::ShowDirsOnly
	// this is needed for flatapk so it doens't use the portal
#ifdef CHI_FOR_FLATPAK
	                                          | QFileDialog::DontUseNativeDialog
#endif
	    );

	if (workspace.isEmpty()) { return; }

	QUrl url = QUrl::fromLocalFile(workspace);

	mOpenRecentAction->addUrl(url);

	openWorkspace(url);
}

void MainWindow::openWorkspace(const QUrl& url) {
	auto ctx = std::make_unique<chi::Context>(url.toLocalFile().toStdString());

	// make sure it actually was a context
	if (!ctx->hasWorkspace()) {
		KMessageBox::error(this,
		                   i18n("Path ") + url.toLocalFile() + i18n(" wasn't a chigraph workspace"),
		                   i18n("Error loading workspace"));

		return;
	}

	mChigContext   = std::move(ctx);
	mLaunchManager = std::make_unique<LaunchConfigurationManager>(*mChigContext);
	updateUsableConfigs();

	emit workspaceOpened(*mChigContext);
}

void MainWindow::moduleDirtied(chi::GraphModule& mod) { mModuleBrowser->moduleDirtied(mod); }

void MainWindow::updateUsableConfigs() {
	// repopulate
	mConfigSelectAction->clear();

	for (const auto& config : launchManager().configurations()) {
		mConfigSelectAction->addAction(config.name());
	}

	auto currentConfig = launchManager().currentConfiguration();

	if (currentConfig.valid()) {
		// set it to the current config
		mConfigSelectAction->setCurrentAction(currentConfig.name(), Qt::CaseSensitive);
	}
}

void MainWindow::closeEvent(QCloseEvent* event) {
	// check through dirty modules
	if (mModuleBrowser->dirtyModules().empty()) {
		KXmlGuiWindow::closeEvent(event);
		return;
	}

	// see if they want to save them
	for (auto mod : mModuleBrowser->dirtyModules()) {
		auto bc = KMessageBox::questionYesNoCancel(
		    this,
		    i18n("Module <b>") + QString::fromStdString(mod->fullName()) +
		        i18n("</b> has unsaved changes. Do you want to save your changes or discard them?"),
		    i18n("Close"), KStandardGuiItem::save(), KStandardGuiItem::discard());

		switch (bc) {
		case KMessageBox::Yes:
			// this means save
			save();
			break;
		case KMessageBox::No:
			// this means discard
			// TODO: implement discard
			break;
		case KMessageBox::Cancel: event->ignore(); return;
		default: assert(false);
		}
	}

	KXmlGuiWindow::closeEvent(event);
}
