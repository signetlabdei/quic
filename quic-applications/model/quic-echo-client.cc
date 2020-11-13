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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "quic-echo-client.h"
#include "ns3/quic-header.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicEchoClientApplication");

NS_OBJECT_ENSURE_REGISTERED (QuicEchoClient);

TypeId
QuicEchoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicEchoClient")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<QuicEchoClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&QuicEchoClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&QuicEchoClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&QuicEchoClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&QuicEchoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&QuicEchoClient::GetDataSize,
                                         &QuicEchoClient::SetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("StreamId",
                   "Identifier of the stream to be used in the QUIC connection",
                   UintegerValue (2),
                   MakeUintegerAccessor (&QuicEchoClient::GetStreamId,
                                         &QuicEchoClient::SetStreamId),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&QuicEchoClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


QuicEchoClient::QuicEchoClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

QuicEchoClient::~QuicEchoClient ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
QuicEchoClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
QuicEchoClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
QuicEchoClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
QuicEchoClient::StartApplication (void)
{
  NS_LOG_INFO ("##########  QUIC Echo Client START at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");

      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&QuicEchoClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (2.));
}

void
QuicEchoClient::StopApplication ()
{
  NS_LOG_INFO ("##########  QUIC Echo Client STOP at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void
QuicEchoClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t
QuicEchoClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
QuicEchoClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
QuicEchoClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
QuicEchoClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
QuicEchoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &QuicEchoClient::Send, this);
}

void
QuicEchoClient::ScheduleClosing (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_closeEvent = Simulator::Schedule (dt, &QuicEchoClient::Close, this);
}

void
QuicEchoClient::ScheduleRestart (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_connectEvent = Simulator::Schedule (dt, &QuicEchoClient::Restart, this);
}

void
QuicEchoClient::Restart (void)
{
  NS_LOG_INFO ("##########  QUIC Echo Client RESTART at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_connectEvent.IsExpired ());

  TypeId tid = TypeId::LookupByName ("ns3::QuicSocketFactory");
  //NS_LOG_INFO("node is "<< GetNode());
  m_socket = Socket::CreateSocket (GetNode (), tid);
  if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
    {
      if (m_socket->Bind () == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
    {
      if (m_socket->Bind6 () == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
    {
      if (m_socket->Bind () == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Connect (m_peerAddress);
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
    {
      if (m_socket->Bind6 () == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Connect (m_peerAddress);
    }
  else
    {
      NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
    }

  m_socket->SetRecvCallback (MakeCallback (&QuicEchoClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  SetFill ("Re-Hello World");
  ScheduleTransmit (Seconds (2.));
}


void
QuicEchoClient::Close (void)
{
  NS_LOG_INFO ("##########  QUIC Echo Client CLOSING at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_closeEvent.IsExpired ());

  m_socket->Close ();
}


void
QuicEchoClient::Send (void)
{
  NS_LOG_INFO ("##########  QUIC Echo Client SENDING at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "QuicEchoClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "QuicEchoClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      NS_LOG_INFO ("no data");
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);

  // send and use the flags param in the Send API to let the QUIC socket know
  // about which stream should be used for data
  int sent = m_socket->Send (p, m_streamId);

  NS_ASSERT_MSG (sent - p->GetSize () == 0, "Could not send data (packet and sent size inconsistent)");

  ++m_sent;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }

  uint8_t *buffer = new uint8_t[p->GetSize ()];
  p->CopyData (buffer, p->GetSize ());
  std::string s = std::string ((char*)buffer);
  NS_LOG_INFO ("Client sent: " << s << "");

  if (m_sent < m_count)
    {
      ScheduleTransmit (m_interval);
    }
}

void
QuicEchoClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_INFO ("##########  QUIC Echo Client RECEIVING at time " << Simulator::Now ().GetSeconds () << " ##########");
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      packet->RemoveAllByteTags ();
      packet->RemoveAllPacketTags ();

      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }

      uint8_t *buffer = new uint8_t[packet->GetSize ()];
      packet->CopyData (buffer, packet->GetSize ());
      std::string s = std::string ((char*)buffer);
      NS_LOG_INFO ("Client received: " << s << "");
    }
}

void
QuicEchoClient::SetStreamId (uint32_t streamId)
{
  NS_LOG_FUNCTION (this << streamId);
  NS_ABORT_MSG_IF (streamId == 0, "Stream 0 cannot be used for application data");

  m_streamId = streamId;
}

uint32_t
QuicEchoClient::GetStreamId (void) const
{
  return m_streamId;
}

} // Namespace ns3
