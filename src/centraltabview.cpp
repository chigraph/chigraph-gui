#include "centraltabview.hpp"

#include <KActionCollection>

#include <QDebug>

#include <chi/Context.hpp>
#include <chi/GraphFunction.hpp>
#include <chi/GraphStruct.hpp>
#include <chi/Support/Result.hpp>

#include "functionview.hpp"
#include "structedit.hpp"

CentralTabView::CentralTabView(QWidget* owner) : QTabWidget{owner} {
	auto closeAction =
	    actionCollection()->addAction(KStandardAction::Close, QStringLiteral("close-function"));
	connect(closeAction, &QAction::triggered, this, [this] { closeTab(currentIndex()); });

	connect(this, &QTabWidget::tabCloseRequested, this, &CentralTabView::closeTab);

	connect(this, &QTabWidget::currentChanged, this, [this](int idx) {
		// see if it's a function or struct
		auto wid = widget(idx);

		if (auto func = qobject_cast<FunctionView*>(wid)) {
			emit functionViewChanged(func, false);
			return;
		}
		if (auto str = qobject_cast<StructEdit*>(wid)) { emit structViewChanged(str, false); }
	});

	setXMLFile("chigraphfunctiontabviewui.rc");
}

void CentralTabView::selectNewFunction(chi::GraphFunction& func) {
	QString qualifiedFunctionName =
	    QString::fromStdString(func.module().fullName() + ":" + func.name());

	// see if it's already open
	auto funcViewIter = mOpenFunctions.find(qualifiedFunctionName);
	if (funcViewIter != mOpenFunctions.end()) {
		setCurrentWidget(funcViewIter->second);
		functionViewChanged(funcViewIter->second, false);
		return;
	}
	// if it's not already open, we'll have to create our own

	auto view                             = new FunctionView(func, this);
	int  idx                              = addTab(view, qualifiedFunctionName);
	mOpenFunctions[qualifiedFunctionName] = view;
	setTabText(idx, qualifiedFunctionName);
	setTabIcon(idx, QIcon::fromTheme(QStringLiteral("code-context")));
	setCurrentWidget(view);

	connect(view, &FunctionView::dirtied, this, [ this, mod = &func.module() ] { dirtied(*mod); });
	connect(view, &FunctionView::functionDoubleClicked, this, &CentralTabView::selectNewFunction);

	functionViewChanged(view, true);
}

void CentralTabView::selectNewStruct(chi::GraphStruct& str) {
	QString qualifiedStructName =
	    QString::fromStdString(str.module().fullName() + ":" + str.name());

	auto strViewIter = mOpenStructs.find(qualifiedStructName);

	if (strViewIter != mOpenStructs.end()) {
		setCurrentWidget(strViewIter->second);
		structViewChanged(strViewIter->second, false);
		return;
	}

	auto view = new StructEdit(str, this);
	int  idx  = addTab(view, qualifiedStructName);

	mOpenStructs[qualifiedStructName] = view;
	setTabText(idx, qualifiedStructName);
	setTabIcon(idx, QIcon::fromTheme(QStringLiteral("code-class")));

	setCurrentWidget(view);

	// TODO: dirtied?

	structViewChanged(view, true);
}

void CentralTabView::centerOnNode(chi::NodeInstance& inst) {
	// load the function
	selectNewFunction(inst.function());

	// center on the node
	currentView()->centerOnNode(inst);
}

void CentralTabView::selectNode(chi::NodeInstance& inst) {
	auto view = viewFromFunction(inst.function());

	if (!view) { return; }
	view->selectNode(inst);
}

void CentralTabView::refreshModule(chi::GraphModule& mod) {
	int currTabId = currentIndex();

	// close the function views
	std::vector<std::pair<std::string, int>> functionNames;
	for (const auto& pair : mOpenFunctions) {
		if (&pair.second->function()->module() == &mod) {
			auto idx = indexOf(pair.second);
			functionNames.emplace_back(pair.second->function()->name(), idx);
			removeTab(idx);
		}
	}
	std::string fullName = mod.fullName();
	mod.context().unloadModule(fullName);

	chi::ChiModule* cMod;
	mod.context().loadModule(fullName, &cMod);
	chi::GraphModule* gMod = dynamic_cast<chi::GraphModule*>(cMod);

	// re-add the tabs in reverse order to keep the ids
	for (auto iter = functionNames.rbegin(); iter != functionNames.rend(); ++iter) {
		chi::GraphFunction* func = gMod->functionFromName(iter->first);
		QString             qualifiedFunctionName =
		    QString::fromStdString(gMod->fullName() + ":" + func->name());
		auto view = new FunctionView(*func);
		insertTab(iter->second, view, qualifiedFunctionName);
		mOpenFunctions[qualifiedFunctionName] = view;

		connect(view, &FunctionView::dirtied, this,
		        [ this, mod = &func->module() ] { dirtied(*mod); });
		connect(view, &FunctionView::functionDoubleClicked, this,
		        &CentralTabView::selectNewFunction);
	}

	setCurrentIndex(currTabId);
}

FunctionView* CentralTabView::viewFromFunction(chi::GraphFunction& func) {
	return viewFromFunctionName(func.module().fullName(), func.name());
}

FunctionView* CentralTabView::viewFromFunctionName(const QString& fullName) {
	auto iter = mOpenFunctions.find(fullName);
	if (iter != mOpenFunctions.end()) { return iter->second; }
	return nullptr;
}

FunctionView* CentralTabView::viewFromFunctionName(const boost::filesystem::path& mod,
                                                   const std::string&             function) {
	return viewFromFunctionName(QString::fromStdString(mod.string() + ":" + function));
}

void CentralTabView::closeView(FunctionView* view) { closeTab(indexOf(view)); }
void CentralTabView::closeView(StructEdit* view) { closeTab(indexOf(view)); }

FunctionView* CentralTabView::currentView() {
	auto widget = currentWidget();

	auto casted = qobject_cast<FunctionView*>(widget);

	if (casted == nullptr) { return nullptr; }

	return casted;
}

void CentralTabView::functionRenamed(chi::GraphFunction& func, const std::string& oldName,
                                     const std::vector<chi::NodeInstance*>& changed) {
	auto fullOldName = QString::fromStdString(func.module().fullName() + ":" + oldName);
	auto fullName    = QString::fromStdString(func.module().fullName() + ":" + func.name());

	// update the strcuture and rename the tab if it's open
	auto iter = mOpenFunctions.find(fullOldName);
	if (iter != mOpenFunctions.end()) {
		auto view = iter->second;
		mOpenFunctions.erase(iter);

		mOpenFunctions.emplace(fullName, view);

		// change tab text
		auto id = indexOf(view);
		if (id != -1) { setTabText(id, fullName); }
	}

	// refresh the changed nodes
	for (const auto node : changed) {
		auto view = viewFromFunction(node->function());

		if (view) { view->refreshGuiForNode(*node); }
	}
}

void CentralTabView::structRenamed(chi::GraphStruct& str, const std::string& oldName,
                                   const std::vector<chi::NodeInstance*>& changed) {
	auto fullOldName = QString::fromStdString(str.module().fullName() + ":" + oldName);
	auto fullName    = QString::fromStdString(str.module().fullName() + ":" + str.name());

	// update the strcuture and rename the tab if it's open
	auto iter = mOpenStructs.find(fullOldName);
	if (iter != mOpenStructs.end()) {
		auto view = iter->second;
		mOpenStructs.erase(iter);

		mOpenStructs.emplace(fullName, view);

		// change tab text
		auto id = indexOf(view);
		if (id != -1) { setTabText(id, fullName); }
	}

	// refresh the changed nodes
	for (const auto node : changed) {
		auto view = viewFromFunction(node->function());

		if (view) { view->refreshGuiForNode(*node); }
	}
}

void CentralTabView::functionDeleted(chi::GraphModule& mod, const std::string& funcName) {
	auto iter = mOpenFunctions.find(QString::fromStdString(mod.fullName() + ":" + funcName));
	if (iter != mOpenFunctions.end()) {
		auto view = iter->second;
		closeView(view);
	}
}

void CentralTabView::structDeleted(chi::GraphModule& mod, const std::string& strName) {
	auto iter = mOpenStructs.find(QString::fromStdString(mod.fullName() + ":" + strName));
	if (iter != mOpenStructs.end()) {
		auto view = iter->second;
		closeView(view);
	}

	return;
}

void CentralTabView::closeTab(int idx) {
	auto funcIter = std::find_if(mOpenFunctions.begin(), mOpenFunctions.end(),
	                             [this, idx](auto& p) { return p.second == this->widget(idx); });

	if (funcIter != mOpenFunctions.end()) { mOpenFunctions.erase(funcIter); }
	auto strIter = std::find_if(mOpenStructs.begin(), mOpenStructs.end(),
	                            [this, idx](auto& p) { return p.second == this->widget(idx); });
	if (strIter != mOpenStructs.end()) { mOpenStructs.erase(strIter); }
	assert((funcIter != mOpenFunctions.end() || strIter != mOpenStructs.end()) &&
	       "Internal error: a tab index was not in mOpenFunctions or mOpenStructs");

	removeTab(idx);
}
