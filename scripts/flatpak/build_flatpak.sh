#!/bin/bash

set -xe

flatpak-builder --repo=repo chigraphgui org.chigraph.chigraphgui.json

flatpak build-sign repo \
  --gpg-sign=2F04F8C4132EF3BF0264FAE1D345FAFC910AA344 \
  --gpg-homedir=~/projects/flatpak-chigraph-gpg


flatpak build-update-repo repo \
  --gpg-sign=2F04F8C4132EF3BF0264FAE1D345FAFC910AA344 \
  --gpg-homedir=~/projects/flatpak-chigraph-gpg
