#include "moduletreemodel.hpp"

#include <KMessageBox>
#include <chi/Context.hpp>
#include <chi/GraphFunction.hpp>
#include <chi/GraphModule.hpp>
#include <chi/GraphStruct.hpp>
#include <chi/NodeInstance.hpp>
#include <chi/Support/Result.hpp>
#include <filesystem>

namespace fs = std::filesystem;

QIcon ModuleTreeModel::iconForItemType(WorkspaceTree::eType type) {
	switch (type) {
	case WorkspaceTree::MODULE: return QIcon::fromTheme(QStringLiteral("code-block"));
	case WorkspaceTree::FUNCTION: return QIcon::fromTheme(QStringLiteral("code-context"));
	case WorkspaceTree::STRUCT: return QIcon::fromTheme(QStringLiteral("code-class"));
	case WorkspaceTree::FOLDER: return QIcon::fromTheme(QStringLiteral("stock_folder"));
	default: return {};
	}
}

std::unique_ptr<ModuleTreeModel> ModuleTreeModel::createFromContext(chi::Context& context,
                                                                    Filter        filter) {
	auto modules = context.listModulesInWorkspace();

	auto tree = std::make_unique<WorkspaceTree>();

	// add src object
	auto srcTree    = std::make_unique<WorkspaceTree>();
	srcTree->parent = tree.get();
	srcTree->type   = WorkspaceTree::FOLDER;
	srcTree->row    = 0;
	srcTree->name   = QStringLiteral("src");

	// create the tree
	for (const auto& modStr : modules) {
		auto           mod    = fs::path(modStr);
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
				if ((isModule && (filter & ModuleBit)) || (!isModule && (filter & Folders))) {
					// insert it
					auto newChild    = std::make_unique<WorkspaceTree>();
					newChild->parent = parent;
					newChild->type   = isModule ? WorkspaceTree::MODULE : WorkspaceTree::FOLDER;
					newChild->row    = parent->children.size();
					newChild->name   = QString::fromStdString(component.string());
					parent->children.push_back(std::move(newChild));

					parent = parent->children[parent->children.size() - 1].get();
				}
			}
		}
	}

	tree->children.push_back(std::move(srcTree));

	return std::make_unique<ModuleTreeModel>(std::move(tree), context, filter);
}

void ModuleTreeModel::updateModule(const std::filesystem::path& name) {
	auto idx = indexFromName(name, WorkspaceTree::MODULE);

	if (!idx.isValid()) { return; }

	auto tree = static_cast<WorkspaceTree*>(idx.internalPointer());

	// don't even bother
	if (tree->module == nullptr) { return; }

	// delete all subs
	beginRemoveRows(idx, 0, tree->children.size() - 1);
	tree->children.clear();
	endRemoveRows();

	beginInsertRows(idx, 0,
	                (mFilter & FunctionBit ? tree->module->functions().size() - 1 : 0) +
	                    (mFilter & StructBit ? tree->module->structs().size() - 1 : 0));
	tree->module = nullptr;
	fetchMore(idx);
	endInsertRows();
}

QModelIndex ModuleTreeModel::indexFromName(const std::filesystem::path& name,
                                           WorkspaceTree::eType         type) {
	auto currentItem = mTree->children[0].get();

	for (auto iter = name.begin(); iter != name.end(); ++iter) {
		bool isLastItem   = iter == --name.end();
		auto expectedType = isLastItem ? type : WorkspaceTree::FOLDER;

		bool succeeded = false;

		// find it in the children
		for (const auto& item : currentItem->children) {
			if (item->name.toStdString() == *iter && item->type == expectedType) {
				currentItem = item.get();
				succeeded   = true;
				break;
			}
		}

		if (!succeeded) { return {}; }
	}

	return createIndex(currentItem->row, 0, currentItem);
}

int ModuleTreeModel::columnCount(const QModelIndex& parent) const { return 1; }

QModelIndex ModuleTreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) { return {}; }

	WorkspaceTree* parentItem;
	if (parent.isValid()) {
		parentItem = static_cast<WorkspaceTree*>(parent.internalPointer());
	} else {
		parentItem = tree();
	}

	// make cast valid
	Q_ASSERT(row >= 0);

	if ((size_t)row < parentItem->children.size()) {
		return createIndex(row, column, parentItem->children[row].get());
	}
	return {};
}

QModelIndex ModuleTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid()) { return {}; }

	auto childItem  = static_cast<WorkspaceTree*>(index.internalPointer());
	auto parentItem = childItem->parent;

	if (parentItem == tree()) { return {}; }

	return createIndex(parentItem->row, 0, parentItem);
}

bool ModuleTreeModel::hasChildren(const QModelIndex& index) const {
	if (!index.isValid()) { return true; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	// if both struct and function are disabled, then just return false for module
	return (item->type == WorkspaceTree::MODULE &&
	        (mFilter & StructBit || mFilter & FunctionBit)) ||
	       item->type == WorkspaceTree::FOLDER;
}

bool ModuleTreeModel::canFetchMore(const QModelIndex& index) const {
	if (!index.isValid()) { return false; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	return item->type == WorkspaceTree::MODULE && (mFilter & StructBit || mFilter & FunctionBit);
}

void ModuleTreeModel::fetchMore(const QModelIndex& index) {
	if (!index.isValid()) { return; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	if (item->module != nullptr) {
		// it's already been fetched
		return;
	}

	// get the name
	fs::path p = item->fullName();

	// load it
	chi::ChiModule* mod;
	chi::Result     res = mCtx->loadModule(p, &mod);
	if (!res) {
		KMessageBox::detailedError(nullptr,
		                           R"(Failed to load JsonModule from file ")" +
		                               QString::fromStdString(p.string()) + R"(")",
		                           QString::fromStdString(res.dump()), "Error Loading");

		return;
	}

	item->module = static_cast<chi::GraphModule*>(mod);

	if (mFilter & FunctionBit) {
		// add functions
		for (const auto& func : item->module->functions()) {
			auto child    = std::make_unique<WorkspaceTree>();
			child->func   = func.get();
			child->parent = item;
			child->name   = QString::fromStdString(func->name());
			child->row    = item->children.size();
			child->type   = WorkspaceTree::FUNCTION;

			item->children.push_back(std::move(child));
		}
	}

	if (mFilter & StructBit) {
		// add structs
		for (const auto& str : item->module->structs()) {
			auto child    = std::make_unique<WorkspaceTree>();
			child->str    = str.get();
			child->name   = QString::fromStdString(str->name());
			child->parent = item;
			child->row    = item->children.size();
			child->type   = WorkspaceTree::STRUCT;

			item->children.push_back(std::move(child));
		}
	}
}

int ModuleTreeModel::rowCount(const QModelIndex& index) const {
	WorkspaceTree* parentItem;
	if (index.isValid()) {
		parentItem = static_cast<WorkspaceTree*>(index.internalPointer());
	} else {
		parentItem = tree();
	}

	return parentItem->children.size();
}

QVariant ModuleTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) { return {}; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	switch (role) {
	case Qt::DisplayRole:
		if (item->dirty) {
			return "* " + item->name;
		} else {
			return item->name;
		}
	case Qt::DecorationRole: return iconForItemType(item->type);
	case Qt::FontRole:
		if (item->dirty || (item->parent != nullptr && item->parent->dirty)) {
			QFont bold;
			bold.setBold(true);
			return bold;
		}
		return {};
	default: return {};
	}
}

Qt::ItemFlags ModuleTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid()) { return {}; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	if (item->type == WorkspaceTree::FUNCTION || item->type == WorkspaceTree::STRUCT) {
		flags |= Qt::ItemIsEditable;
	}

	return flags;
}

bool ModuleTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid()) { return {}; }

	if (role == Qt::EditRole) {
		auto item = static_cast<WorkspaceTree*>(index.internalPointer());

		if (value.toString().isEmpty()) { return false; }

		switch (item->type) {
		case WorkspaceTree::FUNCTION: {
			auto oldName = item->func->name();

			// set the function name
			auto nodesToUpdate = item->func->setName(value.toString().toStdString());
			item->name         = QString::fromStdString(item->func->name());

			emit functionRenamed(*item->func, oldName, nodesToUpdate);

		} break;
		case WorkspaceTree::STRUCT: {
			auto oldName = item->str->name();

			// set the struct name
			auto nodesToUpdate = item->str->setName(value.toString().toStdString());
			item->name         = QString::fromStdString(item->str->name());

			emit structRenamed(*item->str, oldName, nodesToUpdate);

		} break;
		default: return false;
		}

		dataChanged(index, index, {Qt::DisplayRole});
		return true;
	}

	return false;
}
