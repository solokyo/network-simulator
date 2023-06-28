#!/usr/bin/env bash

# Function to teardown network devices
teardown_network() {
  local index=$1
  local bridge="br-$index"
  local tap="tap-$index"

  echo "Tearing down network $index..."
  echo "Bridge: $bridge"
  echo "TAP: $tap"

  # Delete bridge
  sudo ip link del $bridge

  # Delete tap device
  sudo ip link del $tap

  echo "Network $index teardown completed."
  echo
}

# Teardown 64 networks
for i in {0..63}; do
  teardown_network $i
done

# Stop all running containers
docker compose -f scenarios/cttc-3gpp-channel-tap.yaml down

# List remaining network devices
ip link

# Check if any containers still exist
docker container ls -a