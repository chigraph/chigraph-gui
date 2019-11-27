#pragma once

#ifndef CHIGGUI_PARAMLISTWIDGET_HPP
#define CHIGGUI_PARAMLISTWIDGET_HPP

#include <QListWidget>
#include <QTableView>
#include <chi/Fwd.hpp>

class FunctionView;

QStringList createTypeOptions(const chi::GraphModule& mod);

class ParamListWidget : public QWidget {
	Q_OBJECT

public:
	enum Type { Input, Output };

	explicit ParamListWidget(QWidget* parent = nullptr);

	void setFunction(FunctionView* func, Type ty);
signals:
	void dirtied();

private:
	void refreshEntry();
	void refreshExits();

	FunctionView* mFunc = nullptr;
	Type          mType = Input;
};

#endif  // CHIGGUI_PARAMLISTWIDGET_HPP
