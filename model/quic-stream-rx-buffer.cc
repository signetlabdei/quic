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
// #include "ns3/ipv4-end-point-demux.h"
// #include "ns3/ipv6-end-point-demux.h"
// #include "ns3/ipv4-end-point.h"
// #include "ns3/ipv6-end-point.h"
// #include "ns3/ipv4-l3-protocol.h"
// #include "ns3/ipv6-l3-protocol.h"
// #include "ns3/ipv6-routing-protocol.h"
#include <algorithm>
#include "quic-stream-rx-buffer.h"
#include "quic-subheader.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicStreamRxBuffer");

QuicStreamRxItem::QuicStreamRxItem ()
  : m_packet (0),
  m_offset (0),
  m_fin (false)
{
}

QuicStreamRxItem::QuicStreamRxItem (const QuicStreamRxItem &other)
  : m_packet (other.m_packet),
  m_offset (other.m_offset),
  m_fin (other.m_fin)
{
}

void
QuicStreamRxItem::Print (std::ostream &os) const
{
  os << "[OFF " << m_offset << "]";

  if (m_fin)
    {
      os << "|fin|";
    }

}

NS_OBJECT_ENSURE_REGISTERED (QuicStreamRxBuffer);

TypeId
QuicStreamRxBuffer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicStreamRxBuffer")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicStreamRxBuffer> ()
  ;
  return tid;
}

QuicStreamRxBuffer::QuicStreamRxBuffer ()
  : m_numBytesInBuffer (0),
  m_finalSize (0),
  m_maxBuffer (131072),
  m_recvFin (
    false)
{
  m_streamRecvList = QuicStreamRxPacketList ();
}

QuicStreamRxBuffer::~QuicStreamRxBuffer ()
{
}

bool
QuicStreamRxBuffer::Add (Ptr<Packet> p, const QuicSubheader& sub)
{
  NS_LOG_FUNCTION (this << p << sub);

  NS_LOG_INFO (
    "Try to append " << p->GetSize () << " bytes " << ", availSize=" << Available ());

  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {

          QuicStreamRxItem *item = new QuicStreamRxItem ();
          item->m_packet = p->Copy ();
          item->m_offset = sub.GetOffset ();
          item->m_fin = sub.IsStreamFin ();

          QuicStreamRxPacketList::iterator it;

          // FIN packet for the stream
          if (sub.IsStreamFin ())
            {
              NS_LOG_LOGIC ("FIN packet for the stream");
              m_finalSize = sub.GetOffset () + p->GetSize ();
              m_recvFin = true;
              m_streamRecvList.insert (m_streamRecvList.end (), item);
              return true;
            }

          // Find the right place to insert packet (TODO: binary search would be more efficient)
          bool inserted = false;
          for (it = m_streamRecvList.begin (); it < m_streamRecvList.end ();
               ++it)
            {
              if (item->m_offset == (*it)->m_offset)
                {
                  // Duplicate packet
                  NS_LOG_WARN ("Discarded duplicate packet.");
                  return false;
                }
              if (item->m_offset < (*it)->m_offset)
                {
                  m_streamRecvList.insert (it, item);
                  NS_LOG_LOGIC ("Inserted packet");
                  inserted = true;
                  break;
                }
            }
          // The packet is not in the buffer, append it to the end
          if (!inserted)
            {
              NS_LOG_LOGIC ("Appending packet");
              m_streamRecvList.insert (m_streamRecvList.end (), item);
            }
          m_numBytesInBuffer += p->GetSize ();
          NS_LOG_INFO ("Update: Received Size = " << m_numBytesInBuffer);
          return true;

        }
      else
        {
          NS_LOG_WARN ("Discarded. Trying to insert empty packet.");
          return false;
        }
    }
  NS_LOG_WARN ("Rejected. Not enough room to buffer packet.");
  return false;
}

Ptr<Packet>
QuicStreamRxBuffer::Extract (uint32_t maxSize)
{
  NS_LOG_FUNCTION (this << maxSize);

  uint32_t extractSize = std::min (maxSize, m_numBytesInBuffer);
  NS_LOG_INFO (
    "Requested to extract " << extractSize << " bytes from QuicStreamRxBuffer of size = " << m_numBytesInBuffer);

  if (extractSize == 0)
    {
      return 0;
    }

  Ptr<Packet> outPkt = Create<Packet> ();

  QuicStreamRxPacketList::iterator it = m_streamRecvList.begin ();

  while (extractSize > 0 && !m_streamRecvList.empty ()
         && it != m_streamRecvList.end ())
    {
      it = m_streamRecvList.begin ();
      Ptr<Packet> currentPacket = (*it)->m_packet;

      if (currentPacket->GetSize () <= extractSize)   // Merge
        {

          outPkt->AddAtEnd (currentPacket);
          m_streamRecvList.erase (it);
          NS_LOG_LOGIC ("Extracted and removed packet " << (*it)->m_offset << " from RxBuffer, bytes to extract: " << extractSize);

          m_numBytesInBuffer -= currentPacket->GetSize ();
          extractSize -= currentPacket->GetSize ();

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
      NS_LOG_INFO ("Nothing extracted.");
      return 0;
    }

  return outPkt;
}

std::pair<uint64_t, uint64_t>
QuicStreamRxBuffer::GetDeliverable (uint64_t currRecvOffset)
{
  NS_LOG_FUNCTION (this);
  uint64_t offsetToExtract = currRecvOffset;
  uint64_t lengthToExtract = 0;
  NS_LOG_LOGIC ("Calculating deliverable size");

  QuicStreamRxPacketList::iterator i;

  for (i = m_streamRecvList.begin (); i != m_streamRecvList.end (); ++i)
    {
      if ((*i)->m_offset == currRecvOffset + lengthToExtract)
        {
          offsetToExtract = (*i)->m_offset;
          lengthToExtract += (*i)->m_packet->GetSize ();
        }
      NS_LOG_LOGIC ("Inspected packet with offset " << (*i)->m_offset);
    }

  return std::make_pair (offsetToExtract, lengthToExtract);
}

uint32_t
QuicStreamRxBuffer::Size (void) const
{
  NS_LOG_FUNCTION (this);

  // uint32_t inFlight = 0;
  // for (auto recv_it = m_streamRecvList.begin (); recv_it != m_streamRecvList.end () and !m_streamRecvList.empty (); ++recv_it)
  //   {
  //     inFlight += (*recv_it)->m_packet->GetSize ();
  //   }

  return m_numBytesInBuffer;
}

uint32_t
QuicStreamRxBuffer::Available (void) const
{
  return m_maxBuffer - m_numBytesInBuffer;
}

uint32_t
QuicStreamRxBuffer::GetMaxBufferSize (void) const
{
  return m_maxBuffer;
}

void
QuicStreamRxBuffer::SetMaxBufferSize (uint32_t s)
{
  m_maxBuffer = s;
}

uint32_t
QuicStreamRxBuffer::GetFinalSize () const
{
  return m_finalSize;
}

void
QuicStreamRxBuffer::Print (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  QuicStreamRxBuffer::QuicStreamRxPacketList::const_iterator it;
  std::stringstream ss;
  std::stringstream as;

  for (it = m_streamRecvList.begin (); it != m_streamRecvList.end (); ++it)
    {
      (*it)->Print (ss);
    }

  os << "Stream Recv list: \n" << ss.str () << "\n\nCurrent Status: "
     << "\nNumber of receptions = " << m_streamRecvList.size ()
     << "\nReceived Size = " << m_numBytesInBuffer;
  if (m_recvFin)
    {
      os << "\nFinal Size = " << m_finalSize;

    }

}

} //namepsace ns3
