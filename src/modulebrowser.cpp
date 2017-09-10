#include "modulebrowser.hpp"

#include <chi/GraphStruct.hpp>
#include <chi/Support/Result.hpp>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItem>

#include <KActionCollection>

#include "mainwindow.hpp"
#include "moduletreemodel.hpp"
#include "newmoduledialog.hpp"
#include "modulepropertiesdialog.hpp"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

ModuleBrowser::ModuleBrowser(QWidget* parent) : QTreeView(parent) {
	setXMLFile("chigraphmodulebrowserui.rc");

	// setup actions
	newModuleAction = actionCollection()->addAction(
	    QStringLiteral("new-module"),
	    new QAction(QIcon::fromTheme(QStringLiteral("package-new")), i18n("New Module"), nullptr));
	connect(newModuleAction, &QAction::triggered, this, &ModuleBrowser::newModule);

	newFunctionAction = actionCollection()->addAction(
	    QStringLiteral("new-function"), new QAction(QIcon::fromTheme(QStringLiteral("message-new")),
	                                                i18n("New Function"), nullptr));
	connect(newFunctionAction, &QAction::triggered, this, &ModuleBrowser::newFunction);

	newStructAction = actionCollection()->addAction(
	    QStringLiteral("new-struct"),
	    new QAction(QIcon::fromTheme(QStringLiteral("window-new")), i18n("New Struct"), nullptr));
	
	connect(newStructAction, &QAction::triggered, this, &ModuleBrowser::newStruct);

	deleteAction = actionCollection()->addAction(
	    QStringLiteral("remove-item"),
	    new QAction(QIcon::fromTheme(QStringLiteral("entry-delete")), i18n("Delete"), nullptr));

	renameAction = actionCollection()->addAction(
	    QStringLiteral("rename"),
	    new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"), nullptr));

	modulePropertiesAction = actionCollection()->addAction(
		QStringLiteral("module-properties"),
		new QAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18n("Module Properties"), nullptr));
	connect(modulePropertiesAction, &QAction::triggered, this, &ModuleBrowser::moduleProperties);
	
	setAnimated(true);
	setSortingEnabled(true);
	setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
	header()->close();
	connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		switch (item->type) {
		case WorkspaceTree::FUNCTION: functionSelected(*item->func); return;
		case WorkspaceTree::STRUCT: structSelected(*item->str); return;
		default: return;
		}
	});
	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint p) {
		auto idx = indexAt(p);
		if (!idx.isValid()) { return; }

		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());

		if (!item) { return; }

		setCurrentIndex(idx);

		QMenu contextMenu;

		// add actions depending on the type
		switch (item->type) {
		case WorkspaceTree::MODULE:
			contextMenu.addAction(renameAction);
			contextMenu.addAction(newFunctionAction);
			contextMenu.addAction(newStructAction);
			contextMenu.addAction(deleteAction);
			contextMenu.addAction(modulePropertiesAction);
			break;
		case WorkspaceTree::FOLDER:
			contextMenu.addAction(renameAction);
			contextMenu.addAction(newModuleAction);
			// contextMenu.addAction(deleteAction); TODO: do this
			break;
		case WorkspaceTree::FUNCTION:
			contextMenu.addAction(renameAction);
			contextMenu.addAction(deleteAction);
			break;
		case WorkspaceTree::STRUCT:
			contextMenu.addAction(renameAction);
			contextMenu.addAction(deleteAction);
			break;
		}
		contextMenu.exec(mapToGlobal(p));

	});
}

ModuleBrowser::~ModuleBrowser() = default;

std::unordered_set<chi::GraphModule*> ModuleBrowser::dirtyModules() { return mDirtyModules; }

void ModuleBrowser::loadWorkspace(chi::Context& context) {
	mContext = &context;

	mModel = ModuleTreeModel::createFromContext(context);
	mTree  = mModel->tree();
	
	connect(mModel.get(), &ModuleTreeModel::functionRenamed, this, &ModuleBrowser::functionRenamed);
	connect(mModel.get(), &ModuleTreeModel::structRenamed, this, &ModuleBrowser::structRenamed);

	setModel(mModel.get());
	
	// expand src
	expand(mModel->index(0, 0, {}));
}

void ModuleBrowser::moduleDirtied(chi::GraphModule& dirtied) {
	mDirtyModules.insert(&dirtied);
	updateDirtyStatus(dirtied, true);
}

void ModuleBrowser::moduleSaved(chi::GraphModule& saved) {
	updateDirtyStatus(saved, false);
	mDirtyModules.erase(&saved);
}

void ModuleBrowser::newModule() {
	fs::path startingDir;

	// get the currently selected folder
	auto idx = currentIndex();
	if (idx.isValid()) {
		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());

		startingDir = item->fullName();
	}

	// create a new module dialog
	auto dialog = new NewModuleDialog(this, context(), startingDir);
	dialog->exec();
}

void ModuleBrowser::newFunction() {
	fs::path owningModule;
	
	auto idx = currentIndex();
	if (idx.isValid()) {
		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());
		
		if (item->type != WorkspaceTree::MODULE) {
			return;
		}
		
		owningModule = item->fullName();
	}
	
	// get the module from the context
	chi::ChiModule* mod;
	auto res = context().loadModule(owningModule, &mod);
	
	if (!res) {
		KMessageBox::detailedError(this, "Failed to load module", QString::fromStdString(res.dump()));
		return;
	}
	
	chi::GraphModule* gMod = static_cast<chi::GraphModule*>(mod);
	// add the function TODO: detect if "New Function" is already taken
	LLVM_ATTRIBUTE_UNUSED auto func = gMod->getOrCreateFunction("New Function", {}, {}, {""}, {""});
	Q_ASSERT(func);
	
	
	// update the model
	mModel->updateModule(owningModule);
	
	
}

void ModuleBrowser::newStruct() {
	fs::path owningModule;
	
	auto idx = currentIndex();
	if (idx.isValid()) {
		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());
		
		if (item->type != WorkspaceTree::MODULE) {
			return;
		}
		
		owningModule = item->fullName();
	}
	
	// get the module from the context
	chi::ChiModule* mod;
	auto res = context().loadModule(owningModule, &mod);
	
	if (!res) {
		KMessageBox::detailedError(this, "Failed to load module", QString::fromStdString(res.dump()));
		return;
	}
	
	auto gMod = static_cast<chi::GraphModule*>(mod);
	
	// TODO: rename this to something that's garunteed to not exist
	gMod->getOrCreateStruct("New Struct");
	
	mModel->updateModule(owningModule);
}

void ModuleBrowser::moduleProperties() {
	fs::path owningModule;
	
	auto idx = currentIndex();
	if (idx.isValid()) {
		auto item = static_cast<WorkspaceTree*>(idx.internalPointer());
		
		if (item->type != WorkspaceTree::MODULE) {
			return;
		}
		
		owningModule = item->fullName();
	}
	
	// load the module
	chi::ChiModule* mod;
	auto res = context().loadModule(owningModule, &mod);
	if (!res) { qDebug() << "Failed to load module " << QString::fromStdString(owningModule.string()); }
	
	auto castedMod = static_cast<chi::GraphModule*>(mod);
	
	
	auto dialog = new ModulePropertiesDialog(this, *castedMod);
	dialog->exec();
	
}

void ModuleBrowser::updateDirtyStatus(chi::GraphModule& updated, bool dirty) {
	// find the item
	WorkspaceTree* item;
	QModelIndex    idx;  // get the idx of it so we can emit dataChanged
	std::tie(item, idx) = idxFromModuleName(updated.fullName());

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
