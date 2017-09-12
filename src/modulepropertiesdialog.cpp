#include "modulepropertiesdialog.hpp"
#include "chiitemselectiondialog.hpp"

#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

#include <chi/GraphModule.hpp>
#include <chi/Support/Result.hpp>

ModulePropertiesDialog::ModulePropertiesDialog(QWidget* parent, chi::GraphModule& modToEdit)
    : QDialog{parent}, mModule{&modToEdit} {
	setWindowTitle(QString::fromStdString(modToEdit.fullName()) + " - " + i18n("Properties"));

	auto rootLayout = new QVBoxLayout{};
	setLayout(rootLayout);

	// dependencies pane
	auto depsList = new QListWidget;
	rootLayout->addWidget(depsList);

	// populate it
	for (const auto& dep : modToEdit.dependencies()) {
		depsList->addItem(QString::fromStdString(dep.string()));
	}

	// create new button
	auto newDepButton =
	    new QPushButton{QIcon::fromTheme(QStringLiteral("gtk-add")), i18n("New Dependency")};
	rootLayout->addWidget(newDepButton);
	connect(newDepButton, &QPushButton::clicked, this, [this, depsList](bool) {
		// open a module selection dialog

		boost::filesystem::path newDep = ChiItemSelectionDialog::getItem(
		    this, mModule->context(), i18n("Select new dependency"), WorkspaceTree::MODULE);

		if (newDep.empty()) { return; }

		if (mModule->dependencies().count(newDep) != 0) { return; }

		// add it to the module
		auto res = mModule->addDependency(newDep);

		if (!res) {
			KMessageBox::detailedError(this, i18n("Failed to add dependency for module"),
			                           QString::fromStdString(res.dump()));

			return;
		}

		// add it to the list
		depsList->addItem(QString::fromStdString(newDep.string()));

	});

	// the C support button
	auto checkBox = new QCheckBox(i18n("C Support"));
	rootLayout->addWidget(checkBox);

	connect(checkBox, &QCheckBox::stateChanged, this, [this](int newState) {
		switch (newState) {
		case Qt::Unchecked: mModule->setCEnabled(false); return;
		case Qt::Checked: mModule->setCEnabled(true); return;
		default: return;
		}
	});
	checkBox->setCheckState(mModule->cEnabled() ? Qt::Checked : Qt::Unchecked);
}
