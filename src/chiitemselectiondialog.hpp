#pragma once

#ifndef CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP
#define CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP

#include <QDialog>

#include <chi/Fwd.hpp>

#include <filesystem>

#include "moduletreemodel.hpp"

class QTreeView;

class ChiItemSelectionDialog : public QDialog {
public:
	ChiItemSelectionDialog(chi::Context& ctx, std::filesystem::path* toFill,
	                       WorkspaceTree::eType type, QWidget* parent = nullptr);

	void setCurrentItem(const std::filesystem::path& newItem);

	static std::filesystem::path getItem(QWidget* parent, chi::Context& ctx, const QString& title,
	                                     WorkspaceTree::eType         type,
	                                     const std::filesystem::path& startWithSelection = {});

private slots:
	void tryAccept(const QModelIndex& index);

private:
	WorkspaceTree::eType mType;

	std::filesystem::path*           mToFill;
	QTreeView*                       mTreeView;
	std::unique_ptr<ModuleTreeModel> mModel;
};

#endif  // CHIGRAPHGUI_CHI_ITEM_SELECTION_DIALOG_HPP
