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

#include "quic-socket-tx-scheduler.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include "ns3/simulator.h"

#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "quic-subheader.h"
#include "quic-socket-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicSocketTxScheduler");

NS_OBJECT_ENSURE_REGISTERED (QuicSocketTxScheduler);

TypeId
QuicSocketTxScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketTxScheduler")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

QuicSocketTxScheduler::QuicSocketTxScheduler () : Object ()
{
}

QuicSocketTxScheduler::QuicSocketTxScheduler (const QuicSocketTxScheduler &other) : Object (other)
{
}

QuicSocketTxScheduler::~QuicSocketTxScheduler ()
{
}

// FIFO queue

NS_OBJECT_ENSURE_REGISTERED (QuicSocketTxFifoScheduler);

TypeId
QuicSocketTxFifoScheduler::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::QuicSocketTxFifoScheduler").SetParent<QuicSocketTxScheduler> ().SetGroupName (
      "Internet").AddConstructor<QuicSocketTxFifoScheduler> ()
  ;
  return tid;
}

QuicSocketTxFifoScheduler::QuicSocketTxFifoScheduler () : QuicSocketTxScheduler (),
		m_appSize (0)
{
  m_appList = QuicTxPacketList ();
}

QuicSocketTxFifoScheduler::QuicSocketTxFifoScheduler (const QuicSocketTxFifoScheduler &other) : QuicSocketTxScheduler (other),
		m_appSize(other.m_appSize)
{
  m_appList = other.m_appList;
}

QuicSocketTxFifoScheduler::~QuicSocketTxFifoScheduler (void)
{
  QuicTxPacketList::iterator it;

  for (it = m_appList.begin (); it != m_appList.end (); ++it)
    {
      QuicSocketTxItem *item = *it;
      m_appSize -= item->m_packet->GetSize ();
      delete item;
    }
}

void
QuicSocketTxFifoScheduler::Print (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  QuicSocketTxFifoScheduler::QuicTxPacketList::const_iterator it;
  std::stringstream ss;
  std::stringstream as;

  for (it = m_appList.begin (); it != m_appList.end (); ++it)
    {
      (*it)->Print (as);
    }

  os << Simulator::Now ().GetSeconds () << "\nApp list: \n" << as.str () << "\n\nSent list: \n" << ss.str ()
     << "\n\nCurrent Status: \nApplication Size = " << m_appSize;
}

void
QuicSocketTxFifoScheduler::Add (QuicSocketTxItem* item, bool retx)
{
  NS_LOG_FUNCTION (this << item);

  QuicSubheader qsb;
  item->m_packet->PeekHeader (qsb);
  NS_LOG_INFO("Adding packet on stream "<<qsb.GetStreamId());
  if (!retx)
  {
	NS_LOG_INFO("Standard item, add at end (offset "<<qsb.GetOffset()<< ")");
	m_appList.insert (m_appList.end (), item);
  }
  else
  {
	NS_LOG_INFO("Retransmitted item, add at beginning (offset "<<qsb.GetOffset() << ")");
	m_appList.insert (m_appList.begin (), item);
  }
  m_appSize += item->m_packet->GetSize ();
}

QuicSocketTxItem*
QuicSocketTxFifoScheduler::GetNewSegment (uint32_t numBytes)
{
  NS_LOG_FUNCTION (this << numBytes);

  bool firstSegment = true;
  Ptr<Packet> currentPacket = 0;
  QuicSocketTxItem *currentItem = 0;
  QuicSocketTxItem *outItem = new QuicSocketTxItem ();
  outItem->m_isStream = true;   // Packets sent with this method are always stream packets
  outItem->m_isStream0 = false;
  outItem->m_packet = Create<Packet> ();
  uint32_t outItemSize = 0;
  QuicTxPacketList::iterator it = m_appList.begin ();

  while (it != m_appList.end () && outItemSize < numBytes)
    {
      currentItem = *it;
      currentPacket = currentItem->m_packet;

      if (outItemSize + currentItem->m_packet->GetSize ()   /*- subheaderSize*/
          <= numBytes)       // Merge
        {
          NS_LOG_LOGIC ("Add complete frame to the outItem - size "
                        << currentItem->m_packet->GetSize ()
                        << " m_appSize " << m_appSize);
          QuicSubheader qsb;
          currentPacket->PeekHeader (qsb);
          NS_LOG_INFO("Packet: stream " <<qsb.GetStreamId() << ", offset "<<qsb.GetOffset());
          MergeItems (*outItem, *currentItem);
          outItemSize += currentItem->m_packet->GetSize ();

          m_appList.erase (it);
          m_appSize -= currentItem->m_packet->GetSize ();

          delete currentItem;

          it = m_appList.begin ();   // restart to identify if there are other packets that can be merged
          NS_LOG_LOGIC ("Updating application buffer size: " << m_appSize);
          continue;
        }
      else if (firstSegment)  // we cannot transmit a full packet, so let's split it and update the subheaders
        {
          firstSegment = false;

          // subtract the whole packet from m_appSize, then add the remaining fragment (need to account for headers)
          uint32_t removed = currentItem->m_packet->GetSize ();
          m_appSize -= removed;

          // get the currentPacket subheader
          QuicSubheader qsb;
          currentPacket->PeekHeader (qsb);

          // new packet size
          int newPacketSizeInt = (int)numBytes - outItemSize - qsb.GetSerializedSize ();
          if (newPacketSizeInt <= 0)
            {
              NS_LOG_INFO ("Not enough bytes even for the header");
              m_appSize += removed;
              break;
            }
          NS_LOG_INFO("Split packet on stream " <<qsb.GetStreamId() << ", sending " << newPacketSizeInt << " bytes from offset "<<qsb.GetOffset());

          currentPacket->RemoveHeader (qsb);
          uint32_t newPacketSize = (uint32_t)newPacketSizeInt;

          NS_LOG_LOGIC ("Add incomplete frame to the outItem");
          uint32_t totPacketSize = currentItem->m_packet->GetSize ();
          NS_LOG_LOGIC ("Extracted " << outItemSize << " bytes");

          uint32_t oldOffset = qsb.GetOffset ();
          uint32_t newOffset = oldOffset + newPacketSize;
          bool oldOffBit = !(oldOffset == 0);
          bool newOffBit = true;
          uint32_t oldLength = qsb.GetLength ();
          uint32_t newLength = 0;
          bool newLengthBit = true;
          newLength = totPacketSize - newPacketSize;
          if (oldLength == 0)
            {
              newLengthBit = false;
            }
          bool lengthBit = true;
          bool oldFinBit = qsb.IsStreamFin ();
          bool newFinBit = false;

          QuicSubheader newQsbToTx = QuicSubheader::CreateStreamSubHeader (qsb.GetStreamId (),
                                                                           oldOffset, newPacketSize, oldOffBit, lengthBit, newFinBit);
          QuicSubheader newQsbToBuffer = QuicSubheader::CreateStreamSubHeader (qsb.GetStreamId (),
                                                                               newOffset, newLength, newOffBit, newLengthBit, oldFinBit);
          newQsbToTx.SetMaxStreamData (qsb.GetMaxStreamData ());
          newQsbToBuffer.SetMaxStreamData (qsb.GetMaxStreamData ());

          Ptr<Packet> firstPartPacket = currentItem->m_packet->CreateFragment (
              0, newPacketSize);
          NS_ASSERT_MSG (firstPartPacket->GetSize () == newPacketSize,
                         "Wrong size " << firstPartPacket->GetSize ());
          firstPartPacket->AddHeader (newQsbToTx);
          firstPartPacket->Print (std::cerr);

          NS_LOG_INFO("Split packet, putting second part back in application buffer - stream " <<newQsbToBuffer.GetStreamId() << ", storing from offset "<<newQsbToBuffer.GetOffset());


          Ptr<Packet> secondPartPacket = currentItem->m_packet->CreateFragment (
              newPacketSize, newLength);
          secondPartPacket->AddHeader (newQsbToBuffer);

          QuicSocketTxItem *toBeBuffered = new QuicSocketTxItem (*currentItem);
          toBeBuffered->m_packet = secondPartPacket;
          currentItem->m_packet = firstPartPacket;

          MergeItems (*outItem, *currentItem);
          outItemSize += currentItem->m_packet->GetSize ();

          m_appList.erase (it);
          m_appList.insert (m_appList.begin(), toBeBuffered);
          m_appSize += toBeBuffered->m_packet->GetSize ();
          // check correctness of application size
          uint32_t check = m_appSize;
          for (auto itc = m_appList.begin ();
               itc != m_appList.end () and !m_appList.empty (); ++itc)
            {
              check -= (*itc)->m_packet->GetSize ();
            }
          NS_ASSERT(check == 0);

          delete currentItem;

          NS_LOG_LOGIC ("Buffer size: " << m_appSize << " (put back " << toBeBuffered->m_packet->GetSize () << " bytes)");
          break; // at most one segment
        }

      it++;
    }

  NS_LOG_INFO ("Update: remaining App Size " << m_appSize << ", object size " << outItemSize);

  //Print(std::cout);

  return outItem;
}



void
QuicSocketTxFifoScheduler::MergeItems (QuicSocketTxItem &t1,
                                QuicSocketTxItem &t2) const
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
  if (t1.m_acked == true && t2.m_acked == true)
    {
      t1.m_acked = true;
    }
  else
    {
      t1.m_acked = false;
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
  if (t1.m_ackTime > t2.m_ackTime)
    {
      t1.m_ackTime = t2.m_ackTime;
    }

  t1.m_packet->AddAtEnd (t2.m_packet);
}

uint32_t
QuicSocketTxFifoScheduler::AppSize (void) const
{
  return m_appSize;
}


}
