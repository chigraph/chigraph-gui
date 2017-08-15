#pragma once

#ifndef CHIG_GUI_CHIGNODEGUI_HPP
#define CHIG_GUI_CHIGNODEGUI_HPP

#include <nodes/FlowSceneModel>

#include <chi/NodeInstance.hpp>
#include <chi/NodeType.hpp>

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QValidator>

#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>

#include <memory>

class ChigraphFlowSceneModel : public QtNodes::FlowSceneModel {
  QStringList modelRegistry() const override

  QString nodeTypeCatergory(QString const& /*name*/) const { return {}; }
  QString converterNode(NodeDataType const& /*lhs*/, NodeDataType const& ) const { return {}; }

  // Retrieval functions
  //////////////////////

  QList<QUuid> nodeUUids() const override;
  NodeIndex nodeIndex(const QUuid& ID) const override;
  QString nodeTypeIdentifier(NodeIndex const& index) const override;
  QString nodeCaption(NodeIndex const& index) const override;
  QPointF nodeLocation(NodeIndex const& index) const override;
  QWidget* nodeWidget(NodeIndex const& index) const override;
  bool nodeResizable(NodeIndex const& index) const override;
  NodeValidationState nodeValidationState(NodeIndex const& index) const override;
  
  /// Get the validation error/warning
  QString nodeValidationMessage(NodeIndex const& index) const override

  /// Get the painter delegate
  NodePainterDelegate* nodePainterDelegate(NodeIndex const& index) const override
  
  /// Get the style
  NodeStyle nodeStyle(NodeIndex const& index) const { return {}; }
  
  /// Get the count of DataPorts
  unsigned int nodePortCount(NodeIndex const& index, PortType portType) const override

  /// Get the port caption
  QString nodePortCaption(NodeIndex const& index, PortIndex portID, PortType portType) const override

  /// Get the port data type
  NodeDataType nodePortDataType(NodeIndex const& index, PortIndex portID, PortType portType) const override

  /// Port Policy
  ConnectionPolicy nodePortConnectionPolicy(NodeIndex const& index, PortIndex portID, PortType portType) const override

  /// Get a connection at a port
  std::vector<std::pair<NodeIndex, PortIndex>> nodePortConnections(NodeIndex const& index, PortIndex portID, PortType portTypes) const override

  // Mutation functions
  /////////////////////

  /// Remove a connection
  bool removeConnection(NodeIndex const& /*leftNode*/, PortIndex /*leftPortID*/, NodeIndex const& /*rightNode*/, PortIndex /*rightPortID*/) { return false; }

  /// Add a connection
  bool addConnection(NodeIndex const& /*leftNode*/, PortIndex /*leftPortID*/, NodeIndex const& /*rightNode*/, PortIndex /*rightPortID*/) { return false; }

  /// Remove a node
  bool removeNode(NodeIndex const& /*index*/) { return false; }

  /// Add a  -- return {} if it fails
  QUuid addNode(QString const& /*typeID*/, QPointF const& /*pos*/) { return QUuid{}; }

  /// Move a node to a new location
  bool moveNode(NodeIndex const& /*index*/, QPointF /*newLocation*/) { return false; }
};
