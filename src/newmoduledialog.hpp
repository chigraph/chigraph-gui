#pragma once

#ifndef CHIGRAPHGUI_NEW_MODULE_DIALOG_HPP
#define CHIGRAPHGUI_NEW_MODULE_DIALOG_HPP

#include <QDialog>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

class ChiItemSelectWidget;
class QLineEdit;

class NewModuleDialog : public QDialog {
public:
	NewModuleDialog(QWidget* parent, chi::Context& context, const boost::filesystem::path& folder = "");
	

private:
	ChiItemSelectWidget* mFolderWidget;
	QLineEdit* mNameEdit;
};

#endif // CHIGRAPHGUI_NEW_MODULE_DIALOG_HPP
