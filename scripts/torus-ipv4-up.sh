#!/usr/bin/env bash

# Function to configure network devices
configure_network() {
  local index=$1
  local bridge="br-$index"
  local tap="tap-$index"
  local internal="internal-$index"
  local external="external-$index"
  local mac_address=$(printf "12:34:56:78:%02X:%02X" $((index/256)) $((index%256)))
  local ip_address="192.168.$((index/256)).$((index%256))"

  echo "Configuring network $index..."
  echo "Bridge: $bridge"
  echo "TAP: $tap"
  echo "Internal: $internal"
  echo "External: $external"
  echo "MAC Address: $mac_address"
  echo "IP Address: $ip_address"

  # Add bridge
  sudo ip link add name $bridge type bridge

  # Add tap device
  sudo ip tuntap add $tap mode tap
  sudo ifconfig $tap 0.0.0.0 promisc up

  # Attach tap device to bridge
  sudo ip link set $tap master $bridge
  sudo ip link set $bridge up

  # Create Veth pair and attach to bridge
  sudo ip link add $internal type veth peer name $external
  sudo ip link set $internal master $bridge
  sudo ip link set $internal up

  # Configure external side with MAC address and IP address
  sudo ip link set $external address $mac_address
  sudo ip addr add $ip_address/24 dev $external
  sudo ip link set $external up

  # Get PID of container
  local pid=$(docker inspect --format '{{ .State.Pid }}' $container_name)

  # Configure container-side pair with interface and address
  sudo ip link set $external netns $pid
  sudo ip netns exec $pid ip link set dev $external name eth0
  sudo ip netns exec $pid ip link set eth0 address $mac_address
  sudo ip netns exec $pid ip link set eth0 up
  sudo ip netns exec $pid ip addr add $ip_address/24 dev eth0

  echo "Network $index configured."
  echo
}

# Configure 64 networks
for i in {0..63}; do
  configure_network $i
done