#pragma once

#ifndef CHIGUI_MODULE_PROPERTIES_DIALOG_HPP
#define CHIGUI_MODULE_PROPERTIES_DIALOG_HPP

#include <chi/Fwd.hpp>

#include <QDialog>

// for editing properties like dependencies and C support
class ModulePropertiesDialog : public QDialog {
public:
	ModulePropertiesDialog(QWidget* parent, chi::GraphModule& modToEdit);

private:
	chi::GraphModule* mModule;
};

#endif  // CHIGUI_MODULE_PROPERTIES_DIALOG_HPP
