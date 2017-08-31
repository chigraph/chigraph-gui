#pragma once

#ifndef CHIGGUI_FUNCTIONVIEW_HPP
#define CHIGGUI_FUNCTIONVIEW_HPP

#include <nodes/FlowScene>
#include <nodes/FlowView>

#include <chi/GraphFunction.hpp>
#include <chi/GraphModule.hpp>
#include <chi/NodeInstance.hpp>

#include <memory>
#include <unordered_map>

#include "chigraphnodemodel.hpp"

class FunctionView : public QWidget {
	Q_OBJECT
public:
	FunctionView(chi::GraphFunction& func_, QWidget* parent = nullptr);

	void centerOnNode(chi::NodeInstance& inst);
	
	void refreshGuiForNode(chi::NodeInstance& inst) {
		emit mModel->nodePortUpdated(mModel->nodeIndex(inst));
	}

	chi::GraphFunction* function() const { return mFunction; }

	QtNodes::FlowScene& scene() const { return *mScene; }
	
	ChigraphFlowSceneModel& model() const { return *mModel; }

	std::vector<chi::NodeInstance*> selectedNodes();

	void selectNode(chi::NodeInstance& node);

signals:
	void dirtied();
	void functionDoubleClicked(chi::GraphFunction& func);

private:

	QtNodes::FlowScene* mScene;
	QtNodes::FlowView*  mView;
	ChigraphFlowSceneModel* mModel;

	chi::GraphFunction* mFunction;
};

#endif  // CHIGGUI_FUNCTIONVIEW_HPP
