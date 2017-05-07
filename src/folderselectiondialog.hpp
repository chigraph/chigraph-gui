#pragma once

#ifndef CHIGRAPHGUI_FOLDER_SELECTION_DIALOG_HPP
#define CHIGRAPHGUI_FOLDER_SELECTION_DIALOG_HPP

#include <QDialog>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

#include "moduletreemodel.hpp"

class FolderSelectionDialog : public QDialog {
public:
	FolderSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill, QWidget* parent = nullptr);
	
	static boost::filesystem::path getFolder(QWidget* parent, chi::Context& ctx);
	
private:
	std::unique_ptr<ModuleTreeModel> mModel;
};

#endif // CHIGRAPHGUI_FOLDER_SELECTION_DIALOG_HPP
