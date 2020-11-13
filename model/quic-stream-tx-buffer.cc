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

#include <algorithm>
#include <iostream>

#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "quic-subheader.h"
#include "quic-stream-tx-buffer.h"
#include "quic-socket-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicStreamTxBuffer");

QuicStreamTxItem::QuicStreamTxItem ()
  : m_packetNumberSequence (0),
    m_packet (0),
    m_lost (false),
    m_retrans (false),
    m_sacked (false),
    m_lastSent (Time::Min ()),
    m_id (0)
{}

QuicStreamTxItem::QuicStreamTxItem (const QuicStreamTxItem &other)
  : m_packetNumberSequence (other.m_packetNumberSequence),
    m_packet (other.m_packet),
    m_lost (other.m_lost),
    m_retrans (other.m_retrans),
    m_sacked (other.m_sacked),
    m_lastSent (other.m_lastSent),
    m_id (other.m_id)
{}

QuicStreamTxItem::~QuicStreamTxItem ()
{}

void
QuicStreamTxItem::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);
  os << "[ID " << m_id << " SN " << m_packetNumberSequence.GetValue () << " - Last Sent: " << m_lastSent << "]";

  if (m_lost)
    {
      os << "|lost|";
    }
  if (m_retrans)
    {
      os << "|retr|";
    }
  if (m_sacked)
    {
      os << "|ackd|";
    }
}

NS_OBJECT_ENSURE_REGISTERED (QuicStreamTxBuffer);

TypeId
QuicStreamTxBuffer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicStreamTxBuffer")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicStreamTxBuffer> ()
//    .AddTraceSource ("UnackSequence",
//                     "First unacknowledged sequence number (SND.UNA)",
//                     MakeTraceSourceAccessor (&QuicStreamTxBuffer::m_sentSize),
//                     "ns3::SequenceNumber32TracedValueCallback")
  ;
  return tid;
}

QuicStreamTxBuffer::QuicStreamTxBuffer ()
  : m_maxBuffer (131072),
    m_appSize (0),
    m_sentSize (0)
{
  m_appList = QuicTxPacketList ();
  m_sentList = QuicTxPacketList ();
}

QuicStreamTxBuffer::~QuicStreamTxBuffer (void)
{}

void
QuicStreamTxBuffer::Print (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  QuicStreamTxBuffer::QuicTxPacketList::const_iterator it;
  std::stringstream ss;
  std::stringstream as;

  for (it = m_sentList.begin (); it != m_sentList.end (); ++it)
    {
      (*it)->Print (ss);
    }

  for (it = m_appList.begin (); it != m_appList.end (); ++it)
    {
      (*it)->Print (as);
    }

  os << "App list: \n" << as.str () <<
    "\n\nSent list: \n" << ss.str () <<
    "\n\nCurrent Status: " <<
    "\nNumber of transmissions = " << m_sentList.size () <<
    "\nApplication Size = " << m_appSize <<
    "\nSent Size = " << m_sentSize;
}

bool
QuicStreamTxBuffer::Add (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_INFO ("Try to append " << p->GetSize () << " bytes " << ", availSize=" << Available ());

  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {
          Ptr<QuicStreamTxItem> item = CreateObject<QuicStreamTxItem> ();
          item->m_packet = p;
          m_appList.insert (m_appList.end (), item);
          m_appSize += p->GetSize ();

          NS_LOG_INFO ("Update: Application Size = " << m_appSize);
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

bool
QuicStreamTxBuffer::Rejected (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  QuicSubheader qsb;
  p->PeekHeader (qsb);
  NS_LOG_INFO ("Packet of size " << p->GetSize () << " bytes rejected, reappending. Offset " << qsb.GetOffset ());

  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {
          Ptr<QuicStreamTxItem> item = CreateObject<QuicStreamTxItem> ();
          item->m_packet = p;
          m_appList.insert (m_appList.begin (), item);
          m_appSize += p->GetSize ();
          m_sentList.pop_back ();
          m_sentSize -= p->GetSize ();
          Ptr<Packet> added = (*m_appList.begin ())->m_packet;
          added->PeekHeader (qsb);
          NS_LOG_INFO ("Update: Application Size = " << m_appSize << " added packet with offset " << qsb.GetOffset ());
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
QuicStreamTxBuffer::NextSequence (uint32_t numBytes, const SequenceNumber32 seq)
{
  NS_LOG_FUNCTION (this << numBytes << seq);

  Ptr<QuicStreamTxItem> outItem = GetNewSegment (numBytes);

  if (outItem != nullptr)
    {
      outItem->m_packetNumberSequence = seq;
      outItem->m_lastSent = Simulator::Now ();
      Ptr<Packet> toRet = outItem->m_packet->Copy ();
      return toRet;
    }
  else
    {
      return Create<Packet> ();
    }

}

Ptr<QuicStreamTxItem>
QuicStreamTxBuffer::GetNewSegment (uint32_t numBytes)
{
  NS_LOG_FUNCTION (this << numBytes);

  bool toInsert = false;
  Ptr<Packet> currentPacket = 0;
  Ptr<QuicStreamTxItem> currentItem = 0;
  Ptr<QuicStreamTxItem> outItem = CreateObject<QuicStreamTxItem> ();
  outItem->m_packet = Create<Packet> ();
  uint32_t outItemSize = 0;
  QuicTxPacketList::iterator it = m_appList.begin ();

  while (it != m_appList.end () && outItemSize < numBytes)
    {
      currentItem = *it;
      currentPacket = currentItem->m_packet;

//	  uint32_t subheaderSize = 0;
//	  if(m_TxBufferType == SOCKET)
//	  {
//		  QuicSubheader sub;
//		  currentItem->m_packet->PeekHeader(sub);
//		  subheaderSize = sub.GetSerializedSize();
//	  }

      if (outItemSize + currentItem->m_packet->GetSize () /*- subheaderSize*/ <= numBytes)   // Merge
        {
          QuicSubheader qsb;
          NS_LOG_LOGIC ("Extracting packet from stream TX buffer");
          toInsert = true;
          MergeItems (*outItem, *currentItem);
          outItemSize += currentItem->m_packet->GetSize ();

          m_appList.erase (it);
          m_appSize -= currentItem->m_packet->GetSize ();

          it = m_appList.begin (); // Restart to find other possible merges
          continue;
        }
      else
        {
          // The packet is too large, split it
          uint32_t split = numBytes - outItemSize;

          Ptr<QuicStreamTxItem> toBeBuffered = CreateObject<QuicStreamTxItem> ();
          SplitItems (*currentItem, *toBeBuffered, split);

          // Add left part of the split to subframe
          NS_LOG_LOGIC ("Add incomplete subframe to the outItem");
          MergeItems (*outItem, *currentItem);
          outItemSize += currentItem->m_packet->GetSize ();

          m_appList.erase (it);
          m_appSize -= currentItem->m_packet->GetSize ();

          m_appList.push_front (toBeBuffered);

        }

      it++;
    }

  if (toInsert)
    {
      m_sentList.insert (m_sentList.end (), outItem);
      m_sentSize += outItem->m_packet->GetSize ();
    }

  NS_LOG_INFO ("Update: Sent Size = " << m_sentSize);

  return outItem;
}

void
QuicStreamTxBuffer::OnAckUpdate (const uint64_t largestAcknowledged, const std::vector<uint64_t> &additionalAckBlocks, const std::vector<uint64_t> &gaps)
{
  NS_LOG_FUNCTION (this);
  std::vector<uint64_t> compAckBlocks = additionalAckBlocks;
  std::vector<uint64_t> compGaps = additionalAckBlocks;

  NS_LOG_INFO ("Handling Ack - highest packet " << largestAcknowledged);
  compAckBlocks.insert (compAckBlocks.begin (), largestAcknowledged);
  compGaps.push_back (0);
  uint64_t ackBlockCount = compAckBlocks.size ();

  std::vector<uint64_t>::const_iterator ack_it = compAckBlocks.begin ();
  std::vector<uint64_t>::const_iterator gap_it = compGaps.begin ();

  for (uint64_t numAckBlockAnalyzed = 0; numAckBlockAnalyzed < ackBlockCount; numAckBlockAnalyzed++, ack_it++, gap_it++)
    {
      for (auto sent_it = m_sentList.rbegin (); sent_it != m_sentList.rend () and !m_sentList.empty (); ++sent_it)       // Visit sentList in reverse Order for optimization
        {
          if ((*sent_it)->m_packetNumberSequence < (SequenceNumber32)(*gap_it) )              // Just for optimization we suppose All is perfectly ordered
            {
              break;
            }

          if ((*sent_it)->m_packetNumberSequence <= (SequenceNumber32)(*ack_it) and (*sent_it)->m_packetNumberSequence > (SequenceNumber32)(*gap_it) and (*sent_it)->m_sacked == false)
            {
              NS_LOG_LOGIC ("Acked packet " << (*sent_it)->m_packetNumberSequence);
              (*sent_it)->m_sacked = true;
            }

        }
    }
}


void
QuicStreamTxBuffer::MergeItems (QuicStreamTxItem &t1, QuicStreamTxItem &t2) const
{
  NS_LOG_FUNCTION (this);

  if (t1.m_sacked == true && t2.m_sacked == true)
    {
      t1.m_sacked = true;
    }
  else
    {
      t1.m_sacked = false;
    }

  if (t2.m_retrans == true && t1.m_retrans == false)
    {
      t1.m_retrans = true;
    }
  if (t1.m_lastSent < t2.m_lastSent)
    {
      t1.m_lastSent = t2.m_lastSent;
    }
  if (t2.m_lost)
    {
      t1.m_lost = true;
    }

  t1.m_packet->AddAtEnd (t2.m_packet);
}

void
QuicStreamTxBuffer::SplitItems (QuicStreamTxItem &t1, QuicStreamTxItem &t2, uint32_t size) const
{
  uint32_t initialSize = t1.m_packet->GetSize ();

  t2.m_sacked = t1.m_sacked;
  t2.m_retrans = t1.m_retrans;
  t2.m_lastSent = t1.m_lastSent;
  t2.m_lost = t1.m_lost;
  if (t1.m_lastSent < t2.m_lastSent)
    {
      t1.m_lastSent = t2.m_lastSent;
    }
  if (t2.m_lost)
    {
      t1.m_lost = true;
    }
  // Copy the packet into t2
  t2.m_packet = t1.m_packet->Copy ();
  // Remove the first size bytes from t2
  t2.m_packet->RemoveAtStart (size);

  NS_ASSERT_MSG (t2.m_packet->GetSize () == initialSize - size,
                 "Wrong size " << t2.m_packet->GetSize ());
  // Remove the bytes from size to end from t1
  t1.m_packet->RemoveAtEnd (t1.m_packet->GetSize () - size);
  NS_ASSERT_MSG (t1.m_packet->GetSize () == size,
                 "Wrong size " << t1.m_packet->GetSize ());
}


uint32_t
QuicStreamTxBuffer::Available (void) const
{
  return m_maxBuffer - m_appSize;
}

uint32_t
QuicStreamTxBuffer::GetMaxBufferSize (void) const
{
  return m_maxBuffer;
}

void
QuicStreamTxBuffer::SetMaxBufferSize (uint32_t n)
{
  m_maxBuffer = n;
}

uint32_t
QuicStreamTxBuffer::AppSize (void) const
{
  return m_appSize;
}

uint32_t
QuicStreamTxBuffer::BytesInFlight () const
{
  NS_LOG_FUNCTION (this);

  return m_sentSize;

}


}
