#pragma once

#ifndef CHIGRAPHGUI_FOLDER_SELECT_WIDGET_HPP
#define CHIGRAPHGUI_FOLDER_SELECT_WIDGET_HPP

#include <QPushButton>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

class FolderSelectWidget : public QPushButton {
	Q_OBJECT
public:
	
	FolderSelectWidget(chi::Context& ctx);

	void setFolder(const boost::filesystem::path& newModule);
	
	boost::filesystem::path folder() const;
	
signals:
	void folderChanged(const boost::filesystem::path& moduleName);
};

#endif // CHIGRAPHGUI_MODULE_SELECT_WIDGET_HPP

