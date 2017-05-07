#include "moduleselectiondialog.hpp"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTreeView>

#include <cassert>

ModuleSelectionDialog::ModuleSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill, QWidget* parent) : QDialog{parent}
{
	assert(toFill != nullptr);
	
	auto layout = new QVBoxLayout;
	setLayout(layout);
	
	auto treeView = new QTreeView;
	layout->addWidget(treeView);
	
	// get model and set it
	mModel = ModuleTreeModel::createFromContext(ctx, ModuleTreeModel::Modules);
	treeView->setModel(mModel.get());
	
	treeView->setAnimated(true);
	treeView->setSortingEnabled(true);
	treeView->header()->close();
	
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	
	layout->addWidget(buttonBox);

	
	connect(treeView, &QTreeView::doubleClicked, this, [toFill, this](const QModelIndex& index) {
		if (!index.isValid()) {
			return;
		}

		auto item  = static_cast<WorkspaceTree*>(index.internalPointer());
		
		if (item->type == WorkspaceTree::MODULE) {
			*toFill = item->fullName();
			accept();
		}
	
	});
}


boost::filesystem::path ModuleSelectionDialog::getModule(QWidget* parent, chi::Context& ctx)
{
	boost::filesystem::path ret;
	auto dialog = new ModuleSelectionDialog(ctx, &ret, parent);
	
	dialog->exec();
	
	
	return ret;
}

