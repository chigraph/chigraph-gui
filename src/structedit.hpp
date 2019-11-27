#pragma once

#ifndef CHIGRAPHGUI_STRUCT_EDIT_HPP
#define CHIGRAPHGUI_STRUCT_EDIT_HPP

#include <QWidget>
#include <chi/Fwd.hpp>

class CentralTabView;

class StructEdit : public QWidget {
	Q_OBJECT
public:
	explicit StructEdit(chi::GraphStruct& str, CentralTabView* tabWidget);

private:
	void makeGUI();

	void refreshReferences();

	chi::GraphStruct* mStruct;
	CentralTabView*   mTabs;
};

#endif  // CHIGRAPHGUI_STRUCT_EDIT_HPP
