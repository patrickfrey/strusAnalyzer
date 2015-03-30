#!/bin/sh

# DEBIAN
PACKAGEID="strusanalyzer-0.0"

cd pkg/$PACKAGEID
dpkg-buildpackage

