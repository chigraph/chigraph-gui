#pragma once

#ifndef CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP
#define CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP

#include <nodes/NodeDataModel>

// TODO: better solution
#include <../src/NodeGraphicsObject.hpp>

class CurrentNodeDecorator : public QtNodes::NodePainterDelegate {
	void paint(QPainter* painter, QtNodes::NodeGraphicsObject const& ngo) override {
		painter->drawRect(QRect(ngo.geometry().width() / 2, 0, 10, 10));
	}
};

#endif  // CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP
