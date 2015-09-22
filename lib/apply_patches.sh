#!/usr/bin/env bash

cp nvp.patch /usr/local/include/nvector
cd /usr/local/include/nvector
patch < nvp.patch
rm nvp.patch
