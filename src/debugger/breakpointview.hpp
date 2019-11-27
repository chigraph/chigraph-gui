#ifndef CHIGRAPHGUI_DEBUGGER_BREAKPOINT_VIEW_HPP
#define CHIGRAPHGUI_DEBUGGER_BREAKPOINT_VIEW_HPP

#include <KLocalizedString>
#include <QTreeWidget>
#include <chi/Debugger/Debugger.hpp>

#include "../toolview.hpp"

class BreakpointView : public QTreeWidget, public ToolView {
	class BreakpointItem;

public:
	BreakpointView();

	void addBreakpoint(chi::NodeInstance& inst);

	/// Remove a breakpoint
	/// \return true if one was removed, false if it wasn't found
	bool removeBreakpoint(chi::NodeInstance& inst);

	const std::unordered_map<chi::NodeInstance*, BreakpointItem*>& breakpoints() {
		return mBreakpoints;
	}

private:
	// ToolView interface
	QWidget*           toolView() override { return this; }
	Qt::DockWidgetArea defaultArea() const override { return Qt::BottomDockWidgetArea; }
	QString            label() override { return i18n("Breakpoints"); }
	QString            dockObjectName() override { return QStringLiteral("breakpoints"); }

	std::unordered_map<chi::NodeInstance*, BreakpointItem*> mBreakpoints;
};

#endif  // CHIGRAPHGUI_DEBUGGER_BREAKPOINT_VIEW_HPP
