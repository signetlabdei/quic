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

#include "quic-socket-tx-pfifo-scheduler.h"

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

NS_LOG_COMPONENT_DEFINE ("QuicSocketTxPFifoScheduler");

NS_OBJECT_ENSURE_REGISTERED (QuicSocketTxPFifoScheduler);

TypeId QuicSocketTxPFifoScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketTxPFifoScheduler")
    .SetParent<QuicSocketTxScheduler> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicSocketTxPFifoScheduler> ()
    .AddAttribute ("RetxFirst", "Prioritize retransmissions regardless of stream",
                   BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketTxPFifoScheduler::m_retxFirst),
                   MakeBooleanChecker ())
  ;
  return tid;
}

QuicSocketTxPFifoScheduler::QuicSocketTxPFifoScheduler () :
  QuicSocketTxScheduler (), m_retxFirst (false)
{}

QuicSocketTxPFifoScheduler::QuicSocketTxPFifoScheduler (
  const QuicSocketTxPFifoScheduler &other) :
  QuicSocketTxScheduler (other), m_retxFirst (
    other.m_retxFirst)
{}

QuicSocketTxPFifoScheduler::~QuicSocketTxPFifoScheduler (void)
{}

void
QuicSocketTxPFifoScheduler::Add (Ptr<QuicSocketTxItem> item, bool retx)
{
  NS_LOG_FUNCTION (this << item);
  QuicSubheader qsb;
  item->m_packet->PeekHeader (qsb);
  NS_LOG_INFO ("Adding packet on stream " << qsb.GetStreamId ());
  if (!retx)
    {
      NS_LOG_INFO ("Standard item, add at end (offset " << qsb.GetOffset () << ")");
    }
  else
    {
      NS_LOG_INFO ("Retransmitted item, add at beginning (offset " << qsb.GetOffset () << ")");
    }
  AddScheduleItem (CreateObject<QuicSocketTxScheduleItem> (qsb.GetStreamId (), qsb.GetOffset (), 0, item), (retx && m_retxFirst));
}


}
