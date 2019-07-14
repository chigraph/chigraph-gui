#include "newmoduledialog.hpp"

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

#include <chi/Context.hpp>
#include <chi/GraphModule.hpp>
#include <chi/Support/Result.hpp>

#include "chiitemselectwidget.hpp"
#include "mainwindow.hpp"

NewModuleDialog::NewModuleDialog(QWidget* parent, chi::Context& context,
                                 const std::filesystem::path& folder)
    : QDialog(parent) {
	setWindowTitle(i18n("New Module"));

	auto rootLayout = new QVBoxLayout;
	setLayout(rootLayout);

	auto proxyWidget = new QWidget;

	auto layout = new QFormLayout();
	proxyWidget->setLayout(layout);

	mFolderWidget = new ChiItemSelectWidget(context, WorkspaceTree::FOLDER);
	mFolderWidget->setItem(folder);
	mNameEdit = new QLineEdit();

	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
	connect(buttonBox, &QDialogButtonBox::accepted, this, [this, &context] {
		if (mNameEdit->text().isEmpty()) { return; }

		// create the new module
		auto fullPath = mFolderWidget->item() / mNameEdit->text().toStdString();

		auto mod = context.newGraphModule(fullPath);
		mod->addDependency("lang");

		mod->saveToDisk();

		MainWindow::instance()->newModuleCreated(*mod);

		accept();
	});
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	layout->addRow(i18n("Create In"), mFolderWidget);
	layout->addRow(i18n("Name"), mNameEdit);

	rootLayout->addWidget(proxyWidget);
	rootLayout->addWidget(buttonBox);
}
