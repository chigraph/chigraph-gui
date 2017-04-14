#!/usr/bin/env bash

set -xe

if [ "$TRAVIS_OS_NAME" == "linux" ]; then

	if [ "$LLVM_VERSION" == "3.9" ] || [ "$LLVM_VERSION" == "4.0" ]; then
		sudo apt-get install liblldb-${LLVM_VERSION}-dev
	else
		sudo apt-get install lldb-${LLVM_VERSION}-dev
	fi

	bash ./setup.sh

else

	brew install cmake qt5 bison gettext ninja python3 || echo
	brew install llvm --with-clang

	brew tap chigraph/kf5
	brew install chigraph/kf5/kf5-extra-cmake-modules
	brew install chigraph/kf5/kf5-karchive		chigraph/kf5/kf5-ktexteditor		chigraph/kf5/kf5-kjobwidgets	\
		     chigraph/kf5/kf5-kdbusaddons	chigraph/kf5/kf5-kio			chigraph/kf5/kf5-kcrash		\
		     chigraph/kf5/kf5-sonnet		chigraph/kf5/kf5-syntax-highlighting	chigraph/kf5/kf5-kparts		\
		     chigraph/kf5/kf5-kguiaddons	chigraph/kf5/kf5-kitemviews		chigraph/kf5/kf5-kconfig	\
		     chigraph/kf5/kf5-kconfigwidgets	chigraph/kf5/kf5-kauth			chigraph/kf5/kf5-kcodecs	\
		     chigraph/kf5/kf5-kcompletion	chigraph/kf5/kf5-kglobalaccel		chigraph/kf5/kf5-kservice	\
		     chigraph/kf5/kf5-kwindowsystem	chigraph/kf5/kf5-kcoreaddons		kchigraph/kf5/f5-ki18n		\
		     chigraph/kf5/kf5-kxmlgui		chigraph/kf5/kf5-kwidgetsaddons		chigraph/kf5/kf5-ktextwidgets	\
		     chigraph/kf5/kf5-kiconthemes

	bash ./setup.sh

fi

