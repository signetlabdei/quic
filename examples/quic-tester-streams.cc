/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering, University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alvise De Biasio <alvise.debiasio@gmail.com>
 *          Federico Chiariotti <whatever@blbl.it>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *          
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/quic-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("QuicTesterStreams");

// connect to a number of traces
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}

static void
Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, const QuicHeader& q, Ptr<const QuicSocketBase> qsb)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() << std::endl;
}

static void
Traces(uint32_t serverId, std::string pathVersion, std::string finalPart)
{
  AsciiTraceHelper asciiTraceHelper;

  std::ostringstream pathCW;
  pathCW << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/CongestionWindow";
  NS_LOG_INFO("Matches cw " << Config::LookupMatches(pathCW.str().c_str()).GetN());

  std::ostringstream fileCW;
  fileCW << pathVersion << "QUIC-cwnd-change"  << serverId << "" << finalPart;

  std::ostringstream pathRTT;
  pathRTT << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RTT";

  std::ostringstream fileRTT;
  fileRTT << pathVersion << "QUIC-rtt"  << serverId << "" << finalPart;

  std::ostringstream pathRCWnd;
  pathRCWnd<< "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/0/QuicSocketBase/RWND";

  std::ostringstream fileRCWnd;
  fileRCWnd<<pathVersion << "QUIC-rwnd-change"  << serverId << "" << finalPart;

  std::ostringstream fileName;
  fileName << pathVersion << "QUIC-rx-data" << serverId << "" << finalPart;
  std::ostringstream pathRx;
  pathRx << "/NodeList/" << serverId << "/$ns3::QuicL4Protocol/SocketList/*/QuicSocketBase/Rx";
  NS_LOG_INFO("Matches rx " << Config::LookupMatches(pathRx.str().c_str()).GetN());

  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ().c_str ());
  Config::ConnectWithoutContext (pathRx.str ().c_str (), MakeBoundCallback (&Rx, stream));

  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream (fileCW.str ().c_str ());
  Config::ConnectWithoutContext (pathCW.str ().c_str (), MakeBoundCallback(&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream (fileRTT.str ().c_str ());
  Config::ConnectWithoutContext (pathRTT.str ().c_str (), MakeBoundCallback(&RttChange, stream2));

  Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream (fileRCWnd.str ().c_str ());
  Config::ConnectWithoutContextFailSafe (pathRCWnd.str ().c_str (), MakeBoundCallback(&CwndChange, stream4));
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::QuicSocketBase::SocketSndBufSize", UintegerValue(40000000));
  Config::SetDefault ("ns3::QuicSocketBase::SocketRcvBufSize", UintegerValue(40000000));
  Config::SetDefault ("ns3::QuicStreamBase::StreamSndBufSize", UintegerValue(40000000));
  Config::SetDefault ("ns3::QuicStreamBase::StreamRcvBufSize", UintegerValue(40000000));
  Config::SetDefault ("ns3::QuicSocketBase::SchedulingPolicy", TypeIdValue(QuicSocketTxEdfScheduler::GetTypeId ()));
  
  std::cout
      << "\n\n#################### SIMULATION SET-UP ####################\n\n\n";

  LogLevel log_precision = LOG_LEVEL_INFO;
  Time::SetResolution (Time::NS);
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnable ("QuicEchoClientApplication", log_precision);
  LogComponentEnable ("QuicEchoServerApplication", log_precision);
  //LogComponentEnable ("QuicSocketBase", log_precision);
  LogComponentEnable ("QuicStreamBase", log_precision);
  LogComponentEnable("QuicStreamRxBuffer", log_precision);
  LogComponentEnable("QuicStreamTxBuffer", log_precision);
  LogComponentEnable("QuicSocketTxScheduler", log_precision);
  LogComponentEnable("QuicSocketTxEdfScheduler", log_precision);
  //LogComponentEnable ("Socket", log_precision);
  // LogComponentEnable ("Application", log_precision);
  // LogComponentEnable ("Node", log_precision);
  //LogComponentEnable ("InternetStackHelper", log_precision);
  //LogComponentEnable ("QuicSocketFactory", log_precision);
  //LogComponentEnable ("ObjectFactory", log_precision);
  //LogComponentEnable ("TypeId", log_precision);
  //LogComponentEnable ("QuicL4Protocol", log_precision);
  LogComponentEnable ("QuicL5Protocol", log_precision);
  //LogComponentEnable ("ObjectBase", log_precision);

  //LogComponentEnable ("QuicEchoHelper", log_precision);
  //LogComponentEnable ("UdpSocketImpl", log_precision);
  //LogComponentEnable ("QuicHeader", log_precision);
  //LogComponentEnable ("QuicSubheader", log_precision);
  //LogComponentEnable ("Header", log_precision);
  //LogComponentEnable ("PacketMetadata", log_precision);
  LogComponentEnable ("QuicSocketTxBuffer", log_precision);


  NodeContainer nodes;
  nodes.Create (2);
  auto n1 = nodes.Get (0);
  auto n2 = nodes.Get (1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  QuicHelper stack;
  stack.InstallQuic (nodes);
  
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  // QUIC client and server
  uint32_t dlPort = 1025;
  QuicServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (n2));

  double interPacketInterval = 1000;
  QuicClientHelper dlClient (interfaces.GetAddress (1), dlPort);
  dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  dlClient.SetAttribute ("PacketSize", UintegerValue(1000));
  dlClient.SetAttribute ("MaxPackets", UintegerValue(10000000));
  clientApps.Add (dlClient.Install (n1));

  serverApps.Start (Seconds (0.99));
  clientApps.Stop (Seconds (5.0));
  clientApps.Start (Seconds (1.0));

  Simulator::Schedule (Seconds (2.0000001), &Traces, n2->GetId(),
        "./server", ".txt");
  Simulator::Schedule (Seconds (2.0000001), &Traces, n1->GetId(),
        "./client", ".txt");

  Packet::EnablePrinting ();
  Packet::EnableChecking ();

  std::cout << "\n\n#################### STARTING RUN ####################\n\n";
  Simulator::Stop (Seconds (3000));
  Simulator::Run ();
  std::cout
      << "\n\n#################### RUN FINISHED ####################\n\n\n";
  Simulator::Destroy ();

  std::cout
      << "\n\n#################### SIMULATION END ####################\n\n\n";
  return 0;
}
