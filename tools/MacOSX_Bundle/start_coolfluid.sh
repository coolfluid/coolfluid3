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


SERVICE='coolfluid-server'

if ps ax | grep -v grep | grep $SERVICE > /dev/null
then
    echo "$SERVICE service running, everything is fine"
else
    echo "$SERVICE is not running"
    START_SERVER="Yes" #`osascript -e 'tell app "Finder" to return button returned of (display dialog "Start server?" buttons {"No","Yes"} default button 2)'`
    if [ "$START_SERVER" == "Yes" ]; then
        echo "Starting server"
        echo  "export COOLFLUID_PLUGINS=$CF3_RESOURCES/CF_VERSION/lib/libcoolfluid_fvm.dylib:$CF3_RESOURCES/coolfluid-CF_VERSION/lib/libcoolfluid_rdm.dylib" > ~/.start_coolfluid-server.command
        echo  "export LD_LIBRARY_PATH=$CF3_RESOURCES/coolfluid-CF_VERSION/lib" >> ~/.start_coolfluid-server.command
        echo  "export DYLD_LIBRARY_PATH=$CF3_RESOURCES/coolfluid-CF_VERSION/lib" >> ~/.start_coolfluid-server.command
        echo  "export PATH=$CF3_RESOURCES/coolfluid-CF_VERSION/bin:$PATH" >> ~/.start_coolfluid-server.command
        echo  "$CF3_RESOURCES/coolfluid-CF_VERSION/bin/coolfluid-server" >> ~/.start_coolfluid-server.command
        chmod 755 ~/.start_coolfluid-server.command
        open -a Terminal ~/.start_coolfluid-server.command
    else
        echo "Not starting server"
    fi
fi &

$CF3_RESOURCES/coolfluid-CF_VERSION/bin/coolfluid-gui

# if ps ax | grep -v grep | grep $SERVICE > /dev/null
# then
#     echo "$SERVICE service running, everything is maybe not fine"
#     KILL_SERVER=`osascript -e 'tell app "Finder" to return button returned of (display dialog "Quit server?" buttons {"Yes","No"} default button 2)'`
#     if [ "$KILL_SERVER" == "Yes" ]; then
#         echo "Killing server"
#         killall coolfluid-server
#     else
#         echo "Not killing server"
#     fi
#     
# else
#     echo "$SERVICE is not running"
# fi
