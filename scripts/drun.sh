#!/bin/bash

docker run --rm --interactive --tty \
	--volume=/home/paul/dev/surge-rack-170eol:/home/build/rack-plugin-toolchain/surge-rack \
	--env PLUGIN_DIR=surge-rack \
	ghcr.io/qno/rack-plugin-toolchain-win-linux:latest \
	/bin/bash

	#rack-plugin-toolchain:$(DOCKER_IMAGE_VERSION) \
