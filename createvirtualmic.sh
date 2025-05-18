#!/bin/bash

# check and remove existing module
if [ -f /tmp/virtual_mic_module_id ]; then
  module_id=$(cat /tmp/virtual_mic_module_id)
  pactl unload-module $module_id
  rm /tmp/virtual_mic_module_id
fi

# create new module
module_id=$(pactl load-module module-null-sink sink_name=virtual_mic sink_properties=device.description="VirtualMic")
echo $module_id > /tmp/virtual_mic_module_id