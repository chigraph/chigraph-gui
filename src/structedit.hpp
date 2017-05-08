#pragma once

#ifndef CHIGRAPHGUI_STRUCT_EDIT_HPP
#define CHIGRAPHGUI_STRUCT_EDIT_HPP

#include <QWidget>

#include <chi/GraphStruct.hpp>

class StructEdit : public QWidget {
public:
	explicit StructEdit(chi::GraphStruct* str);
};

#endif  // CHIGRAPHGUI_STRUCT_EDIT_HPP
