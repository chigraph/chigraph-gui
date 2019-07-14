#pragma once

#ifndef CHIGRAPHGUI_MODULE_TREE_MODEL_HPP
#define CHIGRAPHGUI_MODULE_TREE_MODEL_HPP

#include <QAbstractItemModel>

#include <memory>

#include <chi/Fwd.hpp>

#include <filesystem>

struct WorkspaceTree {
	enum eType { FUNCTION, MODULE, STRUCT, FOLDER };

	WorkspaceTree*                              parent = nullptr;
	std::vector<std::unique_ptr<WorkspaceTree>> children;
	chi::GraphModule*                           module = nullptr;
	chi::GraphFunction*                         func   = nullptr;
	chi::GraphStruct*                           str    = nullptr;
	QString                                     name;
	bool                                        dirty = false;
	int                                         row   = 0;
	eType                                       type;

	// get full name
	std::filesystem::path fullName() const {
		std::filesystem::path ret = name.toStdString();

		auto parent = this->parent;
		while (parent != nullptr) {
			// don't ` / ret` when ret is empty to avoid trailing slash
			if (ret.empty()) {
				ret = parent->name.toStdString();
			} else {
				ret = parent->name.toStdString() / ret;
			}
			parent = parent->parent;
		}

		if (ret.string().size() <= 4) { return ""; }
		ret = ret.string().substr(4);  // remove src/ at the beginning

		return ret;
	}
};

class ModuleTreeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	static QIcon iconForItemType(WorkspaceTree::eType type);

	enum Filter {
		Folders     = 0b1,
		ModuleBit   = 0b10,
		Modules     = ModuleBit | Folders,
		FunctionBit = 0b100,
		Functions   = FunctionBit | Folders | Modules,
		StructBit   = 0b1000,
		Structs     = StructBit | Folders | Modules,
		All         = Structs | Functions | Modules | Folders,
	};

	ModuleTreeModel(std::unique_ptr<WorkspaceTree> t, chi::Context& ctx,
	                Filter filter = Filter::All)
	    : mTree{std::move(t)}, mCtx{&ctx}, mFilter{filter} {}

	// create a model from just the context
	static std::unique_ptr<ModuleTreeModel> createFromContext(chi::Context& context,
	                                                          Filter        filter = Filter::All);

	void updateModule(const std::filesystem::path& name);

	QModelIndex indexFromName(const std::filesystem::path& name, WorkspaceTree::eType type);

	int           columnCount(const QModelIndex& parent) const override;
	QModelIndex   index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex   parent(const QModelIndex& index) const override;
	bool          hasChildren(const QModelIndex& index) const override;
	bool          canFetchMore(const QModelIndex& index) const override;
	void          fetchMore(const QModelIndex& index) override;
	int           rowCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

	WorkspaceTree* tree() const { return mTree.get(); }

signals:
	void functionRenamed(chi::GraphFunction& func, const std::string& oldName,
	                     const std::vector<chi::NodeInstance*> updatedNodes);
	void structRenamed(chi::GraphStruct& str, const std::string& oldName,
	                   const std::vector<chi::NodeInstance*> updatedNodes);

private:
	std::unique_ptr<WorkspaceTree> mTree;
	chi::Context*                  mCtx;
	Filter                         mFilter;
};

#endif  // CHIGRAPHGUI_MODULE_TREE_MODEL_HPP
