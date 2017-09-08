#pragma once

#ifndef CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP
#define CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP

#include <QDialog>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

#include "moduletreemodel.hpp"

class QTreeView;

class ChiItemSelectionDialog : public QDialog {
public:
	ChiItemSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill,
	                       WorkspaceTree::eType type, QWidget* parent = nullptr);

	void setCurrentItem(const boost::filesystem::path& newItem);

	static boost::filesystem::path getItem(QWidget* parent, chi::Context& ctx, const QString& title,
	                                       WorkspaceTree::eType           type,
	                                       const boost::filesystem::path& startWithSelection = {});

private slots:
	void tryAccept(const QModelIndex& index);

private:
	WorkspaceTree::eType mType;

	boost::filesystem::path*         mToFill;
	QTreeView*                       mTreeView;
	std::unique_ptr<ModuleTreeModel> mModel;
};

#endif  // CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP
