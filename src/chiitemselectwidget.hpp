#pragma once

#ifndef CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP
#define CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP

#include <QPushButton>
#include <chi/Fwd.hpp>
#include <filesystem>

#include "moduletreemodel.hpp"

class ChiItemSelectWidget : public QPushButton {
	Q_OBJECT
public:
	ChiItemSelectWidget(chi::Context& ctx, WorkspaceTree::eType type);

	void setItem(const std::filesystem::path& newItem);

	std::filesystem::path item() const;

private:
	std::filesystem::path mData;
signals:
	void itemChanged(const std::filesystem::path& itemName);
};

#endif  // CHIGRAPHGUI_CHI_ITEM_SELECT_WIDGET_HPP
