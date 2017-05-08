#pragma once

#ifndef CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP
#define CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP

#include <QPushButton>

#include <chi/Fwd.hpp>

#include <boost/filesystem/path.hpp>

#include "moduletreemodel.hpp"

class ChiItemSelectWidget : public QPushButton {
	Q_OBJECT
public:
	
	ChiItemSelectWidget(chi::Context& ctx, WorkspaceTree::eType type);

	void setItem(const boost::filesystem::path& newItem);
	
	boost::filesystem::path item() const;
	
private:
	boost::filesystem::path mData;
signals:
	void itemChanged(const boost::filesystem::path& itemName);
};

#endif // CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP
