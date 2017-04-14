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
	brew install kf5-extra-cmake-module
	brew install kf5-karchive	kf5-ktexteditor		kf5-kjobwidgets	 \
		     kf5-kdbusaddons	kf5-kio			kf5-kcrash	 \
		     kf5-sonnet		kf5-syntax-highlighting	kf5-kparts	 \
		     kf5-kguiaddons	kf5-kitemviews		kf5-kconfig	 \
		     kf5-kconfigwidgets	kf5-kauth		kf5-kcodecs	 \
		     kf5-kcompletion	kf5-kglobalaccel	kf5-kservice	 \
		     kf5-kwindowsystem	kf5-kcoreaddons		kf5-ki18n	 \
		     kf5-kxmlgui	kf5-kwidgetsaddons	kf5-ktextwidgets \
		     kf5-kiconthemes

	bash ./setup.sh

fi

