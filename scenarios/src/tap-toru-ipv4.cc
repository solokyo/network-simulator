#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-static-routing-helper.h"

using namespace ns3;

int main() {
  // Create a new network
  NodeContainer nodes;
  nodes.Create(16);

  // Create point-to-point links between the routers
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  // Connect the routers in a 4x4 grid pattern
  NetDeviceContainer devices;
  for (uint32_t i = 0; i < 16; ++i) {
    uint32_t row = i / 4;
    uint32_t col = i % 4;

    if (col < 3) {
      devices = p2p.Install(nodes.Get(i), nodes.Get(i + 1)); // Horizontal link
    }

    if (row < 3) {
      devices = p2p.Install(nodes.Get(i), nodes.Get(i + 4)); // Vertical link
    }
  }

  // Install the IPv4 stack on the routers
  InternetStackHelper stack;
  stack.Install(nodes);

  // Assign IP addresses to the routers
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < 16; ++i) {
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    address.NewNetwork();

    // Configure the static routing tables
    Ipv4StaticRoutingHelper staticRouting;
    for (uint32_t j = 0; j < 16; ++j) {
      Ptr<Ipv4> ipv4 = nodes.Get(j)->GetObject<Ipv4>();
      Ptr<Ipv4StaticRouting> staticRoutingTable = staticRouting.GetStaticRouting(ipv4);

      // Add routes to the directly connected networks
      for (uint32_t k = 0; k < interfaces.GetN(); ++k) {
        Ipv4Address networkAddress = interfaces.GetAddress(k);
        staticRoutingTable->AddNetworkRouteTo(Ipv4Address::GetBroadcast(), Ipv4Mask("/0"), k);
      }

      // Add routes to the neighboring routers based on the grid pattern
      uint32_t row = j / 4;
      uint32_t col = j % 4;
      if (col > 0) {
        staticRoutingTable->AddNetworkRouteTo(interfaces.GetAddress(j - 1), Ipv4Mask("255.255.255.255"), j - 1, 1); // Left neighbor
      }

      if (col < 3) {
        staticRoutingTable->AddNetworkRouteTo(interfaces.GetAddress(j + 1), Ipv4Mask("255.255.255.255"), j + 1, 1); // Right neighbor
      }

      if (row > 0) {
        staticRoutingTable->AddNetworkRouteTo(interfaces.GetAddress(j - 4), Ipv4Mask("255.255.255.255"), j - 4, 1); // Top neighbor
      }

      if (row < 3) {
        staticRoutingTable->AddNetworkRouteTo(interfaces.GetAddress(j + 4), Ipv4Mask("255.255.255.255"), j + 4, 1); // Bottom neighbor
      }
    }
  }

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}