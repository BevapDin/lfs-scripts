#! /bin/bash

for d in list-decoders list-encoders list-hwaccels list-muxers list-demuxers list-parsers list-protocols list-bsfs list-indevs list-outdevs list-filters ; do
	echo "# $d"
	./configure "--$d"
done

