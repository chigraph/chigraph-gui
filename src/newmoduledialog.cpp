#include "newmoduledialog.hpp"

#include <KLocalizedString>

#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <chi/Context.hpp>
#include <chi/GraphModule.hpp>
#include <chi/Result.hpp>

#include "folderselectwidget.hpp"
#include "mainwindow.hpp"

NewModuleDialog::NewModuleDialog(QWidget* parent, chi::Context& context, const boost::filesystem::path& folder) : QDialog(parent) {
	
	setWindowTitle(i18n("New Module"));
	
	auto rootLayout = new QVBoxLayout;
	setLayout(rootLayout);
	
	auto proxyWidget = new QWidget;
	
	auto layout = new QFormLayout();
	proxyWidget->setLayout(layout);
	
	mFolderWidget = new FolderSelectWidget(context);
	mNameEdit = new QLineEdit();
	
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
	connect(buttonBox, &QDialogButtonBox::accepted, this, [this, &context]{
		if (mFolderWidget->folder().empty() || mNameEdit->text().isEmpty()) {
			return;
		}
		
		// create the new module
		auto fullPath = mFolderWidget->folder() / mNameEdit->text().toStdString();
		
		auto mod = context.newGraphModule(fullPath);
		mod->addDependency("lang");
		
		mod->saveToDisk();
		
		MainWindow::instance()->newModuleCreated(*mod);
		
		accept();
	});
	
	layout->addRow(i18n("Create In"), mFolderWidget);
	layout->addRow(i18n("Name"), mNameEdit);
	
	rootLayout->addWidget(proxyWidget);
	rootLayout->addWidget(buttonBox);
	
}



