#include "functionview.hpp"
#include "mainwindow.hpp"

#include <QHBoxLayout>

#include <KMessageBox>

#include <chi/DataType.hpp>
#include <chi/FunctionValidator.hpp>
#include <chi/GraphModule.hpp>
#include <chi/NodeInstance.hpp>
#include <chi/Support/Result.hpp>

#include <nodes/NodeGraphicsObject>
#include <nodes/ConnectionStyle>

#include "chigraphnodemodel.hpp"

using namespace QtNodes;

FunctionView::FunctionView(chi::GraphFunction& func_, QWidget* parent)
    : QWidget(parent), mFunction{&func_} {
	auto hlayout = new QHBoxLayout(this);

	// TODO: see how to actually set the colors
	ConnectionStyle::setConnectionStyle(R"(
    {
      "ConnectionStyle": {
        "UseDataDefinedColors": true
      }
    }
    )");

	hlayout->setMargin(0);
	hlayout->setSpacing(0);

	mModel = new ChigraphFlowSceneModel(*function());

	mScene = new FlowScene(mModel);
	mModel->updateValidation();

	mView = new FlowView(mScene);
	mView->setSceneRect(-320000, -320000, 640000, 640000);
	mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	connect(mModel, &ChigraphFlowSceneModel::nodeWasDoubleClicked, this,
	        [this](QtNodes::NodeIndex const& index, QPoint const&) {
		        auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

		        auto& module = inst->type().module();
		        // cast to a GraphModule
		        auto* castedModule = dynamic_cast<chi::GraphModule*>(&module);
		        if (castedModule == nullptr) { return; }

		        auto func = castedModule->functionFromName(inst->type().name());
		        if (func == nullptr) { return; }

		        emit functionDoubleClicked(*func);
		    });

	hlayout->addWidget(mView);
}

std::vector<chi::NodeInstance*> FunctionView::selectedNodes() {
	std::vector<chi::NodeInstance*> ret;

	auto guiNodes = mScene->selectedNodes();
	ret.reserve(guiNodes.size());

	for (auto n : guiNodes) {
		auto toAdd = reinterpret_cast<chi::NodeInstance*>(n.internalPointer());

		if (toAdd) { ret.push_back(toAdd); }
	}

	return ret;
}

void FunctionView::selectNode(chi::NodeInstance& node) {
	// clear the selection
	scene().clearSelection();
	auto guiNode = mScene->nodeGraphicsObject(mModel->nodeIndex(node));
	if (guiNode == nullptr) { return; }

	// then select it
	guiNode->setSelected(true);
}

void FunctionView::centerOnNode(chi::NodeInstance& inst) {
	auto guiInst = mScene->nodeGraphicsObject(mModel->nodeIndex(inst));

	if (guiInst) { mView->centerOn(guiInst); }
}
