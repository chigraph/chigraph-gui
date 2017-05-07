#pragma once

#ifndef CHIGRAPHGUI_MODULE_SELECT_WIDGET_HPP
#define CHIGRAPHGUI_MODULE_SELECT_WIDGET_HPP

#include <QPushButton>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

class ModuleSelectWidget : public QPushButton {
	Q_OBJECT
public:
	
	ModuleSelectWidget(chi::Context& ctx);

	void setModule(const boost::filesystem::path& newModule);
	
signals:
	void moduleChanged(const boost::filesystem::path& moduleName);
};

#endif // CHIGRAPHGUI_MODULE_SELECT_WIDGET_HPP
