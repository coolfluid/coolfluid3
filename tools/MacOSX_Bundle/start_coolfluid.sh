#!/bin/sh
#
echo "$0"
CF3_BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/coolfluid//'`"
CF3_RESOURCES="$CF3_BUNDLE/Contents/Resources"
#K3D_TEMP="/tmp/k3d/$UID"
#K3D_ETC="$K3D_TEMP/etc"
#K3D_PANGO_RC_FILE="$K3D_ETC/pango/pangorc"

echo "running $0"
echo "CF3_BUNDLE: $CF3_BUNDLE"
echo "CF3_RESOURCES: $CF3_RESOURCES"

export "DYLD_LIBRARY_PATH=$CF3_RESOURCES/coolfluid-CF_VERSION/lib"
export "PATH=$CF3_RESOURCES/coolfluid-CF_VERSION/bin:$PATH"

#export
$CF3_RESOURCES/coolfluid-CF_VERSION/bin/coolfluid-server &
server_pid=$!
$CF3_RESOURCES/coolfluid-CF_VERSION/bin/coolfluid-client
kill $server_pid
