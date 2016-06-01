// +------------+                           +------------+      +------------+
// |       sim0 |-------------------------->| sim0  sim1 |<-----| sim0       |
// | mclients[0]|           net1            | routers[0] | net2 | mservers[0]|
// |       sim1 |                           | sim2       |      |            |
// +------------+                           +------------+      +------------+
//             |                             ^
//               |                         |
//                 | net3           net4 |
//                   |                 |
//                     |             |
//                      +          | 
// +------------+      +------------+      +------------+
// |            |      | sim0  sim1 |      |            |
// | clients[0] | net5 | routers[1] | net6 | servers[0] |
// |       sim0 |----->| sim2  sim3 |<-----| sim0       |
// +------------+      +------------+      +------------+
// (.1-->.2) (.2<--.1)
// net1:10.1.0.0/24, net2:10.2.0.0/24, net3:10.3.0.0/24
// net4:10.4.0.0/24, net5:10.5.0.0/24, net6:10.6.0.0/24
//
// mclients[0] 10.1.0.1 -- MTCP --> 10.2.0.1 mservers[0]
// mclients[0] 10.3.0.1 -- MTCP --> 10.2.0.1 mservers[0]
//
// clients[0] 10.5.0.1 -- TCP --> 10.6.0.1 mservers[1]

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;

int main (int argc, char *argv[])
{
	uint32_t nRtrs = 2;
	CommandLine cmd;
	cmd.AddValue ("nRtrs", "Number of routers. Default 2", nRtrs);
	cmd.Parse (argc, argv);
	
	std::string ClientRunTime = "20";

	float RouteStart = 0.2;
	float ShowRoute = RouteStart + 0.2;
	
	float AppsStart = 1;
	float AppsStop = (float)atof(ClientRunTime.c_str());
	AppsStop += 10;

	float SimStop = AppsStop + 10;

	float MptcpOn = ShowRoute + 0.2;
	float MptcpOff = AppsStop;

	std::string ClientDataRate = "100Mbps";
	std::string ClientDelay = "150ms";
	
	std::string ServerDataRate = "100Mbps";
	std::string ServerDelay = "150ms";

	std::string RouterDataRate = "100Mbps";
	std::string RouterDelay = "150ms";

	float ERate = 0.02;
	std::string wSize = "150000";
	
	bool olia = true;
	bool runMPTCP = true;
	

	
	NodeContainer clients, mclients, servers, mservers, routers;
	clients.Create (1);
	servers.Create (1);
	routers.Create (2);

	mclients.Create (1);
	mservers.Create (1);
	
	DceManagerHelper dceManager;
	dceManager.SetTaskManagerAttribute ("FiberManagerType", StringValue ("UcontextFiberManager"));
	
	dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
	LinuxStackHelper stack;
	stack.Install (clients);
	stack.Install (mclients);
	stack.Install (servers);
	stack.Install (mservers);
	stack.Install (routers);
	
	dceManager.Install (clients);
	dceManager.Install (mclients);
	dceManager.Install (servers);
	dceManager.Install (mservers);
	dceManager.Install (routers);
	
	PointToPointHelper pointToPoint;

	NetDeviceContainer dev1, dev2, dev3, dev4, dev5, dev6;

	Ipv4AddressHelper net1, net2, net3, net4, net5, net6;
	

	std::ostringstream cmd_oss;

	net1.SetBase ("10.1.0.0", "255.255.255.0");
	net2.SetBase ("10.2.0.0", "255.255.255.0");
	net3.SetBase ("10.3.0.0", "255.255.255.0");
	net4.SetBase ("10.4.0.0", "255.255.255.0");
	net5.SetBase ("10.5.0.0", "255.255.255.0");
	net6.SetBase ("10.6.0.0", "255.255.255.0");

	// mclients 0 -> routers 0
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (ClientDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (ClientDelay));
	Ptr<RateErrorModel> em1 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)); // create ErrorRate variable (em1)
	dev1 = pointToPoint.Install (mclients.Get (0), routers.Get (0));
	dev1.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em1));  // apply ErrorRate (em1) on dev[0]
	// Assign ip addresses
	Ipv4InterfaceContainer if1 = net1.Assign (dev1);
	net1.NewNetwork ();
	
	// mservers 0 -> routers 0
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (RouterDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (RouterDelay));
	Ptr<RateErrorModel> em2 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
	dev2 = pointToPoint.Install (mservers.Get (0), routers.Get (0));
	dev2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
	// Assign ip addresses
	Ipv4InterfaceContainer if2 = net2.Assign (dev2);
	net2.NewNetwork ();

	// mclients 0 -> routers 1: net3
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (ServerDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (ServerDelay));
	Ptr<RateErrorModel> em3 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
	dev3 = pointToPoint.Install (mclients.Get (0), routers.Get (1));
	dev3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em3));
	// Assign ip addresses
	Ipv4InterfaceContainer if3 = net3.Assign (dev3);
	net3.NewNetwork ();

	// routers 1 -> routers 0: net4
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (ClientDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (ClientDelay));
	Ptr<RateErrorModel> em4 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
	dev4 = pointToPoint.Install (routers.Get (1), routers.Get (0));
	dev4.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em4));
	// Assign ip addresses
	Ipv4InterfaceContainer if4 = net4.Assign (dev4);
	net4.NewNetwork ();

	// clients 0 -> routers 1: net5
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (RouterDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (RouterDelay));
	Ptr<RateErrorModel> em5 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
	dev5 = pointToPoint.Install (clients.Get (0), routers.Get (1));
	dev5.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em5));
	// Assign ip addresses
	Ipv4InterfaceContainer if5 = net5.Assign (dev5);
	net5.NewNetwork ();

	// servers 0 -> routers 1: net6
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue (RouterDataRate));
	pointToPoint.SetChannelAttribute ("Delay", StringValue (RouterDelay));
	Ptr<RateErrorModel> em6 = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (ERate), "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
	dev6 = pointToPoint.Install (servers.Get (0), routers.Get (1));
	dev6.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em6));
	// Assign ip addresses
	Ipv4InterfaceContainer if6 = net6.Assign (dev6);
	net6.NewNetwork ();
	
	// clients[0]'s default route
	LinuxStackHelper::RunIp (clients.Get (0), Seconds (RouteStart), "route add default via 10.5.0.2 dev sim0");

	// servers[0]'s default route
	LinuxStackHelper::RunIp (servers.Get (0), Seconds (RouteStart), "route add default via 10.6.0.2 dev sim0");

	if (runMPTCP) {
		// mclients[0]
		cmd_oss.str ("");
		cmd_oss << "route add 10.2.0.0/24 via " << if1.GetAddress (1, 0) << " dev sim0 metric 2";
		LinuxStackHelper::RunIp (mclients.Get (0), Seconds (RouteStart), cmd_oss.str ().c_str ());

		// mclients[0]
		cmd_oss.str ("");
		cmd_oss << "route add 10.2.0.0/24 via " << if3.GetAddress (1, 0) << " dev sim1 metric 1";
		LinuxStackHelper::RunIp (mclients.Get (0), Seconds (RouteStart), cmd_oss.str ().c_str ());
	} else {
		// mclients[0]
		cmd_oss.str ("");
		cmd_oss << "route add 10.2.0.0/24 via " << if3.GetAddress (1, 0) << " dev sim1";
		LinuxStackHelper::RunIp (mclients.Get (0), Seconds (RouteStart), cmd_oss.str ().c_str ());
	}

	// cmd_oss.str ("");
	// LinuxStackHelper::RunIp (mclients.Get (0), Seconds (ShowRoute), "route");

	// Schedule Up/Down for mclients[0]
	// LinuxStackHelper::RunIp (mclients.Get (0), Seconds (RouteStart), "link set dev sim0 multipath off");
	// LinuxStackHelper::RunIp (mclients.Get (0), Seconds (MptcpOn), "link set dev sim0 multipath on");
	
	// mservers[0]'s default route
	LinuxStackHelper::RunIp (mservers.Get (0), Seconds (RouteStart), "route add default via 10.2.0.2 dev sim0");

	// Schedule Up/Down for mservers[0]
	// LinuxStackHelper::RunIp (mservers.Get (0), Seconds (RouteStart), "link set dev sim0 multipath off");
	// LinuxStackHelper::RunIp (mservers.Get (0), Seconds (MptcpOn), "link set dev sim0 multipath on");

	// setup ip route for routers[0]
	cmd_oss.str ("");
	cmd_oss << "route add 10.3.0.0/24 via " << if4.GetAddress (0, 0) << " dev sim2";
	LinuxStackHelper::RunIp (routers.Get (0), Seconds (RouteStart), cmd_oss.str ().c_str ());

	// cmd_oss.str ("");
	// LinuxStackHelper::RunIp (routers.Get (1), Seconds (ShowRoute), "route");


	// setup ip route for routers[1]
	cmd_oss.str ("");
	cmd_oss << "route add 10.2.0.0/24 via " << if4.GetAddress (1, 0) << " dev sim1";
	LinuxStackHelper::RunIp (routers.Get (1), Seconds (RouteStart), cmd_oss.str ().c_str ());

	// cmd_oss.str ("");
	// LinuxStackHelper::RunIp (routers.Get (1), Seconds (ShowRoute), "route");


	// debug
	stack.SysctlSet (clients, ".net.mptcp.mptcp_debug", "1");
	stack.SysctlSet (mclients, ".net.mptcp.mptcp_debug", "1");

	stack.SysctlSet (servers, ".net.mptcp.mptcp_debug", "1");
	stack.SysctlSet (mservers, ".net.mptcp.mptcp_debug", "1");

	stack.SysctlSet (clients, ".net.mptcp.mptcp_enabled", "0");
	stack.SysctlSet (servers, ".net.mptcp.mptcp_enabled", "0");

	if (runMPTCP) {
		stack.SysctlSet (mclients, ".net.mptcp.mptcp_enabled", "1");
		stack.SysctlSet (mservers, ".net.mptcp.mptcp_enabled", "1");
	} else {
		stack.SysctlSet (mclients, ".net.mptcp.mptcp_enabled", "0");
		stack.SysctlSet (mservers, ".net.mptcp.mptcp_enabled", "0");
	}

	if (olia) {
		stack.SysctlSet (mclients, ".net.ipv4.tcp_congestion_control", "olia");
		stack.SysctlSet (mservers, ".net.ipv4.tcp_congestion_control", "olia");
	} else {
		stack.SysctlSet (mclients, ".net.ipv4.tcp_congestion_control", "lia");
		stack.SysctlSet (mservers, ".net.ipv4.tcp_congestion_control", "lia");
	}

	stack.SysctlSet (mclients, ".net.mptcp.mptcp_path_manager", "fullmesh");
	stack.SysctlSet (mservers, ".net.mptcp.mptcp_path_manager", "fullmesh");
	
	
	DceApplicationHelper dce;
	ApplicationContainer apps;
	
	dce.SetStackSize (1 << 20);
	
	// Launch iperf clients[0]
	dce.SetBinary ("iperf");
	dce.ResetArguments ();
	dce.ResetEnvironment ();
	dce.AddArgument ("-c");
	dce.AddArgument ("10.6.0.1");
	dce.AddArgument ("-i");
	dce.AddArgument ("1");
	dce.AddArgument ("--time");
	dce.AddArgument (ClientRunTime);
	dce.AddArgument ("--window");
	dce.AddArgument (wSize);
	apps = dce.Install (clients.Get (0));
	apps.Start (Seconds (AppsStart));
	apps.Stop (Seconds (AppsStop));


	// Launch iperf servers[0]
	dce.SetBinary ("iperf");
	dce.ResetArguments ();
	dce.ResetEnvironment ();
	dce.AddArgument ("-s");




	dce.AddArgument ("-P");
	dce.AddArgument ("1");
	dce.AddArgument ("--window");
	dce.AddArgument (wSize);
	apps = dce.Install (servers.Get (0));
	apps.Start (Seconds (AppsStart));


	// Launch iperf mclients[0]
	dce.SetBinary ("iperf");
	dce.ResetArguments ();
	dce.ResetEnvironment ();
	dce.AddArgument ("-c"); // run in client mode 
	dce.AddArgument ("10.2.0.1"); // connecting to host (10.2.0.1)
	dce.AddArgument ("-i"); // pause n (1) second between periodic bandwidth reports
	dce.AddArgument ("1");
	dce.AddArgument ("--time"); // time (ClientRunTime) in seconds to transmit for
	dce.AddArgument (ClientRunTime);
	dce.AddArgument ("--window");
	dce.AddArgument (wSize);
	apps = dce.Install (mclients.Get (0));
	apps.Start (Seconds (AppsStart));
	apps.Stop (Seconds (AppsStop));

	// Launch iperf mservers[0]
	dce.SetBinary ("iperf");
	dce.ResetArguments ();
	dce.ResetEnvironment ();
	dce.AddArgument ("-s"); // run in server mode
	dce.AddArgument ("-P"); // number (2) of parallel client threads to run
	dce.AddArgument ("2");
	dce.AddArgument ("--window");
	dce.AddArgument (wSize);
	apps = dce.Install (mservers.Get (0));
	apps.Start (Seconds (AppsStart));

//	pointToPoint.EnablePcapAll ("files-mptcp", false);

	pointToPoint.EnablePcap ("files-clients", clients, false);
	pointToPoint.EnablePcap ("files-servers", servers, false);
	pointToPoint.EnablePcap ("files-mclients", mclients, false);
	pointToPoint.EnablePcap ("files-mservers", mservers, false);
	pointToPoint.EnablePcap ("files-routers", routers, false);
	
	
	// pointToPoint.EnablePcap ("files-dev9", dev9, false);
	// pointToPoint.EnablePcap ("files-dev7", dev7, false);
	
	// AsciiTraceHelper ascii;
	// pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("files.tr"));
	// pointToPoint.EnableAscii(ascii.CreateFileStream ("files-mservers.tr"),mservers);

	Simulator::Stop (Seconds (SimStop));
	Simulator::Run ();
	Simulator::Destroy ();
	
	return 0;
}