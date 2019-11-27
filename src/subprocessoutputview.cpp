#include "subprocessoutputview.hpp"

#include <llvm-c/BitWriter.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <chi/Context.hpp>
#include <chi/GraphModule.hpp>
#include <chi/Support/Result.hpp>
#include <chi/Support/TempFile.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

SubprocessOutputView::SubprocessOutputView(chi::GraphModule* module) : mModule(module) {
	// compile!
	chi::OwnedLLVMModule llmod;
	chi::Result          res =
	    module->context().compileModule(module->fullName(), chi::CompileSettings::Default, &llmod);

	if (!res) {
		KMessageBox::detailedError(this, "Failed to compile module",
		                           QString::fromStdString(res.dump()));

		return;
	}

	// write it to a temporary file
	fs::path tempBitcodeFile = chi::makeTempPath(".bc");
	LLVMWriteBitcodeToFile(*llmod, tempBitcodeFile.c_str());
	setReadOnly(true);

	std::filesystem::path chiPath =
	    std::filesystem::path(QApplication::applicationFilePath().toStdString()).parent_path() /
	    "chi";
#ifdef _WIN32
	chiPath.replace_extension(".exe");
#endif

	Q_ASSERT(std::filesystem::is_regular_file(chiPath));

	// run in lli
	mProcess = new QProcess(this);
	mProcess->setProgram(QString::fromStdString(chiPath.string()));

	auto args = QStringList() << QStringLiteral("interpret") << QStringLiteral("-i")
	                          << QString::fromStdString(tempBitcodeFile.string())
	                          << QStringLiteral("-O2");
	mProcess->setArguments(args);

	connect(mProcess, &QProcess::readyReadStandardOutput, this,
	        [this] { appendPlainText(mProcess->readAllStandardOutput().constData()); });

	connect(mProcess, &QProcess::readyReadStandardOutput, this, [this] {
		appendHtml("<span style='color:red'>" +
		           QString(mProcess->readAllStandardOutput().constData()).toHtmlEscaped() +
		           "</span>");
	});

	connect(mProcess,
	        static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
	        &SubprocessOutputView::processFinished);

	mProcess->start();
}

void SubprocessOutputView::cancelProcess() {
	if (mProcess != nullptr) { mProcess->kill(); }
}
