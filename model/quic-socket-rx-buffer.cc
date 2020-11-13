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
 *          Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *
 */

#include "ns3/packet.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "quic-socket-rx-buffer.h"
// #include "ns3/ipv4-end-point-demux.h"
// #include "ns3/ipv6-end-point-demux.h"
// #include "ns3/ipv4-end-point.h"
// #include "ns3/ipv6-end-point.h"
// #include "ns3/ipv4-l3-protocol.h"
// #include "ns3/ipv6-l3-protocol.h"
// #include "ns3/ipv6-routing-protocol.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicSocketRxBuffer");

QuicSocketRxItem::QuicSocketRxItem ()
  : m_packet (0),
  m_offset (0),
  m_fin (false)
{
}

QuicSocketRxItem::QuicSocketRxItem (const QuicSocketRxItem &other)
  : m_packet (other.m_packet),
  m_offset (other.m_offset),
  m_fin (other.m_fin)
{
}

void
QuicSocketRxItem::Print (std::ostream &os) const
{
  os << "[OFF " << m_offset << "]";

  if (m_fin)
    {
      os << "|fin|";
    }

}

NS_OBJECT_ENSURE_REGISTERED (QuicSocketRxBuffer);

TypeId
QuicSocketRxBuffer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketRxBuffer")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicSocketRxBuffer> ()
  ;
  return tid;
}

QuicSocketRxBuffer::QuicSocketRxBuffer ()
  : m_recvSize (0),
  m_recvSizeTot (0),
  m_maxBuffer (32768)
{
  m_socketRecvList = QuicSocketRxPacketList ();
}

QuicSocketRxBuffer::~QuicSocketRxBuffer ()
{
}

bool
QuicSocketRxBuffer::Add (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  NS_LOG_INFO (
    "Try to append " << p->GetSize () << " bytes " << ", availSize=" << Available ());

  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {
          m_socketRecvList.insert (m_socketRecvList.end (), p->Copy ());
          m_recvSize += p->GetSize ();
          m_recvSizeTot += p->GetSize ();

          NS_LOG_INFO ("Update: Received Size = " << m_recvSize);
          return true;
        }
      else
        {
          NS_LOG_WARN ("Discarded. Try to insert empty packet.");
          return false;
        }
    }
  NS_LOG_WARN ("Rejected. Not enough room to buffer packet.");
  return false;
}

Ptr<Packet>
QuicSocketRxBuffer::Extract (uint32_t maxSize)
{
  NS_LOG_FUNCTION (this << maxSize);

  uint32_t extractSize = std::min (maxSize, m_recvSize);
  NS_LOG_INFO (
    "Requested to extract " << extractSize << " bytes from QuicSocketRxBuffer of size=" << m_recvSize);

  if (extractSize == 0)
    {
      return 0;
    }

  Ptr<Packet> outPkt = Create<Packet> ();

  QuicSocketRxPacketList::iterator it = m_socketRecvList.begin ();

  while (extractSize > 0 && !m_socketRecvList.empty () && it != m_socketRecvList.end ())
    {
      it = m_socketRecvList.begin ();
      Ptr<Packet> currentPacket = *it;

      if (currentPacket->GetSize () + outPkt->GetSize () <= extractSize)   // Merge
        {

          outPkt->AddAtEnd ((*it));
          m_socketRecvList.erase (it);

          m_recvSize -= (*it)->GetSize ();
          extractSize -= (*it)->GetSize ();
          NS_LOG_LOGIC ("Added packet of size " << (*it)->GetSize ());
          continue;
        }
      else
        {
          break;
        }

      it++;
    }

  if (outPkt->GetSize () == 0)
    {
      NS_LOG_LOGIC ("Nothing extracted.");
      return 0;
    }
  NS_LOG_INFO (
    "Extracted " << outPkt->GetSize () << " bytes from QuicSocketRxBuffer. New buffer size=" << m_recvSize);
  return outPkt;
}

uint32_t
QuicSocketRxBuffer::Available (void) const
{
  return m_maxBuffer - m_recvSize;
}

uint32_t
QuicSocketRxBuffer::GetMaxBufferSize (void) const
{
  return m_maxBuffer;
}

uint32_t
QuicSocketRxBuffer::Size (void) const
{
  NS_LOG_FUNCTION (this);

  return m_recvSize;
}

void
QuicSocketRxBuffer::SetMaxBufferSize (uint32_t s)
{
  m_maxBuffer = s;
}

void
QuicSocketRxBuffer::Print (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  QuicSocketRxBuffer::QuicSocketRxPacketList::const_iterator it;
  std::stringstream ss;
  std::stringstream as;

  for (it = m_socketRecvList.begin (); it != m_socketRecvList.end (); ++it)
    {
      (*it)->Print (ss);
    }

  os << "Socket Recv list: \n" << ss.str () << "\n\nCurrent Status: "
     << "\nNumber of receptions = " << m_socketRecvList.size ()
     << "\nReceived Size = " << m_recvSize;

}

} //namepsace ns3
