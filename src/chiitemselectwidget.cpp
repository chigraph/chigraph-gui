#include "chiitemselectwidget.hpp"

#include "chiitemselectiondialog.hpp"

#include <KLocalizedString>

ChiItemSelectWidget::ChiItemSelectWidget(chi::Context& ctx, WorkspaceTree::eType type) {
	connect(this, &QPushButton::clicked, this, [this, &ctx, type](bool) {

		auto modName =
		    ChiItemSelectionDialog::getItem(this, ctx, i18n("Select Item"), type, item());

		if (modName.empty()) { return; }

		setItem(modName);

	});

	setIcon(ModuleTreeModel::iconForItemType(type));
}

boost::filesystem::path ChiItemSelectWidget::item() const { return mData; }

void ChiItemSelectWidget::setItem(const boost::filesystem::path& newItem) {
	setText(QString::fromStdString(newItem.string()));
	mData = newItem;

	emit itemChanged(newItem);
}
