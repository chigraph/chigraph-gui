#include "moduleselectwidget.hpp"

#include "moduleselectiondialog.hpp"

ModuleSelectWidget::ModuleSelectWidget(chi::Context& ctx)
{
	connect(this, &QPushButton::clicked, this, [this, &ctx](bool) {
		
		auto modName = ModuleSelectionDialog::getModule(this, ctx);
		
		if (modName.empty()) {
			return;
		}
		
		setModule(modName);
		
	});
	
	setIcon(QIcon::fromTheme(QStringLiteral("package-available")));
}

boost::filesystem::path ModuleSelectWidget::module() const
{
	return text().toStdString();
}

void ModuleSelectWidget::setModule(const boost::filesystem::path& newModule)
{
	setText(QString::fromStdString(newModule.string()));
	
	emit moduleChanged(newModule);
}

