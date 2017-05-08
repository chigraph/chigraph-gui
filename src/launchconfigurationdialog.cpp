#include "launchconfigurationdialog.hpp"
#include "chiitemselectwidget.hpp"

#include <KLocalizedString>

#include <QAction>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

LaunchConfigurationDialog::LaunchConfigurationDialog(LaunchConfigurationManager& manager)
    : mManager{&manager} {
	setWindowTitle(i18n("Launch Configurations"));

	// left side widget
	auto leftWidget = new QWidget;
	{
		auto layout = new QVBoxLayout;
		leftWidget->setLayout(layout);

		// new button
		auto newButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")),
		                                 i18n("New Configuration"));
		layout->addWidget(newButton);
		connect(newButton, &QPushButton::pressed, this, &LaunchConfigurationDialog::addNewConfig);

		// config list
		mConfigList = new QListWidget;
		mConfigList->setEditTriggers(QAbstractItemView::SelectedClicked);
		layout->addWidget(mConfigList);

		// populate it
		for (const auto& config : manager.configurations()) {
			auto listItem = new QListWidgetItem(config.name());

			// make it editable
			listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);

			mConfigList->addItem(listItem);
		}
		connect(mConfigList, &QListWidget::currentItemChanged, this,
		        [this](QListWidgetItem* item, QListWidgetItem*) { selectConfig(item->text()); });
		connect(mConfigList, &QListWidget::itemChanged, this,
		        [this](QListWidgetItem* item) { nameChanged(item->text()); });

		auto renameAction =
		    new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"), nullptr);
		renameAction->setShortcut(Qt::Key_F2);
		connect(renameAction, &QAction::triggered, this,
		        [this] { mConfigList->openPersistentEditor(mConfigList->currentItem()); });

		auto deleteAction =
		    new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete"), nullptr);
		deleteAction->setShortcut(Qt::Key_Delete);
		connect(deleteAction, &QAction::triggered, this, [this] {
			auto currentItem = mConfigList->currentItem();
			if (currentItem->text() == currentlyEditing.name()) {
				// if this is the case, then clear our fields and currentlyEditing
				currentlyEditing = {};

				mModuleEdit->setItem({});
				mWdEdit->setText({});
				mArgsEdit->setText({});
			}
		});

		addAction(renameAction);
		addAction(deleteAction);

		// context menus
		mConfigList->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(mConfigList, &QWidget::customContextMenuRequested, this,
		        [this, renameAction, deleteAction](const QPoint& point) {
			        auto item = mConfigList->itemAt(point);

			        mConfigList->setCurrentItem(item);

			        if (item == nullptr) { return; }

			        QMenu menu;
			        menu.addAction(renameAction);
			        menu.addAction(deleteAction);
			    });
	}

	// right side widget
	auto rightWidget = new QWidget;
	{
		auto layout = new QFormLayout;
		rightWidget->setLayout(layout);

		// module
		{
			mModuleEdit = new ChiItemSelectWidget(manager.context(), WorkspaceTree::MODULE);
			connect(mModuleEdit, &ChiItemSelectWidget::itemChanged, this,
			        &LaunchConfigurationDialog::moduleChanged);
			layout->addRow(i18n("Module"), mModuleEdit);
		}

		// working directory
		{
			mWdEdit = new QLineEdit;
			connect(mWdEdit, &QLineEdit::textChanged, this, &LaunchConfigurationDialog::wdChanged);
			layout->addRow(i18n("Working Directory"), mWdEdit);
		}

		// arguments
		{
			mArgsEdit = new QLineEdit;
			connect(mArgsEdit, &QLineEdit::textChanged, this,
			        &LaunchConfigurationDialog::argsChanged);
			layout->addRow(i18n("Arguments"), mArgsEdit);
		}
	}

	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

	auto rootSlider = new QSplitter;
	rootSlider->addWidget(leftWidget);
	rootSlider->addWidget(rightWidget);

	auto rootLayout = new QVBoxLayout;
	setLayout(rootLayout);
	rootLayout->addWidget(rootSlider);
	rootLayout->addWidget(buttonBox);

	// select the first one
	if (mManager->configurations().size() > 1) {
		selectConfig(mManager->configurations()[0].name());
	}
}

void LaunchConfigurationDialog::selectConfig(const QString& newConfig) {
	// find it
	LaunchConfiguration config;
	for (const auto& conf : mManager->configurations()) {
		if (conf.name() == newConfig) {
			config = conf;
			break;
		}
	}
	if (!config.valid()) { return; }

	currentlyEditing = config;

	mWdEdit->setText(config.workingDirectory());
	mModuleEdit->setItem(config.module().toStdString());
	mArgsEdit->setText(config.arguments());
}

void LaunchConfigurationDialog::addNewConfig() {
	auto config = mManager->newConfiguration();
	config.setName(i18n("New Configuration"));

	auto newItem = new QListWidgetItem(config.name());

	// make it editable
	newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

	mConfigList->addItem(newItem);

	selectConfig(config.name());
}

void LaunchConfigurationDialog::argsChanged(const QString& newArgs) {
	if (currentlyEditing.valid()) { currentlyEditing.setArguments(newArgs); }
}

void LaunchConfigurationDialog::moduleChanged(const boost::filesystem::path& newModule) {
	if (currentlyEditing.valid()) {
		currentlyEditing.setModule(QString::fromStdString(newModule.string()));
	}
}

void LaunchConfigurationDialog::nameChanged(const QString& newName) {
	if (currentlyEditing.valid()) { currentlyEditing.setName(newName); }
}

void LaunchConfigurationDialog::wdChanged(const QString& newWd) {
	if (currentlyEditing.valid()) { currentlyEditing.setWorkingDirectory(newWd); }
}
