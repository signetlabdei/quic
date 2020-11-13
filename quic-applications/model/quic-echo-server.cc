/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Authors: Davide Marcato <davide.marcato.4@studenti.unipd.it>
 *          Stefano Ravazzolo <stefano.ravazzolo@studenti.unipd.it>
 *          Alvise De Biasio <alvise.debiasio@studenti.unipd.it>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/quic-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "quic-echo-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicEchoServerApplication");

NS_OBJECT_ENSURE_REGISTERED (QuicEchoServer);

TypeId
QuicEchoServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicEchoServer")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<QuicEchoServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&QuicEchoServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("StreamId",
                   "Identifier of the stream to be used in the QUIC connection",
                   UintegerValue (2),
                   MakeUintegerAccessor (&QuicEchoServer::GetStreamId,
                                         &QuicEchoServer::SetStreamId),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

QuicEchoServer::QuicEchoServer ()
{
  NS_LOG_FUNCTION (this);
}

QuicEchoServer::~QuicEchoServer ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
QuicEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
QuicEchoServer::StartApplication (void)
{
  NS_LOG_INFO ("##########  QUIC Echo Server START at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);
  //il client ha bisogno di un socket udp per ogni socket quic
  //il server per tutti i socket quic gli basta un socket udp che ascolta

  if (m_socket == 0)
    {
      //NS_LOG_INFO("Need to create IPv4");
      TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");
      //NS_LOG_INFO("node is "<< GetNode());
      m_socket = Socket::CreateSocket (GetNode (), tid);
      //NS_LOG_INFO("Created IPv4 socket");
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          NS_FATAL_ERROR ("Error: Failed to join multicast group");
        }
    }

//  if (m_socket6 == 0)
//    {
//      NS_LOG_INFO("Need to create IPv6");
//      TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");
//      m_socket6 = Socket::CreateSocket (GetNode (), tid);
//      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
//      if (m_socket6->Bind (local6) == -1)
//        {
//          NS_FATAL_ERROR ("Failed to bind socket");
//        }
//      if (addressUtils::IsMulticast (local6))
//      {
//       NS_FATAL_ERROR ("Error: Failed to join multicast group");
//      }
//    }

  m_socket->Listen ();
  m_socket->SetRecvCallback (MakeCallback (&QuicEchoServer::HandleRead, this));
  //m_socket6->SetRecvCallback (MakeCallback (&QuicEchoServer::HandleRead, this));
}

void
QuicEchoServer::StopApplication ()
{
  NS_LOG_INFO ("##########  QUIC Echo Server STOP at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
QuicEchoServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_INFO ("##########  QUIC Echo Server RECEIVING at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      // print the received packet
      uint8_t *buffer = new uint8_t[packet->GetSize ()];
      packet->CopyData (buffer, packet->GetSize ());
      std::string s = std::string ((char*)buffer);
      if (s != "CONNECTION_CLOSE")
        {
          NS_LOG_INFO ("Server received: " << s);
        }
      else
        {
          NS_LOG_INFO ("Server received: " << s);
        }



      if (s != "CONNECTION_CLOSE")
        {
          NS_LOG_INFO ("##########  QUIC Echo Server ECHOING at time " << Simulator::Now ().GetSeconds () << " ##########");
          socket->SendTo (packet, 0, from);
        }
      //socket->Send (packet, 0);
      //socket->SendTo (packet, 0, from);
      //NS_LOG_LOGIC ("Echoed packet");
      if (InetSocketAddress::IsMatchingType (from) and s != "CONNECTION_CLOSE")
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from) and s != "CONNECTION_CLOSE")
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }


      if (s != "CONNECTION_CLOSE")
        {
          NS_LOG_INFO ("Server sent: " << s);
        }

      //NS_LOG_LOGIC ("Echoed packettt");
    }
}

Ptr<Socket>
QuicEchoServer::GetSocket ()
{
  return m_socket;
}

void
QuicEchoServer::SetStreamId (uint32_t streamId)
{
  NS_LOG_FUNCTION (this << streamId);
  NS_ABORT_MSG_IF (streamId == 0, "Stream 0 cannot be used for application data");

  m_streamId = streamId;
}

uint32_t
QuicEchoServer::GetStreamId (void) const
{
  return m_streamId;
}

} // Namespace ns3
