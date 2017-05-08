#include "launchconfigurationmanager.hpp"

#include <chi/Context.hpp>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QUuid>
#include <QVector>

LaunchConfiguration::LaunchConfiguration(KConfigGroup grp) : mConfigGroup{grp} {}

LaunchConfigurationManager::LaunchConfigurationManager(chi::Context& context) : mContext{&context} {
	KConfigGroup contextConfig(KSharedConfig::openConfig(),
	                           context.workspacePath().string().c_str());

	mConfigGroup = contextConfig.group("launchconfigurations");

	auto configurations = mConfigGroup.readEntry("configurations", QStringList());
	auto currentName    = mConfigGroup.readEntry("current", QString());

	for (const auto& configName : configurations) {
		mConfigurations.emplace_back(mConfigGroup.group(configName));

		if (configName == currentName) { mCurrent = mConfigurations[mConfigurations.size() - 1]; }
	}
}

LaunchConfiguration LaunchConfigurationManager::newConfiguration() {
	// generate uuid for it
	auto uuid = QUuid::createUuid();

	// add it to the list
	mConfigGroup.writeEntry("configurations",
	                        mConfigGroup.readEntry("configurations", QStringList())
	                            << uuid.toString());

	auto group = mConfigGroup.group(uuid.toString());

	mConfigurations.emplace_back(group);

	return mConfigurations[mConfigurations.size() - 1];
}

void LaunchConfigurationManager::removeConfiguration(LaunchConfiguration config) {
	// find it and remove it from the list
	auto iter = std::find(mConfigurations.begin(), mConfigurations.end(), config);
	if (iter == mConfigurations.end()) {
		qDebug() << "Failed to find config to delete: " << config.id();
		return;
	}

	mConfigurations.erase(iter);

	mConfigGroup.deleteEntry(config.id());
}

void LaunchConfigurationManager::setCurrentConfiguration(LaunchConfiguration config) {
	mCurrent = config;

	mConfigGroup.writeEntry("current", config.id());
}
