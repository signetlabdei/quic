/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "packet-loss-counter.h"

#include "seq-ts-header.h"
#include "quic-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicServer");

NS_OBJECT_ENSURE_REGISTERED (QuicServer);


TypeId
QuicServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicServer")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<QuicServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&QuicServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (128),
                   MakeUintegerAccessor (&QuicServer::GetPacketWindowSize,
                                         &QuicServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
    .AddAttribute ("OutputFilename",
                   "A string with the name of the file in which rx packets will be logged",
                   StringValue ("QuicServerRx.txt"),
                   MakeStringAccessor (&QuicServer::m_outFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

QuicServer::QuicServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received = 0;
}

QuicServer::~QuicServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
QuicServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
QuicServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
QuicServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint64_t
QuicServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
QuicServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
QuicServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket->Listen ();
  m_socket->SetRecvCallback (MakeCallback (&QuicServer::HandleRead, this));

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                     m_port);
      if (m_socket6->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket->Listen ();
  m_socket6->SetRecvCallback (MakeCallback (&QuicServer::HandleRead, this));

  // if(!m_outFile.is_open())
  // {
  //   m_outFile.open(m_outFilename.c_str());
  // }

}

void
QuicServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
QuicServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          // SeqTsHeader seqTs;
          // bool removed = false;
          // uint32_t m_hSize = 0;
          // uint32_t m_currentSequenceNumber = 0;
          // Time m_txTs;
          // removed = (packet->PeekHeader (seqTs) == 12);
          // if(removed)
          // {
          //   packet->RemoveHeader (seqTs);
          //   m_currentSequenceNumber = seqTs.GetSeq ();
          //   m_hSize = seqTs.GetSerializedSize();
          //   m_txTs = seqTs.GetTs ();
          //   m_lossCounter.NotifyReceived (m_currentSequenceNumber);
          // }

          // if(!m_outFile.is_open())
          // {
          //   m_outFile.open(m_outFilename.c_str(), std::ios::app);
          // }
          // if (InetSocketAddress::IsMatchingType (from))
          //   {
          //     NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () + m_hSize*removed <<
          //                  " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
          //                  " Sequence Number: " << m_currentSequenceNumber <<
          //                  " Uid: " << packet->GetUid () <<
          //                  " TXtime: " << m_txTs <<
          //                  " RXtime: " << Simulator::Now () <<
          //                  " Delay: " << Simulator::Now () - m_txTs);
          //     m_outFile << packet->GetSize () + m_hSize <<
          //                  " " << InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
          //                  " " << m_port <<
          //                  " " << m_currentSequenceNumber <<
          //                  " " << packet->GetUid () <<
          //                  " " << m_txTs <<
          //                  " " << Simulator::Now () <<
          //                  " " << Simulator::Now () - m_txTs << "\n";
          //   }
          // else if (Inet6SocketAddress::IsMatchingType (from))
          //   {
          //     NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () + m_hSize*removed <<
          //                  " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
          //                  " Sequence Number: " << m_currentSequenceNumber <<
          //                  " Uid: " << packet->GetUid () <<
          //                  " TXtime: " << m_txTs <<
          //                  " RXtime: " << Simulator::Now () <<
          //                  " Delay: " << Simulator::Now () - m_txTs);
          //     m_outFile << packet->GetSize () + m_hSize*removed <<
          //                  " " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
          //                  " " << m_currentSequenceNumber <<
          //                  " " << packet->GetUid () <<
          //                  " " << m_txTs <<
          //                  " " << Simulator::Now () <<
          //                  " " << Simulator::Now () - m_txTs << "\n";
          //   }
          m_received++;
        }
    }
}

} // Namespace ns3
