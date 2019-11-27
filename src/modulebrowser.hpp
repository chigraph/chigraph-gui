#pragma once

#ifndef CHIGRAPHGUI_MODULE_BROWSER_HPP
#define CHIGRAPHGUI_MODULE_BROWSER_HPP

#include <KLocalizedString>
#include <QTreeView>

class MainWindow;

#include <chi/Fwd.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "toolview.hpp"

struct WorkspaceTree;
class ModuleTreeModel;

class ModuleBrowser : public QTreeView, public ToolView {
	Q_OBJECT

	// ToolView interface
public:
	QWidget*           toolView() override { return this; }
	Qt::DockWidgetArea defaultArea() const override { return Qt::LeftDockWidgetArea; }
	QString            label() override { return i18n("Module Browser"); }
	QString            dockObjectName() override { return QStringLiteral("module-browser"); }

	chi::Context& context() const { return *mContext; }

public:
	ModuleBrowser(QWidget* parent = nullptr);
	~ModuleBrowser();

	std::unordered_set<chi::GraphModule*> dirtyModules();

	ModuleTreeModel* model() { return mModel.get(); }

signals:
	void functionSelected(chi::GraphFunction& func);
	void structSelected(chi::GraphStruct& str);

	void functionRenamed(chi::GraphFunction& func, const std::string& oldName,
	                     const std::vector<chi::NodeInstance*> updatedNodes);
	void structRenamed(chi::GraphStruct& str, const std::string& oldName,
	                   const std::vector<chi::NodeInstance*> updatedNodes);

	void functionDeleted(chi::GraphModule& mod, const std::string& funcName);
	void structDeleted(chi::GraphModule& mod, const std::string& strName);

public slots:
	void loadWorkspace(chi::Context& context);
	void moduleDirtied(chi::GraphModule& dirtied);
	void moduleSaved(chi::GraphModule& saved);

private slots:
	void newModule();
	void newFunction();
	void newStruct();
	void moduleProperties();
	void deleteItem();

private:
	void                                   updateDirtyStatus(chi::GraphModule& updated, bool dirty);
	std::pair<WorkspaceTree*, QModelIndex> idxFromModuleName(const std::filesystem::path& name);

	chi::Context* mContext = nullptr;

	std::unordered_set<chi::GraphModule*> mDirtyModules;

	WorkspaceTree*                   mTree = nullptr;
	std::unique_ptr<ModuleTreeModel> mModel;

	QAction* newModuleAction;
	QAction* newFunctionAction;
	QAction* newStructAction;
	QAction* deleteAction;
	QAction* renameAction;
	QAction* modulePropertiesAction;
};

#endif  // CHIGRAPHGUI_MODULE_BROWSER_HPP
