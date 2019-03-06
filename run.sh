#!/bin/bash

docker run \
	-it \
	--name cachedoge \
	--privileged \
	-v "$(pwd)"/src:/root/src \
	cachedoge/v3 /bin/bash
