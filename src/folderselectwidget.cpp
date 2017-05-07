#include "folderselectwidget.hpp"

#include "folderselectiondialog.hpp"

FolderSelectWidget::FolderSelectWidget(chi::Context& ctx)
{
	connect(this, &QPushButton::clicked, this, [this, &ctx](bool) {
		
		auto folderName = FolderSelectionDialog::getFolder(this, ctx);
		
		if (folderName.empty()) {
			return;
		}
		
		setFolder(folderName);
		
	});
	
	setIcon(QIcon::fromTheme(QStringLiteral("stock_folder")));
}

boost::filesystem::path FolderSelectWidget::folder() const
{
	return text().toStdString();
}


void FolderSelectWidget::setFolder(const boost::filesystem::path& newFolder)
{
	setText(QString::fromStdString(newFolder.string()));
	
	emit folderChanged(newFolder);
}


