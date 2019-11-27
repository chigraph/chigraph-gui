#pragma once

#ifndef CHI_GUI_CHIGNODEGUI_HPP
#define CHI_GUI_CHIGNODEGUI_HPP

#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>
#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QValidator>
#include <chi/Fwd.hpp>
#include <chi/Support/Result.hpp>
#include <memory>
#include <nodes/FlowSceneModel>
#include <nodes/NodeIndex>

class ChigraphFlowSceneModel : public QtNodes::FlowSceneModel {
	Q_OBJECT
public:
	ChigraphFlowSceneModel(chi::GraphFunction& func);
	void updateValidation();

	QStringList modelRegistry() const override;

	QString nodeTypeCategory(QString const& name) const override;
	bool    getTypeConvertable(QtNodes::TypeConverterId const& id) const override;

	// Retrieval functions
	//////////////////////

	QtNodes::NodeIndex nodeIndex(const chi::NodeInstance& node) const;

	QList<QUuid>                 nodeUUids() const override;
	QtNodes::NodeIndex           nodeIndex(const QUuid& ID) const override;
	QString                      nodeTypeIdentifier(QtNodes::NodeIndex const& index) const override;
	QString                      nodeCaption(QtNodes::NodeIndex const& index) const override;
	QPointF                      nodeLocation(QtNodes::NodeIndex const& index) const override;
	QWidget*                     nodeWidget(QtNodes::NodeIndex const& index) const override;
	bool                         nodeResizable(QtNodes::NodeIndex const& index) const override;
	QtNodes::NodeValidationState nodeValidationState(
	    QtNodes::NodeIndex const& index) const override;

	/// Get the validation error/warning
	QString nodeValidationMessage(QtNodes::NodeIndex const& index) const override;

	/// Get the painter delegate
	QtNodes::NodePainterDelegate* nodePainterDelegate(
	    QtNodes::NodeIndex const& index) const override;

	/// Get the count of DataPorts
	unsigned int nodePortCount(QtNodes::NodeIndex const& index,
	                           QtNodes::PortType         portType) const override;

	/// Get the port caption
	QString nodePortCaption(QtNodes::NodeIndex const& index, QtNodes::PortIndex portID,
	                        QtNodes::PortType portType) const override;

	/// Get the port data type
	QtNodes::NodeDataType nodePortDataType(QtNodes::NodeIndex const& index,
	                                       QtNodes::PortIndex        portID,
	                                       QtNodes::PortType         portType) const override;

	/// Port Policy
	QtNodes::ConnectionPolicy nodePortConnectionPolicy(QtNodes::NodeIndex const& index,
	                                                   QtNodes::PortIndex        portID,
	                                                   QtNodes::PortType portType) const override;

	/// Get a connection at a port
	std::vector<std::pair<QtNodes::NodeIndex, QtNodes::PortIndex>> nodePortConnections(
	    QtNodes::NodeIndex const& index, QtNodes::PortIndex portID,
	    QtNodes::PortType portType) const override;

	// Mutation functions
	/////////////////////

	/// Remove a connection
	bool removeConnection(QtNodes::NodeIndex const& leftNode, QtNodes::PortIndex leftPortID,
	                      QtNodes::NodeIndex const& rightNode,
	                      QtNodes::PortIndex        rightPortID) override;

	/// Add a connection
	bool addConnection(QtNodes::NodeIndex const& leftNode, QtNodes::PortIndex leftPortID,
	                   QtNodes::NodeIndex const& rightNode,
	                   QtNodes::PortIndex        rightPortID) override;

	/// Remove a node
	bool removeNode(QtNodes::NodeIndex const& index) override;

	/// Add a  -- return {} if it fails
	QUuid addNode(QString const& typeID, QPointF const& pos) override;

	/// Move a node to a new location
	bool moveNode(QtNodes::NodeIndex const& index, QPointF newLocation) override;

	void connectionHovered(QtNodes::NodeIndex const& lhs, QtNodes::PortIndex lPortIndex,
	                       QtNodes::NodeIndex const& rhs, QtNodes::PortIndex rPortIndex,
	                       QPoint const& pos, bool entered) override {
		emit connectionWasHovered(lhs, lPortIndex, rhs, rPortIndex, pos, entered);
	}

	void nodeHovered(QtNodes::NodeIndex const& index, QPoint const& pos, bool entered) override {
		emit nodeWasHovered(index, pos, entered);
	}

	void nodeDoubleClicked(QtNodes::NodeIndex const& index, QPoint const& pos) override {
		emit nodeWasDoubleClicked(index, pos);
	}

signals:

	void connectionWasHovered(QtNodes::NodeIndex const& lhs, QtNodes::PortIndex lPortIndex,
	                          QtNodes::NodeIndex const& rhs, QtNodes::PortIndex rPortIndex,
	                          QPoint const& pos, bool entered);
	void nodeWasHovered(QtNodes::NodeIndex const& index, QPoint const& pos, bool entered);
	void nodeWasDoubleClicked(QtNodes::NodeIndex const& index, QPoint const& pos);

private:
	QWidget* createEmbeddedWidget(chi::NodeInstance& inst);

	chi::GraphFunction*                              mFunc;
	chi::Result                                      mValidation;
	std::unordered_map<chi::NodeInstance*, QWidget*> mEmbeddedWidgets;
};

#endif  // CHI_GUI_CHIGNODEGUI_HPP
