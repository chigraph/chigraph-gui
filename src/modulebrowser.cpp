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

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

struct WorkspaceTree {
	
	enum eType {
		FUNCTION,
		MODULE,
		STRUCT,
		FOLDER
	};
	
	
	WorkspaceTree*                              parent = nullptr;
	std::vector<std::unique_ptr<WorkspaceTree>> children;
	chi::GraphModule*                           module   = nullptr;
	chi::GraphFunction*                         func     = nullptr;
	chi::GraphStruct*                           str      = nullptr;
	QString                                     name;
	bool                                        dirty = false;
	int                                         row   = 0;
	eType type;
	
};

class ModuleTreeModel : public QAbstractItemModel {
public:
	ModuleTreeModel(std::unique_ptr<WorkspaceTree> t, chi::Context& ctx)
	    : tree{std::move(t)}, mCtx{&ctx} {}

	int columnCount(const QModelIndex& parent) const override { return 1; }
	QModelIndex index(int row, int column, const QModelIndex& parent) const override {
		if (!hasIndex(row, column, parent)) { return {}; }

		WorkspaceTree* parentItem;
		if (parent.isValid()) {
			parentItem = static_cast<WorkspaceTree*>(parent.internalPointer());
		} else {
			parentItem = tree.get();
		}

		if (row < parentItem->children.size()) {
			return createIndex(row, column, parentItem->children[row].get());
		}
		return {};
	}
	QModelIndex parent(const QModelIndex& index) const override {
		if (!index.isValid()) { return {}; }

		auto childItem  = static_cast<WorkspaceTree*>(index.internalPointer());
		auto parentItem = childItem->parent;

		if (parentItem == nullptr) return {};

		return createIndex(parentItem->row, 0, parentItem);
	}
	bool hasChildren(const QModelIndex& index) const override {
		if (!index.isValid()) { return true; }

		auto item = static_cast<WorkspaceTree*>(index.internalPointer());
		return item->type == WorkspaceTree::MODULE || item->type == WorkspaceTree::FOLDER;
	}
	bool canFetchMore(const QModelIndex& index) const override {
		if (!index.isValid()) { return false; }

		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		return item->type == WorkspaceTree::MODULE;
	}
	void fetchMore(const QModelIndex& index) override {
		if (!index.isValid()) { return; }

		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		if (item->module != nullptr) {
			// it's already been fetched
			return;
		}

		// get the name
		fs::path p;
		{
			auto parent = item;
			while (parent != nullptr) {
				p      = parent->name.toStdString() / p;
				parent = parent->parent;
			}
			p = p.string().substr(4);
		}

		// load it
		chi::ChiModule* mod;
		chi::Result     res = mCtx->loadModule(p, chi::LoadSettings::Default, &mod);
		if (!res) {
			KMessageBox::detailedError(MainWindow::instance(),
			                           R"(Failed to load JsonModule from file ")" +
			                               QString::fromStdString(p.string()) + R"(")",
			                           QString::fromStdString(res.dump()), "Error Loading");

			return;
		}

		item->module = static_cast<chi::GraphModule*>(mod);

		// add functions
		for (const auto& func : item->module->functions()) {
			auto child    = std::make_unique<WorkspaceTree>();
			child->func   = func.get();
			child->parent = item;
			child->name   = QString::fromStdString(func->name());
			child->row    = item->children.size();
			child->type = WorkspaceTree::FUNCTION;

			item->children.push_back(std::move(child));
		}
		
		// add structs
		for (const auto& str : item->module->structs()) {
			auto child = std::make_unique<WorkspaceTree>();
			child->str = str.get();
			child->name = QString::fromStdString(str->name());
			child->parent = item;
			child->row = item->children.size();
			child->type = WorkspaceTree::STRUCT;
			
			item->children.push_back(std::move(child));
		}
	}
	int rowCount(const QModelIndex& index) const override {
		WorkspaceTree* parentItem;
		if (index.isValid()) {
			parentItem = static_cast<WorkspaceTree*>(index.internalPointer());
		} else {
			parentItem = tree.get();
		}

		return parentItem->children.size();
	}
	QVariant data(const QModelIndex& index, int role) const override {
		if (!index.isValid()) { return {}; }

		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		switch (role) {
		case Qt::DisplayRole:
			if (item->dirty) {
				return "* " + item->name;
			} else {
				return item->name;
			}
		case Qt::DecorationRole:
			switch (item->type) {
				case WorkspaceTree::MODULE: return QIcon::fromTheme(QStringLiteral("package-available"));
				case WorkspaceTree::FUNCTION: return QIcon::fromTheme(QStringLiteral("code-context"));
				case WorkspaceTree::STRUCT: return QIcon::fromTheme(QStringLiteral("code-class"));
				default: return {};
			}
		case Qt::FontRole:
			if (item->dirty || (item->parent != nullptr && item->parent->dirty)) {
				QFont bold;
				bold.setBold(true);
				return bold;
			}
		default: return {};
		}
	}
	std::unique_ptr<WorkspaceTree> tree;
	chi::Context*                  mCtx;
};

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
		
		if (item->func == nullptr) {  // don't do module folders or modules
			return;
		}
		emit functionSelected(*item->func);
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

	auto modules = context.listModulesInWorkspace();

	auto tree = std::make_unique<WorkspaceTree>();
	
	// add src object
	auto srcTree = std::make_unique<WorkspaceTree>();
	srcTree->parent = tree.get();
	srcTree->type = WorkspaceTree::FOLDER;
	srcTree->row = 0;
	srcTree->name = QStringLiteral("src");
	

	// create the tree
	for (const fs::path& mod : modules) {
		WorkspaceTree* parent = srcTree.get();

		// for each component of mod
		for (auto componentIter = mod.begin(); componentIter != mod.end(); ++componentIter) {
			fs::path component = *componentIter;
			bool     isModule  = componentIter == --mod.end();

			// make sure it exists
			bool found = false;
			for (const auto& child : parent->children) {
				if (child->name.toStdString() == component.string() &&
				    (child->type == WorkspaceTree::MODULE) == isModule) {
					found  = true;
					parent = child.get();
					break;
				}
			}
			if (!found) {
				// insert it
				auto newChild      = std::make_unique<WorkspaceTree>();
				newChild->parent   = parent;
				newChild->type = isModule ? WorkspaceTree::MODULE : WorkspaceTree::FOLDER;
				newChild->row      = parent->children.size();
				newChild->name     = QString::fromStdString(component.string());
				parent->children.push_back(std::move(newChild));

				parent = parent->children[parent->children.size() - 1].get();
			}
		}
	}
	
	tree->children.push_back(std::move(srcTree));
	mTree = tree.get();

	mModel = std::make_unique<ModuleTreeModel>(std::move(tree), *mContext);
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
