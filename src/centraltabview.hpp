#pragma once

#ifndef CHIGRAPHGUI_FUNCTION_TAB_VIEW_HPP
#define CHIGRAPHGUI_FUNCTION_TAB_VIEW_HPP

#include <QString>
#include <QTabWidget>

#include <KXMLGUIClient>

#include <chi/Fwd.hpp>

#include <unordered_map>

#include <boost/filesystem.hpp>

class FunctionView;
class StructEdit;

class CentralTabView : public QTabWidget, public KXMLGUIClient {
	Q_OBJECT
public:
	CentralTabView(QWidget* parent = nullptr);

	void selectNewFunction(chi::GraphFunction& func);
	void selectNewStruct(chi::GraphStruct& str);
	void centerOnNode(chi::NodeInstance& inst);
	void selectNode(chi::NodeInstance& inst);

	// refresh all the functions in the module
	void refreshModule(chi::GraphModule& mod);

	FunctionView* viewFromFunction(chi::GraphFunction& func);
	FunctionView* viewFromFunctionName(const QString& fullName);
	FunctionView* viewFromFunctionName(const boost::filesystem::path& mod,
	                                   const std::string&             function);

	StructEdit* viewFromStruct(chi::GraphStruct& str);
	StructEdit* viewFromStructName(const QString& fullName);
	StructEdit* viewFromStructName(const boost::filesystem::path& mod, const std::string& module);

	void closeView(FunctionView* view);
	void closeView(StructEdit* view);

	FunctionView* currentView();

signals:
	void dirtied(chi::GraphModule& mod);
	void functionViewChanged(FunctionView* func, bool newlyOpened);
	void structViewChanged(StructEdit* func, bool newlyOpened);

public slots:
	void functionRenamed(chi::GraphFunction& func, const std::string& oldName,
	                     const std::vector<chi::NodeInstance*>& changed);
	void structRenamed(chi::GraphStruct& str, const std::string& oldName,
	                   const std::vector<chi::NodeInstance*>& changed);

	void functionDeleted(chi::GraphModule& mod, const std::string& funcName);
	void structDeleted(chi::GraphModule& mod, const std::string& strName);

private:
	void closeTab(int idx);

	std::map<QString, FunctionView*> mOpenFunctions;
	std::map<QString, StructEdit*>   mOpenStructs;
};

#endif  // CHIGRAPHGUI_FUNCTION_TAB_VIEW_HPP
