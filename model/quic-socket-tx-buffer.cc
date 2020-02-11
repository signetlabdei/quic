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

#include "quic-socket-tx-buffer.h"

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

NS_LOG_COMPONENT_DEFINE ("QuicSocketTxBuffer");

QuicSocketTxItem::QuicSocketTxItem ()
  : m_packet (0),
    m_packetNumber (0),
    m_lost (false),
    m_retrans (false),
    m_sacked (
      false),
    m_acked (false),
    m_isStream (false),
    m_isStream0 (false),
    m_lastSent (
      Time::Min ())
{

}

QuicSocketTxItem::QuicSocketTxItem (const QuicSocketTxItem &other)
  : m_packet (other.m_packet),
    m_packetNumber (other.m_packetNumber),
    m_lost (
      other.m_lost),
    m_retrans (other.m_retrans),
    m_sacked (other.m_sacked),
    m_acked (other.m_acked),
    m_isStream (
      other.m_isStream),
    m_isStream0 (other.m_isStream0),
    m_lastSent (
      other.m_lastSent)
{

}

void
QuicSocketTxItem::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);
  os << "[SN " << m_packetNumber.GetValue () << " - Last Sent: " << m_lastSent
     << " size " << m_packet->GetSize () << "]";

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

NS_OBJECT_ENSURE_REGISTERED (QuicSocketTxBuffer);

TypeId
QuicSocketTxBuffer::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::QuicSocketTxBuffer").SetParent<Object> ().SetGroupName (
      "Internet").AddConstructor<QuicSocketTxBuffer> ()
//    .AddTraceSource ("UnackSequence",
//                     "First unacknowledged sequence number (SND.UNA)",
//                     MakeTraceSourceAccessor (&QuicSocketTxBuffer::m_sentSize),
//                     "ns3::SequenceNumber32TracedValueCallback")
  ;
  return tid;
}

QuicSocketTxBuffer::QuicSocketTxBuffer ()
  : m_maxBuffer (32768),
    m_appSize (0),
    m_sentSize (0),
    m_numFrameStream0InBuffer (
      0)
{
  m_appList = QuicTxPacketList ();
  m_sentList = QuicTxPacketList ();
}

QuicSocketTxBuffer::~QuicSocketTxBuffer (void)
{
  QuicTxPacketList::iterator it;

  for (it = m_sentList.begin (); it != m_sentList.end (); ++it)
    {
      QuicSocketTxItem *item = *it;
      m_sentSize -= item->m_packet->GetSize ();
      delete item;
    }

  for (it = m_appList.begin (); it != m_appList.end (); ++it)
    {
      QuicSocketTxItem *item = *it;
      m_appSize -= item->m_packet->GetSize ();
      delete item;
    }
}

void
QuicSocketTxBuffer::Print (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  QuicSocketTxBuffer::QuicTxPacketList::const_iterator it;
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

  os << Simulator::Now ().GetSeconds () << "\nApp list: \n" << as.str () << "\n\nSent list: \n" << ss.str ()
     << "\n\nCurrent Status: " << "\nNumber of transmissions = "
     << m_sentList.size () << "\nApplication Size = " << m_appSize
     << "\nSent Size = " << m_sentSize;
}

bool
QuicSocketTxBuffer::Add (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  QuicSubheader qsb;
  uint32_t headerSize = p->PeekHeader (qsb);
  NS_LOG_INFO (
    "Try to append " << p->GetSize () << " bytes " << ", availSize=" << Available () << " offset " << qsb.GetOffset () << " on stream " <<qsb.GetStreamId());

  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {
          QuicSocketTxItem *item = new QuicSocketTxItem ();
          item->m_packet = p->Copy ();
          // check to which stream this packet belongs to
          uint32_t streamId = 0;
          bool isStream = false;
          if (headerSize)
            {
              streamId = qsb.GetStreamId ();
              isStream = qsb.IsStream ();
            }
          else
            {
              NS_ABORT_MSG ("No QuicSubheader in this QUIC frame " << p);
            }
          item->m_isStream = isStream;
          item->m_isStream0 = (streamId == 0);
          m_numFrameStream0InBuffer += (streamId == 0);
          m_appList.insert (m_appList.end (), item);
          m_appSize += p->GetSize ();

          NS_LOG_INFO ("Update: Application Size = " << m_appSize << ", offset " << qsb.GetOffset ());
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
QuicSocketTxBuffer::NextStream0Sequence (const SequenceNumber32 seq)
{
  NS_LOG_FUNCTION (this << seq);

  QuicSocketTxItem* outItem = new QuicSocketTxItem ();

  QuicTxPacketList::iterator it = m_appList.begin ();
  while (it != m_appList.end ())
    {
      Ptr<Packet> currentPacket = (*it)->m_packet;

      if ((*it)->m_isStream0)
        {
          outItem->m_packetNumber = seq;
          outItem->m_lastSent = Now ();
          outItem->m_packet = currentPacket;
          outItem->m_isStream0 = (*it)->m_isStream0;
          m_appList.erase (it);
          m_appSize -= currentPacket->GetSize ();
          m_sentList.insert (m_sentList.end (), outItem);
          m_sentSize += outItem->m_packet->GetSize ();
          --m_numFrameStream0InBuffer;
          Ptr<Packet> toRet = outItem->m_packet->Copy ();
          return toRet;
        }
      it++;
    }
  return 0;
}

Ptr<Packet>
QuicSocketTxBuffer::NextSequence (uint32_t numBytes,
                                  const SequenceNumber32 seq)
{
  NS_LOG_FUNCTION (this << numBytes << seq);

  QuicSocketTxItem* outItem = GetNewSegment (numBytes);

  if (outItem != nullptr)
    {
      NS_LOG_INFO ("Extracting " << outItem->m_packet->GetSize () << " bytes");
      outItem->m_packetNumber = seq;
      outItem->m_lastSent = Now ();
      Ptr<Packet> toRet = outItem->m_packet->Copy ();
      return toRet;
    }
  else
    {
      NS_LOG_INFO ("Empty packet");
      return Create<Packet> ();
    }

}

QuicSocketTxItem*
QuicSocketTxBuffer::GetNewSegment (uint32_t numBytes)
{
  NS_LOG_FUNCTION (this << numBytes);

  bool toInsert = false;
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
          toInsert = true;
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
              NS_LOG_LOGIC ("Not enough bytes even for the header");
              m_appSize += removed;
              break;
            }
          currentPacket->RemoveHeader (qsb);
          uint32_t newPacketSize = (uint32_t)newPacketSizeInt;

          NS_LOG_LOGIC ("Add incomplete frame to the outItem");
          toInsert = true;
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

          NS_LOG_LOGIC ("Split packet, putting second part back in application buffer");

          Ptr<Packet> secondPartPacket = currentItem->m_packet->CreateFragment (
              newPacketSize, newLength);
          secondPartPacket->AddHeader (newQsbToBuffer);

          QuicSocketTxItem *toBeBuffered = new QuicSocketTxItem (*currentItem);
          toBeBuffered->m_packet = secondPartPacket;
          currentItem->m_packet = firstPartPacket;

          MergeItems (*outItem, *currentItem);
          outItemSize += currentItem->m_packet->GetSize ();

          m_appList.erase (it);
          m_appList.push_front (toBeBuffered);
          m_appSize += toBeBuffered->m_packet->GetSize ();
          // check correctness of application size
          uint32_t check = m_appSize;
          for (auto itc = m_appList.begin ();
               itc != m_appList.end () and !m_appList.empty (); ++itc)
            {
              check -= (*itc)->m_packet->GetSize ();
            }
          if (check != 0)
            {
              outItemSize += 0;
            }

          delete currentItem;

          NS_LOG_LOGIC ("Buffer size: " << m_appSize << " (put back " << toBeBuffered->m_packet->GetSize () << " bytes)");

          it = m_appList.begin ();   // restart to identify if there are other packets that can be merged
          break; // at most one segment
        }

      it++;
    }

  if (toInsert)
    {
      NS_LOG_LOGIC ("Adding packet to sent buffer");
      m_sentList.insert (m_sentList.end (), outItem);
      m_sentSize += outItem->m_packet->GetSize ();
    }

  NS_LOG_INFO ("Update: Sent Size = " << m_sentSize << " remaining App Size " << m_appSize << " object size " << outItemSize);

  //Print(std::cout);

  return outItem;
}

std::vector<QuicSocketTxItem*>
QuicSocketTxBuffer::OnAckUpdate (
  Ptr<TcpSocketState> tcb, const uint32_t largestAcknowledged,
  const std::vector<uint32_t> &additionalAckBlocks,
  const std::vector<uint32_t> &gaps)
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> compAckBlocks = additionalAckBlocks;
  std::vector<uint32_t> compGaps = gaps;

  std::vector<QuicSocketTxItem*> newlyAcked;
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));

  compAckBlocks.insert (compAckBlocks.begin (), largestAcknowledged);
  uint32_t ackBlockCount = compAckBlocks.size ();

  std::vector<uint32_t>::const_iterator ack_it = compAckBlocks.begin ();
  std::vector<uint32_t>::const_iterator gap_it = compGaps.begin ();

  std::stringstream gap_print;
  for (auto i = gaps.begin (); i != gaps.end (); ++i)
    {
      gap_print << (*i) << " ";
    }

  std::stringstream block_print;
  for (auto i = compAckBlocks.begin (); i != compAckBlocks.end (); ++i)
    {
      block_print << (*i) << " ";
    }

  NS_LOG_INFO ("Largest ACK: " << largestAcknowledged
                               << ", blocks: " << block_print.str () << ", gaps: " << gap_print.str ());

  // Iterate over the ACK blocks and gaps
  for (uint32_t numAckBlockAnalyzed = 0; numAckBlockAnalyzed < ackBlockCount;
       ++numAckBlockAnalyzed, ++ack_it, ++gap_it)
    {
      for (auto sent_it = m_sentList.rbegin ();
           sent_it != m_sentList.rend () and !m_sentList.empty (); ++sent_it)  // Visit sentList in reverse Order for optimization
        {
          NS_LOG_LOGIC ("Consider packet " << (*sent_it)->m_packetNumber
                                           << " (ACK block " << SequenceNumber32 ((*ack_it)) << ")");
          // The packet is in the next gap
          bool inGap = (gap_it != compGaps.end ()) && ((*sent_it)->m_packetNumber <= SequenceNumber32 ((*gap_it)));
          if (inGap)   // Just for optimization we suppose All is perfectly ordered
            {
              NS_LOG_LOGIC ("Packet " << (*sent_it)->m_packetNumber << " missing");
              break;
            }
          // The packet is in the current block: ACK it
          NS_LOG_LOGIC ("Packet " << (*sent_it)->m_packetNumber << " ACKed");
          bool notInGap = ((gap_it == compGaps.end ()) || ((*sent_it)->m_packetNumber > SequenceNumber32 ((*gap_it))));

          if ((*sent_it)->m_packetNumber <= SequenceNumber32 ((*ack_it))
              and notInGap
              and (*sent_it)->m_sacked == false)
            {
              (*sent_it)->m_sacked = true;
              (*sent_it)->m_ackTime = Now ();
              newlyAcked.push_back ((*sent_it));
              UpdateRateSample ((*sent_it));
            }

        }
    }
  NS_LOG_LOGIC ("Mark lost packets");
  // Mark packets as lost as in RFC (Sec. 4.2.1 of draft-ietf-quic-recovery-15)
  uint32_t index = m_sentList.size ();
  bool lost = false;
  bool outstanding = false;
  auto acked_it = m_sentList.rend ();
  // Iterate over the sent packet list in reverse
  for (auto sent_it = m_sentList.rbegin ();
       sent_it != m_sentList.rend () and !m_sentList.empty ();
       ++sent_it, --index)
    {
      // All previous packets are lost
      if (lost)
        {
          if (!(*sent_it)->m_sacked)
            {
              (*sent_it)->m_lost = true;
              NS_LOG_LOGIC ("Packet " << (*sent_it)->m_packetNumber << " lost");
            }
        }
      else
        {
          // The packet is the last ACKed packet
          if ((*sent_it)->m_packetNumber.GetValue () == largestAcknowledged)
            {
              // Mark the packet as ACKed
              acked_it = sent_it;
              outstanding = true;
            }
          else if (outstanding && !(*sent_it)->m_sacked)
            {
              //ACK-based detection
              if (largestAcknowledged - (*sent_it)->m_packetNumber.GetValue () >= tcbd->m_kReorderingThreshold)
                {
                  (*sent_it)->m_lost = true;
                  lost = true;
                  NS_LOG_INFO ("Largest ACK " << largestAcknowledged << ", lost packet " << (*sent_it)->m_packetNumber.GetValue () << " - reordering " << tcbd->m_kReorderingThreshold);
                }
              // Time-based detection (optional)
              if (tcbd->m_kUsingTimeLossDetection)
                {
                  double lhsComparison = ((*acked_it)->m_ackTime - (*sent_it)->m_lastSent).GetSeconds();
                  double rhsComparison = tcbd->m_kTimeReorderingFraction * tcbd->m_smoothedRtt.GetSeconds();
                  if (lhsComparison >= rhsComparison)
                    {
                      NS_LOG_UNCOND ("Largest ACK " << largestAcknowledged << ", lost packet " << (*sent_it)->m_packetNumber.GetValue () << " - time " << rhsComparison);
                      (*sent_it)->m_lost = true;
                      lost = true;
                    }
                }
            }
        }
    }

  // Clean up acked packets and return new ACKed packet vector
  CleanSentList ();
  return newlyAcked;
}

void
QuicSocketTxBuffer::ResetSentList (uint32_t keepItems)
{
  NS_LOG_FUNCTION (this << keepItems);
  uint32_t kept = 0;
  for (auto sent_it = m_sentList.rbegin ();
       sent_it != m_sentList.rend () and !m_sentList.empty ();
       ++sent_it, kept++)
    {
      if (kept >= keepItems && !(*sent_it)->m_sacked)
        {
          (*sent_it)->m_lost = true;
        }
    }
}

bool
QuicSocketTxBuffer::MarkAsLost (const SequenceNumber32 seq)
{
  NS_LOG_FUNCTION (this << seq);
  bool found = false;
  for (auto sent_it = m_sentList.begin ();
       sent_it != m_sentList.end () and !m_sentList.empty (); ++sent_it)
    {
      if ((*sent_it)->m_packetNumber == seq)
        {
          found = true;
          (*sent_it)->m_lost = true;
        }
    }
  return found;
}

uint32_t
QuicSocketTxBuffer::Retransmission (SequenceNumber32 packetNumber)
{
  NS_LOG_FUNCTION (this);
  uint32_t toRetx = 0;
  // First pass: add lost packets to the application buffer
  for (auto sent_it = m_sentList.rbegin (); sent_it != m_sentList.rend ();
       ++sent_it)
    {
      QuicSocketTxItem *item = *sent_it;
      if (item->m_lost)
        {
          // Add lost packet contents to app buffer
          QuicSocketTxItem *retx = new QuicSocketTxItem ();
          retx->m_packetNumber = packetNumber;
          retx->m_isStream = item->m_isStream;
          retx->m_isStream0 = item->m_isStream0;
          retx->m_packet = Create<Packet> ();
          NS_LOG_LOGIC ("Add packet " << retx->m_packetNumber.GetValue () << " to retx packet");
          MergeItems (*retx, *item);
          retx->m_lost = false;
          retx->m_retrans = true;
          m_appList.insert (m_appList.begin (), retx);
          m_appSize += retx->m_packet->GetSize ();
          toRetx += retx->m_packet->GetSize ();
          NS_LOG_INFO ("Retransmit packet " << (*sent_it)->m_packetNumber);
        }
    }

  NS_LOG_LOGIC ("Remove retransmitted packets from sent list");
  auto sent_it = m_sentList.begin ();
  // Remove lost packets from the sent list
  while (!m_sentList.empty () && sent_it != m_sentList.end ())
    {
      QuicSocketTxItem *item = *sent_it;
      if (item->m_lost)
        {
          // Remove lost packet from sent vector
          m_sentSize -= item->m_packet->GetSize ();
          sent_it = m_sentList.erase (sent_it);
        }
      else
        {
          sent_it++;
        }
    }
  return toRetx;
}

std::vector<QuicSocketTxItem*>
QuicSocketTxBuffer::DetectLostPackets ()
{
  NS_LOG_FUNCTION (this);
  std::vector<QuicSocketTxItem*> lost;

  for (auto sent_it = m_sentList.begin ();
       sent_it != m_sentList.end () and !m_sentList.empty (); ++sent_it)
    {
      if ((*sent_it)->m_lost)
        {
          lost.push_back ((*sent_it));
          NS_LOG_INFO ("Packet " << (*sent_it)->m_packetNumber << " is lost");
        }
    }
  return lost;
}

uint32_t
QuicSocketTxBuffer::GetLost ()
{
  NS_LOG_FUNCTION (this);
  uint32_t lostCount = 0;
  for (auto sent_it = m_sentList.begin ();
       sent_it != m_sentList.end() and !m_sentList.empty (); ++sent_it)
    {
      if ((*sent_it)->m_lost)
        {
          lostCount += (*sent_it)->m_packet->GetSize ();
        }
    }
  return lostCount;
}

void
QuicSocketTxBuffer::CleanSentList ()
{
  NS_LOG_FUNCTION (this);
  auto sent_it = m_sentList.begin ();
  // All packets up to here are ACKed (already sent to the receiver app)
  while (!m_sentList.empty () && (*sent_it)->m_sacked && !(*sent_it)->m_lost)
    {
      // Remove ACKed packet from sent vector
      QuicSocketTxItem *item = *sent_it;
      item->m_acked = true;
      m_sentSize -= item->m_packet->GetSize ();
      m_sentList.erase (sent_it);
      NS_LOG_LOGIC ("Packet " << (*sent_it)->m_packetNumber << " received and ACKed. Removing from sent buffer");
      sent_it = m_sentList.begin ();
    }
}

void
QuicSocketTxBuffer::MergeItems (QuicSocketTxItem &t1,
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
QuicSocketTxBuffer::Available (void) const
{
  return m_maxBuffer - m_appSize;
}

uint32_t
QuicSocketTxBuffer::GetMaxBufferSize (void) const
{
  return m_maxBuffer;
}

void
QuicSocketTxBuffer::SetMaxBufferSize (uint32_t n)
{
  m_maxBuffer = n;
}

uint32_t
QuicSocketTxBuffer::AppSize (void) const
{
  return m_appSize;
}

uint32_t
QuicSocketTxBuffer::GetNumFrameStream0InBuffer (void) const
{
  return m_numFrameStream0InBuffer;
}

uint32_t
QuicSocketTxBuffer::BytesInFlight () const
{
  NS_LOG_FUNCTION (this);

  uint32_t inFlight = 0;

  for (auto sent_it = m_sentList.begin ();
       sent_it != m_sentList.end () and !m_sentList.empty (); ++sent_it)
    {
      if (!(*sent_it)->m_isStream0 && (*sent_it)->m_isStream
          && !(*sent_it)->m_sacked)
        {
          inFlight += (*sent_it)->m_packet->GetSize ();
        }
    }

  NS_LOG_INFO ("Compute bytes in flight " << inFlight
                                          << " m_sentSize " << m_sentSize
                                          << " m_appSize " << m_appSize);
  return inFlight;

}

void
QuicSocketTxBuffer::SetQuicSocketState (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this);
  m_tcb = tcb;
}

void
QuicSocketTxBuffer::UpdatePacketSent (SequenceNumber32 seq, uint32_t sz)
{
  NS_LOG_FUNCTION (this << seq << sz);

  if (m_tcb == nullptr or sz == 0)
    {
      return;
    }

  if (m_tcb->m_bytesInFlight.Get () == 0)
    {
      m_tcb->m_firstSentTime = Simulator::Now ();
      m_tcb->m_deliveredTime = Simulator::Now ();
    }

  QuicSocketTxItem *item = nullptr;
  for (auto it = m_sentList.rbegin (); it != m_sentList.rend (); ++it)
    {
      if ((*it)->m_packetNumber == seq)
        {
          item = *it;
          break;
        }
    }
  NS_ASSERT_MSG (item != nullptr, "not found seq " << seq);
  item->m_firstSentTime = m_tcb->m_firstSentTime;
  item->m_deliveredTime = m_tcb->m_deliveredTime;
  item->m_isAppLimited  = (m_tcb->m_appLimitedUntil > m_tcb->m_delivered);
  item->m_delivered     = m_tcb->m_delivered;
  item->m_ackBytesSent  = m_tcb->m_ackBytesSent;
}

void
QuicSocketTxBuffer::UpdateAckSent (SequenceNumber32 seq, uint32_t sz)
{
  NS_UNUSED (seq);
  if (m_tcb == nullptr or sz == 0)
    {
      return;
    }
  
  m_tcb->m_ackBytesSent += sz;
}

struct RateSample*
QuicSocketTxBuffer::GetRateSample ()
{
  NS_LOG_FUNCTION (this);
  return &m_rs;
}

void
QuicSocketTxBuffer::UpdateRateSample (QuicSocketTxItem *item)
{
  NS_LOG_FUNCTION (this << item);

  if (m_tcb == nullptr or item->m_deliveredTime == Time::Max ()) // item already SACKed
    {
      return;
    }

  m_tcb->m_delivered         += item->m_packet->GetSize ();;
  m_tcb->m_deliveredTime      = Simulator::Now ();

  if (item->m_delivered > m_rs.m_priorDelivered)
    {
      m_rs.m_priorDelivered   = item->m_delivered;
      m_rs.m_priorTime        = item->m_deliveredTime;
      m_rs.m_isAppLimited     = item->m_isAppLimited;
      m_rs.m_sendElapsed      = item->m_lastSent - item->m_firstSentTime;
      m_rs.m_ackElapsed       = m_tcb->m_deliveredTime - item->m_deliveredTime;
      m_tcb->m_firstSentTime  = item->m_lastSent;
      m_rs.m_priorAckBytesSent  = item->m_ackBytesSent;
    }

  /* Mark the packet as delivered once it is SACKed to avoid
   * being used again when it's cumulatively acked. 
   */
  item->m_deliveredTime = Time::Max ();
  m_tcb->m_txItemDelivered = item->m_delivered;
}

bool
QuicSocketTxBuffer::GenerateRateSample ()
{
  NS_LOG_FUNCTION (this);

  if (m_tcb == nullptr)
    {
      return false;
    }

  if (m_rs.m_priorTime == Seconds (0))
    {
      return false;
    }

  m_rs.m_interval = std::max (m_rs.m_sendElapsed, m_rs.m_ackElapsed);

  m_rs.m_delivered = m_tcb->m_delivered - m_rs.m_priorDelivered;


  if (m_rs.m_ackBytesSent < m_tcb->m_ackBytesSent - m_rs.m_priorAckBytesSent or ++m_rs.m_ackBytesSentWin >= 5) //quick maxfilter implementation
    {
      m_rs.m_ackBytesSent = m_tcb->m_ackBytesSent - m_rs.m_priorAckBytesSent;
      m_rs.m_ackBytesSentWin = 0;
    }

  uint32_t discountedDelivered = m_rs.m_delivered > m_rs.m_ackBytesSent ? m_rs.m_delivered - m_rs.m_ackBytesSent : 0U;

  if (m_rs.m_interval < m_tcb->m_minRtt)
    {
      m_rs.m_interval = Seconds (0);
      return false;
    }

  if (m_rs.m_interval != Seconds (0))
    {
      m_rs.m_deliveryRate = DataRate (discountedDelivered * 8.0 / m_rs.m_interval.GetSeconds ());
    }
  NS_LOG_DEBUG ("computed delivery rate: " << m_rs.m_deliveryRate);
  return true;
}

}
