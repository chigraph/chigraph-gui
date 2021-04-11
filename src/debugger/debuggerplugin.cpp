#include "debuggerplugin.hpp"

#include <lldb/API/SBThread.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QToolTip>
#include <chi/NodeInstance.hpp>
#include <chi/NodeType.hpp>
#include <chi/Support/Result.hpp>
#include <thread>

#include "../centraltabview.hpp"
#include "../chigraphnodemodel.hpp"
#include "../mainwindow.hpp"
#include "currentnodedecorator.hpp"

DebuggerPlugin::DebuggerPlugin() {
	Q_INIT_RESOURCE(chigraphdebugger);

	qRegisterMetaType<lldb::SBEvent>("lldb::SBEvent");

	debugAction = actionCollection()->addAction(
	    QStringLiteral("start-debug"),
	    new QAction(QIcon::fromTheme(QStringLiteral("debug-run")), i18n("Debug"), nullptr));
	actionCollection()->setDefaultShortcut(debugAction, Qt::Key_F8);
	connect(debugAction, &QAction::triggered, this, &DebuggerPlugin::debugStart);

	toggleBreakpointAction =
	    actionCollection()->addAction(QStringLiteral("toggle-breakpoint"),
	                                  new QAction(QIcon::fromTheme(QStringLiteral("draw-donut")),
	                                              i18n("Toggle Breakpoint"), nullptr));
	actionCollection()->setDefaultShortcut(toggleBreakpointAction, Qt::Key_F9);
	connect(toggleBreakpointAction, &QAction::triggered, this, &DebuggerPlugin::toggleBreakpoint);

	stepAction = actionCollection()->addAction(
	    QStringLiteral("debug-step"),
	    new QAction(QIcon::fromTheme(QStringLiteral("debug-step-over")), i18n("Step Over"),
	                nullptr));
	actionCollection()->setDefaultShortcut(stepAction, Qt::Key_F10);

	stepOutAction = actionCollection()->addAction(
	    QStringLiteral("debug-step-out"),
	    new QAction(QIcon::fromTheme(QStringLiteral("debug-step-out")), i18n("Step Out"), nullptr));
	actionCollection()->setDefaultShortcut(stepOutAction, Qt::Key_F12);

	stepInAction = actionCollection()->addAction(
	    QStringLiteral("debug-step-into"),
	    new QAction(QIcon::fromTheme(QStringLiteral("debug-step-into")), i18n("Step Into"),
	                nullptr));
	actionCollection()->setDefaultShortcut(stepInAction, Qt::Key_F11);

	continueAction = actionCollection()->addAction(
	    QStringLiteral("debug-continue"),
	    new QAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), i18n("Continue"),
	                nullptr));
	actionCollection()->setDefaultShortcut(continueAction, Qt::Key_F5);
	connect(continueAction, &QAction::triggered, this, &DebuggerPlugin::continueDebugging);

	mBreakpointView = new BreakpointView();
	mVariableView   = new VariableView();

	connect(&MainWindow::instance()->tabView(), &CentralTabView::functionViewChanged, this,
	        [this](FunctionView* view, bool isNew) {
		        // only connect to it if it was just opened
		        if (!isNew) { return; }

		        connect(&view->model(), &ChigraphFlowSceneModel::connectionWasHovered, this,
		                [this](QtNodes::NodeIndex const& lhs, QtNodes::PortIndex lPortIndex,
		                       QtNodes::NodeIndex const& rhs, QtNodes::PortIndex /*rPortIndex*/,
		                       QPoint const&             pos, bool /*entered*/) {
			                // only print value when it's stopped
			                if (!stopped()) { return; }

			                auto leftChiNode =
			                    reinterpret_cast<chi::NodeInstance*>(lhs.internalPointer());

			                Q_ASSERT(lPortIndex >= 0);

			                // if it's an exec output, then return
			                if (!leftChiNode->type().pure() &&
			                    (size_t)lPortIndex <= leftChiNode->outputExecConnections.size()) {
				                return;
			                }

			                auto dataConnID =
			                    leftChiNode->type().pure()
			                        ? lPortIndex
			                        : lPortIndex - leftChiNode->outputExecConnections.size();

			                // get the LLDB value
			                auto value = mDebugger->inspectNodeOutput(*leftChiNode, dataConnID);
			                if (!value.IsValid()) { return; }

			                // display it
			                QToolTip::showText(pos, QString::fromLatin1(value.GetValue()) + " : " +
			                                            QString::fromLatin1(value.GetSummary()));
		                });
	        });

	setXMLFile("chigraphguidebuggerui.rc");
}

void DebuggerPlugin::toggleBreakpoint() {
	auto currentView = MainWindow::instance()->tabView().currentView();

	if (currentView == nullptr) { return; }

	for (auto node : currentView->selectedNodes()) { mBreakpointView->addBreakpoint(*node); }
}

void DebuggerPlugin::debugStart() {
	std::filesystem::path chiPath =
	    std::filesystem::path(QApplication::applicationFilePath().toStdString()).parent_path() /
	    "chi";
#ifdef _WIN32
	chiPath.replace_extension(".exe");
#endif

	MainWindow* window = MainWindow::instance();

	mDebugger = nullptr;

	// delete it
	if (mThread) { mThread->wait(); }
	mEventListener = nullptr;

	auto currentConfig = window->launchManager().currentConfiguration();
	if (!currentConfig.valid()) { return; }

	auto pair = window->loadModule(currentConfig.module());

	if (!pair.first || !pair.second) {
		KMessageBox::detailedError(window, i18n("Failed to load module"),
		                           QString::fromStdString(pair.first.dump()), i18n("run: error"));
		return;
	}

	// TODO: this really really needs a fix
	mDebugger = std::make_shared<chi::Debugger>(chiPath.string().c_str(), *pair.second);

	mThread        = new QThread;
	mEventListener = std::make_unique<DebuggerWorkerThread>(mDebugger);
	mEventListener->moveToThread(mThread);

	connect(mThread, &QThread::started, mEventListener.get(), &DebuggerWorkerThread::process);
	connect(mEventListener.get(), &DebuggerWorkerThread::eventOccured, this,
	        [this, window](lldb::SBEvent ev) {
		        if (lldb::SBProcess::GetStateFromEvent(ev) == lldb::eStateStopped) {
			        variableView().setDisabled(false);
			        mStopped = true;
			        variableView().setFrame(
			            mDebugger->lldbProcess().GetSelectedThread().GetSelectedFrame());

			        // get the node
			        auto node = mDebugger->nodeFromFrame(
			            mDebugger->lldbProcess().GetSelectedThread().GetSelectedFrame());
			        if (node == nullptr) { return; }

			        window->tabView().centerOnNode(*node);

		        } else {
			        variableView().setDisabled(true);
			        mStopped = false;
		        }
	        });

	mThread->start();

	// set breakpoints
	for (const auto& bp : breakpointView().breakpoints()) { mDebugger->setBreakpoint(*bp.first); }

	auto res = mDebugger->start();
	if (!res) {
		qDebug() << QString::fromStdString(res.dump());
		return;
	}
}

void DebuggerPlugin::continueDebugging() {
	if (mDebugger != nullptr) { mDebugger->processContinue(); }
}
