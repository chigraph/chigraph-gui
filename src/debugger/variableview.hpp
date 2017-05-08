#pragma once

#ifndef CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP
#define CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP

#include "../toolview.hpp"

#include <QTreeWidget>

#include <KLocalizedString>

#include <lldb/API/SBFrame.h>

class VariableView : public QTreeWidget, public ToolView {
public:
	VariableView();

	void setFrame(lldb::SBFrame frame);

private:
	// ToolView interface
	QWidget*           toolView() override { return this; }
	Qt::DockWidgetArea defaultArea() const override { return Qt::LeftDockWidgetArea; }
	QString            label() override { return i18n("Variables"); }
	QString dockObjectName() override { return QStringLiteral("variable-view"); }
};

#endif  // CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP
