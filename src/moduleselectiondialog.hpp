#pragma once

#ifndef CHIGRAPHGUI_MODULE_SELECTION_DIALOG_HPP
#define CHIGRAPHGUI_MODULE_SELECTION_DIALOG_HPP

#include <QDialog>

#include "moduletreemodel.hpp"

#include <boost/filesystem/path.hpp>

class ModuleSelectionDialog : public QDialog {
public:
	ModuleSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill, QWidget* parent = nullptr);
	
	static boost::filesystem::path getModule(QWidget* parent, chi::Context& ctx);
	
private:
	std::unique_ptr<ModuleTreeModel> mModel;
};

#endif // CHIGRAPHGUI_MODULE_SELECTION_DIALOG_HPP
