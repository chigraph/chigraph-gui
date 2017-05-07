#include "modulebrowser.hpp"

#include <chi/GraphStruct.hpp>
#include <chi/Result.hpp>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItem>

#include <KActionCollection>

#include "mainwindow.hpp"
#include "moduletreemodel.hpp"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


ModuleBrowser::ModuleBrowser(QWidget* parent) : QTreeView(parent) {
	setXMLFile("chigraphmodulebrowserui.rc");

	setAnimated(true);
	setSortingEnabled(true);
	header()->close();
	connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		switch (item->type) {
			case WorkspaceTree::FUNCTION:
				functionSelected(*item->func);
				return;
			case WorkspaceTree::STRUCT:
				structSelected(*item->str);
				return;
			default: return;
		}
	});
	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint p) {
		auto idx = indexAt(p);
		if (!idx.isValid()) { return; }

		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());

		if (!item || !item->dirty) { return; }

		setCurrentIndex(idx);

		QMenu contextMenu;
		// TODO: actions
		contextMenu.exec(mapToGlobal(p));
	});
}

ModuleBrowser::~ModuleBrowser() = default;

std::unordered_set<chi::GraphModule*> ModuleBrowser::dirtyModules() { return mDirtyModules; }

void ModuleBrowser::loadWorkspace(chi::Context& context) {
	mContext = &context;
	
	mModel = ModuleTreeModel::createFromContext(context);
	mTree = mModel->tree.get();
	
	setModel(mModel.get());
}

void ModuleBrowser::moduleDirtied(chi::GraphModule& dirtied) {
	mDirtyModules.insert(&dirtied);
	updateDirtyStatus(dirtied, true);
}

void ModuleBrowser::moduleSaved(chi::GraphModule& saved) {
	updateDirtyStatus(saved, false);
	mDirtyModules.erase(&saved);
}

void ModuleBrowser::updateDirtyStatus(chi::GraphModule& updated, bool dirty) {
	// find the item
	WorkspaceTree* item;
	QModelIndex    idx;  // get the idx of it so we can emit dataChanged
	std::tie(item, idx) = idxFromModuleName(updated.fullName());

	// when we save, it reloads the module, invalidating these pointers
	if (!dirty) {
		item->module = nullptr;
		item->children.clear();
		setExpanded(idx, false);
	}

	mModel->dataChanged(idx, idx);

	if (item->name.toStdString() !=
	    fs::path(updated.fullName()).filename().replace_extension("").string()) {
		return;
	}

	item->dirty = dirty;
}

std::pair<WorkspaceTree*, QModelIndex> ModuleBrowser::idxFromModuleName(const fs::path& name) {
	WorkspaceTree* item = mTree;
	QModelIndex    idx;  // get the idx of it so we can emit dataChanged
	for (auto componentIter = name.begin(); componentIter != name.end(); ++componentIter) {
		auto component = *componentIter;

		auto id = 0ull;
		for (const auto& ch : item->children) {
			if (ch->name.toStdString() == component.string() &&
			    (ch->type == WorkspaceTree::MODULE) == (componentIter == --name.end())) {
				item = ch.get();
				idx  = mModel->index(id, 0, idx);
				break;
			}

			++id;
		}
	}

	return {item, idx};
}
