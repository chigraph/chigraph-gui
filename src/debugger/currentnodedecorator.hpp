#pragma once

#ifndef CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP
#define CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP

#include <nodes/NodeDataModel>
#include <nodes/NodeGeometry>

class CurrentNodeDecorator : public QtNodes::NodePainterDelegate {

	void paint(QPainter* painter,
        QtNodes::NodeGeometry const& geometry,
        QtNodes::NodeIndex const& index) override {
		painter->drawRect(QRect(geometry.width() / 2, 0, 10, 10));
	}
};

#endif  // CHIGRAPHGUI_DEBUGGER_CURRENT_NODE_DECORATOR_HPP
