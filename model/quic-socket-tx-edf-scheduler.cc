/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 SIGNET Lab, Department of Information Engineering, University of Padova
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
 * Authors: Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Umberto Paro <umberto.paro@me.com>
 *
 */

#include "quic-socket-tx-edf-scheduler.h"

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

NS_LOG_COMPONENT_DEFINE ("QuicSocketTxEdfScheduler");

NS_OBJECT_ENSURE_REGISTERED (QuicSocketTxEdfScheduler);

TypeId QuicSocketTxEdfScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketTxEdfScheduler")
    .SetParent<QuicSocketTxScheduler>()
    .SetGroupName ("Internet")
    .AddConstructor<QuicSocketTxEdfScheduler>()
    .AddAttribute ("RetxFirst", "Prioritize retransmissions regardless of stream",
                   BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketTxEdfScheduler::m_retxFirst),
                   MakeBooleanChecker ())
  ;
  return tid;
}

QuicSocketTxEdfScheduler::QuicSocketTxEdfScheduler () :
  QuicSocketTxScheduler (), m_retxFirst (false)
{
  m_defaultLatency = Seconds (0.1);
}

QuicSocketTxEdfScheduler::QuicSocketTxEdfScheduler (
  const QuicSocketTxEdfScheduler &other) :
  QuicSocketTxScheduler (other), m_retxFirst (
    other.m_retxFirst)
{
  m_defaultLatency = other.m_defaultLatency;
  m_latencyMap = other.m_latencyMap;
}

QuicSocketTxEdfScheduler::~QuicSocketTxEdfScheduler (void)
{}

void QuicSocketTxEdfScheduler::Add (Ptr<QuicSocketTxItem> item, bool retx)
{
  NS_LOG_FUNCTION (this << item);

  if (retx)
    {
      if (m_retxFirst)
        {
          QuicSubheader sub;
          item->m_packet->PeekHeader (sub);
          NS_LOG_INFO ("Adding retransmitted packet with highest priority");
          AddScheduleItem (CreateObject<QuicSocketTxScheduleItem> (sub.GetStreamId (), sub.GetOffset (), -1, item), retx);
        }
      else
        {
          uint32_t dataSizeByte = item->m_packet->GetSize ();
          QuicSubheader sub;
          item->m_packet->PeekHeader (sub);
          if (sub.GetSerializedSize () + sub.GetLength ()
              < dataSizeByte)
            {

              NS_LOG_INFO (
                "Disgregate packet to be retransmitted" << dataSizeByte << "; first fragment size" << sub.GetSerializedSize () + sub.GetLength ());
              Ptr<Packet> remaining = item->m_packet->Copy ();

              // the packet could contain multiple frames
              // each of them starts with a subheader
              // cycle through the data packet and extract the frames
              for (uint32_t start = 0; start < dataSizeByte;)
                {
                  item->m_packet->RemoveHeader (sub);
                  Ptr<Packet> nextFragment = Create<Packet> ();
                  if (sub.IsStream ())
                    {
                      NS_LOG_INFO (
                        "subheader " << sub << " dataSizeByte " << dataSizeByte << " remaining " << item->m_packet->GetSize () << " frame size " << sub.GetLength ());
                      nextFragment = item->m_packet->Copy ();
                      NS_LOG_INFO ("fragment size " << nextFragment->GetSize () << " " << sub.GetLength ());
                      nextFragment->RemoveAtEnd (
                        nextFragment->GetSize () - sub.GetLength ());
                      NS_LOG_INFO ("fragment size " << nextFragment->GetSize ());

                      // remove the first portion of the packet
                      item->m_packet->RemoveAtStart (sub.GetLength ());
                    }
                  nextFragment->AddHeader (sub);
                  start += nextFragment->GetSize ();
                  Ptr<QuicSocketTxItem> it = CreateObject<QuicSocketTxItem> (
                    *item);
                  uint64_t streamId = sub.GetStreamId ();
                  uint64_t offset = sub.GetOffset ();
                  it->m_packet = nextFragment;
                  NS_LOG_INFO (
                    "Added retx fragment on stream " << streamId << " with offset " << offset << " and length " << it->m_packet->GetSize () << ", pointer " << GetPointer (it->m_packet));
                  AddScheduleItem (CreateObject<QuicSocketTxScheduleItem> (streamId, offset, GetDeadline (it).GetSeconds (), it), false);
                }
            }
          else
            {
              NS_LOG_INFO (
                "Added retx packet on stream " << sub.GetStreamId () << " with offset " << sub.GetOffset ());
              AddScheduleItem (CreateObject<QuicSocketTxScheduleItem> (sub.GetStreamId (), sub.GetOffset (), GetDeadline (item).GetSeconds (), item), false);
            }
        }
    }
  else
    {
      QuicSubheader sub;
      item->m_packet->PeekHeader (sub);
      NS_LOG_INFO (
        "Added packet on stream " << sub.GetStreamId () << " with offset " << sub.GetOffset ());
      AddScheduleItem (CreateObject<QuicSocketTxScheduleItem> (sub.GetStreamId (), sub.GetOffset (), GetDeadline (item).GetSeconds (), item), retx);
    }
}

void QuicSocketTxEdfScheduler::SetLatency (uint32_t streamId, Time latency)
{
  m_latencyMap[streamId] = latency;
}

const Time QuicSocketTxEdfScheduler::GetLatency (uint32_t streamId)
{
  Time latency = m_defaultLatency;
  if (m_latencyMap.count (streamId) > 0)
    {
      latency = m_latencyMap.at (streamId);
    }
  else
    {
      NS_LOG_INFO (
        "Stream " << streamId << " does not have a pre-specified latency, using default");
    }
  return latency;
}

void QuicSocketTxEdfScheduler::SetDefaultLatency (Time latency)
{
  m_defaultLatency = latency;
}

const Time QuicSocketTxEdfScheduler::GetDefaultLatency ()
{
  return m_defaultLatency;
}

Time QuicSocketTxEdfScheduler::GetDeadline (Ptr<QuicSocketTxItem> item)
{
  Ptr<Packet> packet = item->m_packet;
  QuicSubheader sub;
  packet->PeekHeader (sub);
  return item->m_generated + GetLatency (sub.GetStreamId ());
}

}
