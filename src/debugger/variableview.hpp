#pragma once

#ifndef CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP
#define CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP

#include <lldb/API/SBFrame.h>

#include <KLocalizedString>
#include <QTreeWidget>

#include "../toolview.hpp"

class VariableView : public QTreeWidget, public ToolView {
public:
	VariableView();

	void setFrame(lldb::SBFrame frame);

private:
	// ToolView interface
	QWidget*           toolView() override { return this; }
	Qt::DockWidgetArea defaultArea() const override { return Qt::LeftDockWidgetArea; }
	QString            label() override { return i18n("Variables"); }
	QString            dockObjectName() override { return QStringLiteral("variable-view"); }
};

#endif  // CHIGRAPHGUI_DEBUGGER_VARIABLE_VIEW_HPP
