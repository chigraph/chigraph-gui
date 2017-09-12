#include "launchconfigurationdialog.hpp"
#include "chiitemselectwidget.hpp"

#include <KLocalizedString>
#include <KUrlRequester>

#include <QAction>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

LaunchConfigurationDialog::LaunchConfigurationDialog(LaunchConfigurationManager& manager)
    : mManager{&manager} {
	setWindowTitle(i18n("Launch Configurations"));

	// config list
	mConfigList = new QListWidget;
	mConfigList->setEditTriggers(QAbstractItemView::SelectedClicked |
	                             QAbstractItemView::EditTrigger::EditKeyPressed);

	// init actions
	auto renameAction =
	    new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"), nullptr);
	connect(renameAction, &QAction::triggered, this,
	        [this] { mConfigList->openPersistentEditor(mConfigList->currentItem()); });

	auto deleteAction =
	    new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete"), nullptr);
	deleteAction->setShortcut(Qt::Key_Delete);
	connect(deleteAction, &QAction::triggered, this, [this] {
		auto currentItem = mConfigList->currentItem();
		if (idToItem[currentItem] == currentlyEditing) {
			// if this is the case, then clear our fields and currentlyEditing
			currentlyEditing = {};

			mModuleEdit->setItem({});
			mWdEdit->setText({});
			mArgsEdit->setText({});
		}

		// remove it from the manager
		mManager->removeConfiguration(idToItem[currentItem]);

		// delete it
		delete mConfigList->takeItem(mConfigList->row(currentItem));

	});

	addAction(renameAction);
	addAction(deleteAction);

	// left side widget
	auto leftWidget = new QWidget;
	{
		auto layout = new QVBoxLayout;
		leftWidget->setLayout(layout);

		// buttons
		auto buttonWidget = new QWidget;
		{
			auto hbuttonlayout = new QHBoxLayout;
			buttonWidget->setLayout(hbuttonlayout);
			hbuttonlayout->setAlignment(Qt::AlignRight);

			// new button
			auto newButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), {});
			newButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
			connect(newButton, &QPushButton::pressed, this,
			        &LaunchConfigurationDialog::addNewConfig);
			hbuttonlayout->addWidget(newButton);

			// delete button
			auto deleteButton =
			    new QPushButton(QIcon::fromTheme(QStringLiteral("edit-delete")), {});
			deleteButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
			connect(deleteButton, &QPushButton::pressed, deleteAction, &QAction::trigger);
			hbuttonlayout->addWidget(deleteButton);

			hbuttonlayout->addWidget(deleteButton);
		}

		layout->addWidget(buttonWidget);

		layout->addWidget(mConfigList);

		// populate it
		for (const auto& config : manager.configurations()) {
			auto listItem = new QListWidgetItem(config.name());

			// make it editable
			listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);

			mConfigList->addItem(listItem);

			// store it in the map
			idToItem[listItem] = config;
		}
		connect(mConfigList, &QListWidget::currentItemChanged, this,
		        [this](QListWidgetItem* item, QListWidgetItem*) { selectConfig(item); });
		connect(mConfigList, &QListWidget::itemChanged, this,
		        [this](QListWidgetItem* item) { nameChanged(item->text()); });

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

			        menu.exec(mConfigList->mapToGlobal(point));
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
			mWdEdit = new KUrlRequester();
			// mWdEdit->setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
			mWdEdit->setMode(KFile::Directory);
			connect(mWdEdit, &KUrlRequester::textChanged, this,
			        &LaunchConfigurationDialog::wdChanged);
			layout->addRow(i18n("Working Directory"), mWdEdit);
		}

		// arguments
		{
			mArgsEdit = new QLineEdit;
			mArgsEdit->setPlaceholderText(i18n("Enter arguments to be passed to the executable"));
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
	if (mManager->configurations().size() > 1) { selectConfig(mManager->configurations()[0]); }
}

void LaunchConfigurationDialog::selectConfig(QListWidgetItem* newItem) {
	// find it
	auto iter = idToItem.find(newItem);
	if (iter == idToItem.end()) { return; }

	selectConfig(iter->second);
}

void LaunchConfigurationDialog::selectConfig(LaunchConfiguration config) {
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

	idToItem[newItem] = config;

	selectConfig(config);
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
