#include "chigraphnodemodel.hpp"

#include <chi/ChiModule.hpp>
#include <chi/Context.hpp>
#include <chi/DataType.hpp>
#include <chi/FunctionValidator.hpp>
#include <chi/GraphFunction.hpp>
#include <chi/GraphModule.hpp>
#include <chi/NodeInstance.hpp>
#include <chi/Support/Result.hpp>

#include <nodes/NodeData>

#include <KActionCollection>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <boost/range/join.hpp>

namespace {

class EditCodeDialog : public QDialog {
public:
	EditCodeDialog(chi::NodeInstance* inst, ChigraphFlowSceneModel* fview) {
		setWindowTitle(i18n("Edit C Call Node"));

		auto layout = new QVBoxLayout;
		setLayout(layout);

		auto lineEdit = new QLineEdit;
		layout->addWidget(lineEdit);
		lineEdit->setText(QString::fromStdString(inst->type().toJSON()["function"]));

		// get KTextEditor stuff
		KTextEditor::Editor* editor = KTextEditor::Editor::instance();
		// create a new document
		KTextEditor::Document* doc = editor->createDocument(this);
		doc->setText(QString::fromStdString(inst->type().toJSON()["code"]));
		doc->setHighlightingMode("C");

		// create a widget to display the document
		KTextEditor::View* textEdit = doc->createView(nullptr);
		// delete save and saveAs actions
		{
			for (const auto& action : textEdit->actionCollection()->actions()) {
				QString name = action->text();
			}
			auto saveAction = textEdit->actionCollection()->action("file_save");
			textEdit->actionCollection()->removeAction(saveAction);
			auto saveAsAction = textEdit->actionCollection()->action("file_save_as");
			textEdit->actionCollection()->removeAction(saveAsAction);
		}
		textEdit->setMinimumHeight(200);
		layout->addWidget(textEdit);

		auto okButton = new QPushButton;
		layout->addWidget(okButton);
		okButton->setText(i18n("Ok"));
		connect(okButton, &QPushButton::clicked, this, [this, doc, lineEdit, inst, fview] {
			std::string function = lineEdit->text().toStdString();
			std::string code     = doc->text().toStdString();

			std::unique_ptr<chi::NodeType> ty;
			auto res = static_cast<chi::GraphModule&>(inst->type().module())
			               .createNodeTypeFromCCode(code, function, {}, &ty);
			if (!res) {
				KMessageBox::detailedError(this, "Failed to compile C node",
				                           QString::fromStdString(res.dump()));

				return;
			}
			inst->setType(std::move(ty));

			close();

		});

		connect(this, &QDialog::accepted, this,
		        [fview, inst] { emit fview->nodePortUpdated(fview->nodeIndex(*inst)); });
	}
};

}  // namespace

QWidget* ChigraphFlowSceneModel::createEmbeddedWidget(chi::NodeInstance& inst) {
	if (inst.type().name() == "const-bool") {
		QCheckBox* box     = new QCheckBox("");
		bool       checked = inst.type().toJSON();
		box->setCheckState(checked ? Qt::Checked : Qt::Unchecked);

		connect(box, &QCheckBox::stateChanged, this, [this, &inst](int newState) {
			std::unique_ptr<chi::NodeType> newType;

			inst.context().nodeTypeFromModule("lang", "const-bool", newState == Qt::Checked,
			                                  &newType);

			inst.setType(std::move(newType));
		});

		box->setMaximumSize(box->sizeHint());
		return box;
	}
	if (inst.type().name() == "strliteral") {
		auto        edit = new QLineEdit();
		std::string s    = inst.type().toJSON();
		edit->setText(QString::fromStdString(s));

		edit->setMaximumSize(edit->sizeHint());

		connect(edit, &QLineEdit::textChanged, this, [this, &inst](const QString& s) {
			std::unique_ptr<chi::NodeType> newType;

			inst.context().nodeTypeFromModule("lang", "strliteral", s.toUtf8().constData(),
			                                  &newType);

			inst.setType(std::move(newType));

		});

		return edit;
	}
	if (inst.type().name() == "const-int") {
		auto edit = new QLineEdit();
		edit->setValidator(new QIntValidator);
		int val = inst.type().toJSON();
		edit->setText(QString::number(val));

		edit->setMaximumSize(edit->sizeHint());

		connect(edit, &QLineEdit::textChanged, this, [this, &inst](const QString& s) {
			std::unique_ptr<chi::NodeType> newType;

			inst.context().nodeTypeFromModule("lang", "const-int", s.toInt(), &newType);

			inst.setType(std::move(newType));

		});

		return edit;
	}
	if (inst.type().name() == "c-call") {
		QPushButton* butt = new QPushButton(i18n("Edit code"));
		connect(butt, &QPushButton::clicked, this, [this, &inst] {
			auto dialog = new EditCodeDialog(&inst, this);

			dialog->exec();
		});

		butt->setMaximumSize(butt->sizeHint());

		return butt;
	}
	if (inst.type().name() == "const-float") {
		auto edit = new QLineEdit();
		edit->setValidator(new QDoubleValidator);
		double val = inst.type().toJSON();
		edit->setText(QString::number(val));

		edit->setMaximumSize(edit->sizeHint());

		connect(edit, &QLineEdit::textChanged, this, [this, &inst](const QString& s) {
			std::unique_ptr<chi::NodeType> newType;

			inst.context().nodeTypeFromModule("lang", "const-float", s.toDouble(), &newType);

			inst.setType(std::move(newType));
		});

		return edit;
	}
	return nullptr;
}

ChigraphFlowSceneModel::ChigraphFlowSceneModel(chi::GraphFunction& func) : mFunc{&func} {
	// create embedded widgets

	for (const auto& node : mFunc->nodes()) {
		auto& inst = *node.second;

		auto widget = createEmbeddedWidget(inst);

		if (widget != nullptr) { mEmbeddedWidgets[&inst] = widget; }
	}
}

void ChigraphFlowSceneModel::updateValidation() {
	auto res = chi::validateFunction(*mFunc);

	auto old    = mValidation;
	mValidation = std::move(res);

	// see who's changed
	for (const auto& inst : boost::range::join(old.result_json, mValidation.result_json)) {
		if (inst["data"].find("Node ID") != inst["data"].end() &&
		    inst["data"]["Node ID"].is_string()) {
			std::string nodeID = inst["data"]["Node ID"];
			auto        iter   = mFunc->nodes().find(boost::uuids::string_generator()(nodeID));
			if (iter != mFunc->nodes().end()) {
				emit nodeValidationUpdated(nodeIndex(*iter->second));
			}
		}
	}
}

QStringList ChigraphFlowSceneModel::modelRegistry() const {
	QStringList ret;

	auto& mod = mFunc->module();
	auto& ctx = mFunc->context();

	// add dependencies
	for (const auto& dep : mod.dependencies()) {
		auto depMod = ctx.moduleByFullName(dep);

		Q_ASSERT(depMod != nullptr && "Failed to get dependency");

		for (const auto& nodeType : depMod->nodeTypeNames()) {
			ret << QString::fromStdString(dep.string() + ":" + nodeType);
		}
	}

	for (const auto& nodeType : mod.nodeTypeNames()) {
		ret << QString::fromStdString(mod.fullName() + ":" + nodeType);
	}

	// add local variables
	for (const auto& var : mFunc->localVariables()) {
		ret << QString::fromStdString(mod.fullName() + ":_get_" + var.name);
		ret << QString::fromStdString(mod.fullName() + ":_set_" + var.name);
	}

	return ret;
}

QString ChigraphFlowSceneModel::nodeTypeCategory(QString const& name) const {
	// everyting before the :
	return name.mid(0, name.lastIndexOf(':'));
}
QString ChigraphFlowSceneModel::converterNode(QtNodes::NodeDataType const& lhs,
                                              QtNodes::NodeDataType const& rhs) const {
	if (lhs.id == "_exec" || rhs.id == "_exec") { return {}; }

	// parse the types
	std::string lModule, lTypeName, rModule, rTypeName;
	std::tie(lModule, lTypeName) = chi::parseColonPair(lhs.id.toStdString());
	std::tie(rModule, rTypeName) = chi::parseColonPair(rhs.id.toStdString());

	// get the types
	auto lType = mFunc->context().moduleByFullName(lModule)->typeFromName(lTypeName);
	auto rType = mFunc->context().moduleByFullName(rModule)->typeFromName(rTypeName);

	if (!lType.valid() || !rType.valid()) { return {}; }

	auto converter = mFunc->context().createConverterNodeType(lType, rType);
	if (converter == nullptr) { return {}; }
	return QString::fromStdString(converter->qualifiedName());
}

QList<QUuid> ChigraphFlowSceneModel::nodeUUids() const {
	QList<QUuid> ret;

	for (const auto& node : mFunc->nodes()) {
		ret << QUuid::fromRfc4122(
		    QByteArray(reinterpret_cast<const char*>(node.second->id().data), 16));
	}

	return ret;
}
QtNodes::NodeIndex ChigraphFlowSceneModel::nodeIndex(const QUuid& ID) const {
	Q_ASSERT(!ID.isNull());

	// create a boost::uuid
	boost::uuids::uuid uuid;

	auto QuuidByteArray = ID.toRfc4122();

	std::copy(QuuidByteArray.begin(), QuuidByteArray.end(), uuid.begin());

	// find the node
	auto iter = mFunc->nodes().find(uuid);
	if (iter == mFunc->nodes().end()) { return {}; }

	return createIndex(ID, iter->second.get());
}
QString ChigraphFlowSceneModel::nodeTypeIdentifier(QtNodes::NodeIndex const& index) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	return QString::fromStdString(inst->type().qualifiedName());
}
QString ChigraphFlowSceneModel::nodeCaption(QtNodes::NodeIndex const& index) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	return QString::fromStdString(inst->type().description());
}
QPointF ChigraphFlowSceneModel::nodeLocation(QtNodes::NodeIndex const& index) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	return QPointF{inst->x(), inst->y()};
}
QWidget* ChigraphFlowSceneModel::nodeWidget(QtNodes::NodeIndex const& index) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	auto iter = mEmbeddedWidgets.find(inst);

	if (iter == mEmbeddedWidgets.end()) { return nullptr; }
	return iter->second;
}
bool ChigraphFlowSceneModel::nodeResizable(QtNodes::NodeIndex const& index) const {
	return false;  // TODO: is this right?
}
QtNodes::NodeValidationState ChigraphFlowSceneModel::nodeValidationState(
    QtNodes::NodeIndex const& index) const {
	Q_ASSERT(index.isValid());

	// see if this node is in the JSON
	for (const auto& entry : mValidation.result_json) {
		if (entry["data"].find("Node ID") != entry["data"].end() &&
		    entry["data"]["Node ID"].is_string()) {
			auto Uuid = QUuid(QString::fromStdString(entry["data"]["Node ID"]));

			if (Uuid == index.id()) {
				std::string ec = entry["errorcode"];
				if (ec[0] == 'E') { return QtNodes::NodeValidationState::Error; }
				return QtNodes::NodeValidationState::Warning;
			}
		}
	}

	return QtNodes::NodeValidationState::Valid;
}

/// Get the validation error/warning
QString ChigraphFlowSceneModel::nodeValidationMessage(QtNodes::NodeIndex const& index) const {
	// see if this node is in the JSON
	for (const auto& entry : mValidation.result_json) {
		if (entry["data"].find("Node ID") != entry["data"].end() &&
		    entry["data"]["Node ID"].is_string()) {
			auto Uuid = QUuid(QString::fromStdString(entry["data"]["Node ID"]));

			if (Uuid == index.id()) { return QString::fromStdString(entry["overview"]); }
		}
	}

	return {};
}

/// Get the painter delegate
QtNodes::NodePainterDelegate* ChigraphFlowSceneModel::nodePainterDelegate(
    QtNodes::NodeIndex const& index) const {
	return nullptr;
}

/// Get the count of DataPorts
unsigned int ChigraphFlowSceneModel::nodePortCount(QtNodes::NodeIndex const& index,
                                                   QtNodes::PortType         portType) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	if (portType == QtNodes::PortType::In) {
		return inst->inputExecConnections.size() + inst->inputDataConnections.size();
	}
	return inst->outputExecConnections.size() + inst->outputDataConnections.size();
}

/// Get the port caption
QString ChigraphFlowSceneModel::nodePortCaption(QtNodes::NodeIndex const& index,
                                                QtNodes::PortType         portType,
                                                QtNodes::PortIndex        portID) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	// make the casts safe
	Q_ASSERT(portID >= 0);

	if (portType == QtNodes::PortType::In) {
		return QString::fromStdString(
		    (size_t)portID < inst->inputExecConnections.size()
		        ? inst->type().execInputs()[portID]
		        : inst->type().dataInputs()[portID - inst->inputExecConnections.size()].name);
	}
	return QString::fromStdString(
	    (size_t)portID < inst->outputExecConnections.size()
	        ? inst->type().execOutputs()[portID]
	        : inst->type().dataOutputs()[portID - inst->outputExecConnections.size()].name);
}

/// Get the port data type
QtNodes::NodeDataType ChigraphFlowSceneModel::nodePortDataType(QtNodes::NodeIndex const& index,
                                                               QtNodes::PortType         portType,
                                                               QtNodes::PortIndex portID) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	// make casts safe
	Q_ASSERT(portID >= 0);

	if (portType == QtNodes::PortType::In) {
		if ((size_t)portID < inst->inputExecConnections.size()) {
			return QtNodes::NodeDataType{"_exec", ""};
		}
		auto typeName =
		    QString::fromStdString(inst->type()
		                               .dataInputs()[portID - inst->inputExecConnections.size()]
		                               .type.qualifiedName());
		return QtNodes::NodeDataType{typeName, typeName};
	}
	if ((size_t)portID < inst->outputExecConnections.size()) {
		return QtNodes::NodeDataType{"_exec", ""};
	}
	auto typeName =
	    QString::fromStdString(inst->type()
	                               .dataOutputs()[portID - inst->outputExecConnections.size()]
	                               .type.qualifiedName());
	return QtNodes::NodeDataType{typeName, typeName};
}

/// Port Policy
QtNodes::ConnectionPolicy ChigraphFlowSceneModel::nodePortConnectionPolicy(
    QtNodes::NodeIndex const& index, QtNodes::PortType portType, QtNodes::PortIndex portID) const {
	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	// make casts safe
	Q_ASSERT(portID >= 0);

	if (portType == QtNodes::PortType::In) {
		return (size_t)portID < inst->inputExecConnections.size() ? QtNodes::ConnectionPolicy::Many
		                                                          : QtNodes::ConnectionPolicy::One;
	}
	return (size_t)portID < inst->outputExecConnections.size() ? QtNodes::ConnectionPolicy::One
	                                                           : QtNodes::ConnectionPolicy::Many;
}

/// Get a connection at a port
std::vector<std::pair<QtNodes::NodeIndex, QtNodes::PortIndex>>
ChigraphFlowSceneModel::nodePortConnections(QtNodes::NodeIndex const& index,
                                            QtNodes::PortType         portType,
                                            QtNodes::PortIndex        portID) const {
	Q_ASSERT(index.isValid());

	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	Q_ASSERT(portID >= 0);

	std::vector<std::pair<QtNodes::NodeIndex, QtNodes::PortIndex>> ret;

	if (portType == QtNodes::PortType::In) {
		if ((size_t)portID < inst->inputExecConnections.size()) {
			for (const auto& conn : inst->inputExecConnections[portID]) {
				ret.emplace_back(nodeIndex(*conn.first), conn.second);
			}
			Q_ASSERT(std::all_of(ret.begin(), ret.end(),
			                     [](auto pair) { return pair.first.isValid(); }));
			return ret;
		}

		const auto& connpair =
		    inst->inputDataConnections[portID - inst->inputExecConnections.size()];
		if (connpair.first != nullptr) {
			ret.emplace_back(nodeIndex(*connpair.first),
			                 connpair.second + connpair.first->outputExecConnections.size());
		}
		Q_ASSERT(
		    std::all_of(ret.begin(), ret.end(), [](auto pair) { return pair.first.isValid(); }));
		return ret;
	}
	if ((size_t)portID < inst->outputExecConnections.size()) {
		const auto& connpair = inst->outputExecConnections[portID];
		if (connpair.first != nullptr) {
			ret.emplace_back(nodeIndex(*connpair.first), connpair.second);
		}
		Q_ASSERT(
		    std::all_of(ret.begin(), ret.end(), [](auto pair) { return pair.first.isValid(); }));
		return ret;
	}
	for (const auto& conn :
	     inst->outputDataConnections[portID - inst->outputExecConnections.size()]) {
		ret.emplace_back(nodeIndex(*conn.first),
		                 conn.second + conn.first->inputExecConnections.size());
	}

	// make sure they're all valid
	Q_ASSERT(std::all_of(ret.begin(), ret.end(), [](auto pair) { return pair.first.isValid(); }));

	return ret;
}

// Mutation functions
/////////////////////

/// Remove a connection
bool ChigraphFlowSceneModel::removeConnection(QtNodes::NodeIndex const& leftNode,
                                              QtNodes::PortIndex        leftPortID,
                                              QtNodes::NodeIndex const& rightNode,
                                              QtNodes::PortIndex        rightPortID) {
	Q_ASSERT(leftNode.isValid());
	Q_ASSERT(rightNode.isValid());

	// make casts safe
	Q_ASSERT(leftPortID >= 0);
	Q_ASSERT(rightPortID >= 0);

	auto leftInst  = reinterpret_cast<chi::NodeInstance*>(leftNode.internalPointer());
	auto rightInst = reinterpret_cast<chi::NodeInstance*>(rightNode.internalPointer());

	if ((size_t)leftPortID < leftInst->outputExecConnections.size()) {
		auto res = chi::disconnectExec(*leftInst, (size_t)leftPortID);

		if (!res) {
			KMessageBox::detailedError(nullptr, i18n("Failed to disconnect exec"),
			                           QString::fromStdString(res.dump()));

			return false;
		}

		emit connectionRemoved(leftNode, leftPortID, rightNode, rightPortID);
		updateValidation();
		return true;
	}

	auto res = chi::disconnectData(
	    *leftInst, size_t(leftPortID - leftInst->outputExecConnections.size()), *rightInst);
	if (!res) {
		KMessageBox::detailedError(nullptr, i18n("Failed to disconnect data"),
		                           QString::fromStdString(res.dump()));
		return false;
	}
	emit connectionRemoved(leftNode, leftPortID, rightNode, rightPortID);
	updateValidation();
	return true;
}

/// Add a connection
bool ChigraphFlowSceneModel::addConnection(QtNodes::NodeIndex const& leftNode,
                                           QtNodes::PortIndex        leftPortID,
                                           QtNodes::NodeIndex const& rightNode,
                                           QtNodes::PortIndex        rightPortID) {
	Q_ASSERT(leftNode.isValid());
	Q_ASSERT(rightNode.isValid());

	// make casts safe
	Q_ASSERT(leftPortID >= 0);
	Q_ASSERT(rightPortID >= 0);

	auto leftInst  = reinterpret_cast<chi::NodeInstance*>(leftNode.internalPointer());
	auto rightInst = reinterpret_cast<chi::NodeInstance*>(rightNode.internalPointer());

	if ((size_t)leftPortID < leftInst->outputExecConnections.size()) {
		// then it's an exec connection
		auto ret = chi::connectExec(*leftInst, leftPortID, *rightInst, rightPortID);

		if (!ret) { return false; }

		emit connectionAdded(leftNode, leftPortID, rightNode, rightPortID);
		updateValidation();
		return true;
	}
	// then it's data
	auto ret = chi::connectData(*leftInst, leftPortID - leftInst->outputExecConnections.size(),
	                            *rightInst, rightPortID - rightInst->inputExecConnections.size());

	if (!ret) { return false; }

	emit connectionAdded(leftNode, leftPortID, rightNode, rightPortID);
	updateValidation();
	return true;
}

/// Remove a node
bool ChigraphFlowSceneModel::removeNode(QtNodes::NodeIndex const& index) {
	Q_ASSERT(index.isValid());

	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	// don't remove it if it's an entry node
	if (inst->type().qualifiedName() == "lang:entry") { return false; }

	auto res = inst->function().removeNode(*inst);

	if (!res) { return false; }
	emit nodeRemoved(index.id());
	updateValidation();
	return true;
}

/// Add a  -- return {} if it fails
QUuid ChigraphFlowSceneModel::addNode(QString const& typeID, QPointF const& pos) {
	// see if it's a valid typeid

	// parse the typeID
	std::string modName;
	std::string tyName;
	std::tie(modName, tyName) = chi::parseColonPair(typeID.toStdString());

	auto& ctx = mFunc->context();

	// create json if it's a local var
	nlohmann::json json;
	if (tyName.substr(0, 5) == "_get_" || tyName.substr(0, 5) == "_set_") {
		auto local = mFunc->localVariableFromName(tyName.substr(5));
		Q_ASSERT(local.valid());

		json = local.type.qualifiedName();
	}

	std::unique_ptr<chi::NodeType> nodeType;
	
	chi::Result res;
	if (modName == "lang" && tyName == "exit") {
		res = mFunc->createExitNodeType(&nodeType);
	} else {
		res = ctx.nodeTypeFromModule(modName, tyName, json, &nodeType);
	}
	
	if (!res) {
		qDebug() << "Failed to create node type: " << QString::fromStdString(res.dump());
		return {};
	}

	// create the node
	chi::NodeInstance* inst;
	res +=
	    mFunc->insertNode(std::move(nodeType), 0.f, 0.f, boost::uuids::random_generator()(), &inst);
	if (!res) { return {}; }

	auto quuid = QUuid::fromRfc4122(QByteArray(reinterpret_cast<const char*>(inst->id().data), 16));

	// create a widget
	auto widget = createEmbeddedWidget(*inst);
	if (widget != nullptr) { mEmbeddedWidgets[inst] = widget; }

	emit nodeAdded(quuid);

	updateValidation();
	return quuid;
}

/// Move a node to a new location
bool ChigraphFlowSceneModel::moveNode(QtNodes::NodeIndex const& index, QPointF newLocation) {
	Q_ASSERT(index.isValid());

	auto inst = reinterpret_cast<chi::NodeInstance*>(index.internalPointer());

	inst->setX(newLocation.x());
	inst->setY(newLocation.y());

	emit nodeMoved(index);

	return true;
}

