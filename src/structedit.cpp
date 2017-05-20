#include "structedit.hpp"
#include "typeselector.hpp"
#include "centraltabview.hpp"
#include "execparamlistwidget.hpp"
#include "functionview.hpp"

#include <chi/GraphStruct.hpp>
#include <chi/Context.hpp>
#include <chi/GraphModule.hpp>
#include <chi/LangModule.hpp>
#include <chi/NodeInstance.hpp>

#include <QGridLayout>
#include <QLineEdit>
#include <QString>
#include <QPushButton>

StructEdit::StructEdit(chi::GraphStruct& str, CentralTabView* centralTabWidget) : mStruct{&str}, mTabs{centralTabWidget} {
	makeGUI();
}

void StructEdit::makeGUI() {
	
	// destroy the previous layout
	if (auto oldLay = layout()) {
		deleteLayout(oldLay);
	}
	
	auto layout = new QGridLayout;
	setLayout(layout);
	
	
	auto id = 0;
	for (const auto& namedType : mStruct->types()) {
		
		auto edit = new QLineEdit;
		edit->setText(QString::fromStdString(namedType.name));
		
		connect(edit, &QLineEdit::textChanged, this, [this, id](const QString& newText) {
			mStruct->modifyType(id, mStruct->types()[id].type, newText.toStdString());
			refreshReferences();
		});
		layout->addWidget(edit, id, 0);

		
		auto tySelector = new TypeSelector(mStruct->module());
		tySelector->setCurrentType(namedType.type);
		connect(tySelector, &TypeSelector::typeSelected, this,
		        [this, id](const chi::DataType& newType) {

			        if (!newType.valid()) { return; }

					mStruct->modifyType(id, newType, mStruct->types()[id].name);
					
					refreshReferences();
				
			    });
		layout->addWidget(tySelector, id, 1, Qt::AlignTop);

		
		auto deleteButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), {});
		connect(deleteButton, &QAbstractButton::clicked, this, [this, id](bool) {
			mStruct->removeType(id);
			refreshReferences();
			
			
			makeGUI();
		});
		layout->addWidget(deleteButton, id, 2);

		++id;
	}

	// create the "new" button
	auto newButton = new QPushButton(QIcon::fromTheme("list-add"), {});
	newButton->setSizePolicy({QSizePolicy::Maximum, QSizePolicy::Maximum});
	connect(newButton, &QAbstractButton::clicked, this, [this](bool) {
		mStruct->addType(mStruct->context().langModule()->typeFromName("i32"), {}, mStruct->types().size());
		
		refreshReferences();
		
		makeGUI();
	});
	layout->addWidget(newButton, id, 1, Qt::AlignRight);
}

void StructEdit::refreshReferences()
{
	auto& ctx = mStruct->context();
	
	auto allInstances = ctx.findInstancesOfType(mStruct->module().fullNamePath(), "_make_" + mStruct->name());
	auto breakInstances = ctx.findInstancesOfType(mStruct->module().fullNamePath(), "_break_" + mStruct->name());
	
	allInstances.reserve(allInstances.size() + breakInstances.size());
	std::copy(breakInstances.begin(), breakInstances.end(), std::back_inserter(allInstances));
	
	// just refresh the gui
	for (auto inst : allInstances) {
		auto view = mTabs->viewFromFunction(inst->function());
		
		if (view != nullptr) {
			view->refreshGuiForNode(*inst);
		}
	}
}
