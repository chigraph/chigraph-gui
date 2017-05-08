#pragma once

#ifndef CHIGRAPHGUI_MODULE_TREE_MODEL_HPP
#define CHIGRAPHGUI_MODULE_TREE_MODEL_HPP

#include <QAbstractItemModel>

#include <memory>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

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
	boost::filesystem::path fullName() const {
		boost::filesystem::path ret;

		auto parent = this;
		while (parent != nullptr) {
			ret    = parent->name.toStdString() / ret;
			parent = parent->parent;
		}
		ret = ret.string().substr(4);  // remove src/ at the beginning

		return ret;
	}
};

class ModuleTreeModel : public QAbstractItemModel {
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
	    : tree{std::move(t)}, mCtx{&ctx}, mFilter{filter} {}

	// create a model from just the context
	static std::unique_ptr<ModuleTreeModel> createFromContext(chi::Context& context,
	                                                          Filter        filter = Filter::All);

	QModelIndex indexFromName(const boost::filesystem::path& name, WorkspaceTree::eType type);

	int columnCount(const QModelIndex& parent) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	bool hasChildren(const QModelIndex& index) const override;
	bool canFetchMore(const QModelIndex& index) const override;
	void fetchMore(const QModelIndex& index) override;
	int rowCount(const QModelIndex& index) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	std::unique_ptr<WorkspaceTree> tree;
	chi::Context*                  mCtx;
	Filter                         mFilter;
};

#endif  // CHIGRAPHGUI_MODULE_TREE_MODEL_HPP
