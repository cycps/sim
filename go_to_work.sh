#!/usr/bin/env bash

docker run -ti -v `pwd`:/sim --hostname=simbuilder --name=simbuilder simbuilder
