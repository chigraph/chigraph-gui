#include "folderselectiondialog.hpp"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTreeView>

#include <cassert>

FolderSelectionDialog::FolderSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill, QWidget* parent) : QDialog{parent}
{
	assert(toFill != nullptr);
	
	auto layout = new QVBoxLayout;
	setLayout(layout);
	
	auto treeView = new QTreeView;
	layout->addWidget(treeView);
	
	// get model and set it
	mModel = ModuleTreeModel::createFromContext(ctx, ModuleTreeModel::Folders);
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
		
		// make sure it's a folder and it isn't the src folder
		if (item->type == WorkspaceTree::FOLDER && item->parent->parent != nullptr) {
			*toFill = item->fullName();
			accept();
		}
	
	});
}


boost::filesystem::path FolderSelectionDialog::getFolder(QWidget* parent, chi::Context& ctx)
{
	boost::filesystem::path ret;
	auto dialog = new FolderSelectionDialog(ctx, &ret, parent);
	
	dialog->exec();
	
	
	return ret;
}


