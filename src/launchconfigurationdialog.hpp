#pragma once

#ifndef CHIGRPAHGUI_LAUNCH_CONFIGURATION_DIALOG_HPP
#define CHIGRPAHGUI_LAUNCH_CONFIGURATION_DIALOG_HPP

#include <QDialog>

#include "launchconfigurationmanager.hpp"

#include <boost/filesystem/path.hpp>

class QListWidget;
class QLineEdit;

class ChiItemSelectWidget;

class LaunchConfigurationDialog : public QDialog {
	Q_OBJECT
public:
	LaunchConfigurationDialog(LaunchConfigurationManager& manager);

private slots:
	void addNewConfig();
	void selectConfig(const QString& newConfig);

	void nameChanged(const QString& newName);
	void wdChanged(const QString& newWd);
	void moduleChanged(const boost::filesystem::path& newModule);
	void argsChanged(const QString& newArgs);

private:
	LaunchConfigurationManager* mManager;
	LaunchConfiguration         currentlyEditing;

	QLineEdit* mWdEdit;
	ChiItemSelectWidget* mModuleEdit;
	QLineEdit* mArgsEdit;

	QListWidget* mConfigList;
};

#endif  // CHIGRPAHGUI_LAUNCH_CONFIGURATION_DIALOG_HPP
