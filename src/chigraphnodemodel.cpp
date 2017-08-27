#include "chigraphnodemodel.hpp"

#include <chi/GraphFunction.hpp>
#include <chi/GraphModule.hpp>

ChigraphFlowSceneModel::ChigraphFlowSceneModel(chi::GraphFunction& func) 
	: mFunc{&func} {
	
}

QStringList ChigraphFlowSceneModel::modelRegistry() const {
	
	QStringList ret;
	
	auto& mod = mFunc->module();
	auto& ctx = mFunc->context();
	
	// add dependencies
	for (const auto& dep : mod.dependencies()) {
		
	}
}

QString ChigraphFlowSceneModel::nodeTypeCatergory(QString const& name) const {
	
}
QString ChigraphFlowSceneModel::converterNode(QtNodes::NodeDataType const& lhs, QtNodes::NodeDataType const& rhs) const {
	
}

QList<QUuid> ChigraphFlowSceneModel::nodeUUids() const {
	
}
QtNodes::NodeIndex ChigraphFlowSceneModel::nodeIndex(const QUuid& ID) const {
	
}
QString ChigraphFlowSceneModel::nodeTypeIdentifier(QtNodes::NodeIndex const& index) const {
	
}
QString ChigraphFlowSceneModel::nodeCaption(QtNodes::NodeIndex const& index) const {
	
}
QPointF ChigraphFlowSceneModel::nodeLocation(QtNodes::NodeIndex const& index) const {
	
}
QWidget* ChigraphFlowSceneModel::nodeWidget(QtNodes::NodeIndex const& index) const {
	
}
bool ChigraphFlowSceneModel::nodeResizable(QtNodes::NodeIndex const& index) const {
	
}
QtNodes::NodeValidationState ChigraphFlowSceneModel::nodeValidationState(QtNodes::NodeIndex const& index) const {
	
}

/// Get the validation error/warning
QString ChigraphFlowSceneModel::nodeValidationMessage(QtNodes::NodeIndex const& index) const {
	
}

/// Get the painter delegate
QtNodes::NodePainterDelegate* ChigraphFlowSceneModel::nodePainterDelegate(QtNodes::NodeIndex const& index) const {
	
}

/// Get the style
QtNodes::NodeStyle ChigraphFlowSceneModel::nodeStyle(QtNodes::NodeIndex const& index) const {
	
}

/// Get the count of DataPorts
unsigned int ChigraphFlowSceneModel::nodePortCount(QtNodes::NodeIndex const& index, QtNodes::PortType portType) const {
	
}

/// Get the port caption
QString ChigraphFlowSceneModel::nodePortCaption(QtNodes::NodeIndex const& index, QtNodes::PortIndex portID, QtNodes::PortType portType) const {
	
}

/// Get the port data type
QtNodes::NodeDataType ChigraphFlowSceneModel::nodePortDataType(QtNodes::NodeIndex const& index, QtNodes::PortIndex portID, QtNodes::PortType portType) const {
	
}

/// Port Policy
QtNodes::ConnectionPolicy ChigraphFlowSceneModel::nodePortConnectionPolicy(QtNodes::NodeIndex const& index, QtNodes::PortIndex portID, QtNodes::PortType portType) const {
	
}

/// Get a connection at a port
std::vector<std::pair<QtNodes::NodeIndex, QtNodes::PortIndex>> ChigraphFlowSceneModel::nodePortConnections(QtNodes::NodeIndex const& index, QtNodes::PortIndex portID, QtNodes::PortType portTypes) const {
	
}

// Mutation functions
/////////////////////

/// Remove a connection
bool ChigraphFlowSceneModel::removeConnection(QtNodes::NodeIndex const& leftNode, QtNodes::PortIndex /*leftPortID*/, QtNodes::NodeIndex const& /*rightNode*/, QtNodes::PortIndex /*rightPortID*/) {
	
}

/// Add a connection
bool ChigraphFlowSceneModel::addConnection(QtNodes::NodeIndex const& leftNode, QtNodes::PortIndex leftPortID, QtNodes::NodeIndex const& rightNode, QtNodes::PortIndex rightPortID) {
	
}

/// Remove a node
bool ChigraphFlowSceneModel::removeNode(QtNodes::NodeIndex const& index) {
	
}

/// Add a  -- return {} if it fails
QUuid ChigraphFlowSceneModel::addNode(QString const& typeID, QPointF const& pos) {
	
}

/// Move a node to a new location
bool ChigraphFlowSceneModel::moveNode(QtNodes::NodeIndex const& index, QPointF newLocation) {
	
}
