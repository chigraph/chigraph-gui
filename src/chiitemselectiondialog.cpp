#include "chiitemselectiondialog.hpp"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTreeView>
#include <QVBoxLayout>

#include <cassert>

ChiItemSelectionDialog::ChiItemSelectionDialog(chi::Context& ctx, boost::filesystem::path* toFill,
                                               WorkspaceTree::eType type, QWidget* parent)
    : QDialog{parent}, mType{type}, mToFill{toFill} {
	assert(toFill != nullptr);

	auto layout = new QVBoxLayout;
	setLayout(layout);

	mTreeView = new QTreeView;
	layout->addWidget(mTreeView);

	auto filter = [&] {
		switch (mType) {
		case WorkspaceTree::MODULE: return ModuleTreeModel::Modules;
		case WorkspaceTree::STRUCT: return ModuleTreeModel::Structs;
		case WorkspaceTree::FUNCTION: return ModuleTreeModel::Functions;
		case WorkspaceTree::FOLDER: return ModuleTreeModel::Folders;
		}
		// panic as a defult
		Q_ASSERT(false);
		return ModuleTreeModel::All;
	}();

	// get model and set it
	mModel = ModuleTreeModel::createFromContext(ctx, filter);
	mTreeView->setModel(mModel.get());

	mTreeView->setAnimated(true);
	mTreeView->setSortingEnabled(true);
	mTreeView->header()->close();

	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, this,
	        [this] { tryAccept(mTreeView->currentIndex()); });

	layout->addWidget(buttonBox);

	connect(mTreeView, &QTreeView::doubleClicked, this, &ChiItemSelectionDialog::tryAccept);
}

void ChiItemSelectionDialog::setCurrentItem(const boost::filesystem::path& newItem) {
	auto index = mModel->indexFromName(newItem, mType);
	if (index.isValid()) {
		mTreeView->scrollTo(index);
		mTreeView->setCurrentIndex(index);
	}
}

boost::filesystem::path ChiItemSelectionDialog::getItem(
    QWidget* parent, chi::Context& ctx, const QString& title, WorkspaceTree::eType type,
    const boost::filesystem::path& startWithSelection) {
	boost::filesystem::path ret;
	auto                    dialog = new ChiItemSelectionDialog(ctx, &ret, type, parent);
	dialog->setWindowTitle(title);
	dialog->setCurrentItem(startWithSelection);

	dialog->exec();

	return ret;
}

void ChiItemSelectionDialog::tryAccept(const QModelIndex& index) {
	if (!index.isValid()) { return; }

	auto item = static_cast<WorkspaceTree*>(index.internalPointer());

	// make sure it's a folder and it isn't the src folder
	if (mType == item->type && item->parent->parent != nullptr) {
		*mToFill = item->fullName();
		accept();
	}
}
