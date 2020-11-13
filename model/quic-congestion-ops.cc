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
 * Authors: Alvise De Biasio <alvise.debiasio@gmail.com>
 *          Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *          Umberto Paro <umberto.paro@me.com>
 *
 */

#define __STDC_LIMIT_MACROS

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include "quic-socket.h"
#include "quic-congestion-ops.h"
#include "quic-socket-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicCongestionControl");

NS_OBJECT_ENSURE_REGISTERED (QuicCongestionOps);

TypeId
QuicCongestionOps::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicCongestionControl")
    .SetParent<TcpNewReno> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicCongestionOps> ()
  ;
  return tid;
}

QuicCongestionOps::QuicCongestionOps (void)
  : TcpNewReno ()
{
  NS_LOG_FUNCTION (this);
}

QuicCongestionOps::QuicCongestionOps (
  const QuicCongestionOps& sock)
  : TcpNewReno (sock)
{
  NS_LOG_FUNCTION (this);
}

QuicCongestionOps::~QuicCongestionOps (void)
{}

std::string
QuicCongestionOps::GetName () const
{
  return "QuicCongestionControl";
}

Ptr<TcpCongestionOps>
QuicCongestionOps::Fork ()
{
  return CopyObject<QuicCongestionOps> (this);
}

// Quic DRAFT 10

void
QuicCongestionOps::OnPacketSent (Ptr<TcpSocketState> tcb,
                                 SequenceNumber32 packetNumber,
                                 bool isAckOnly)
{
  NS_LOG_FUNCTION (this << packetNumber << isAckOnly);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  tcbd->m_timeOfLastSentPacket = Now ();
  tcbd->m_highTxMark = packetNumber;
}

void
QuicCongestionOps::OnAckReceived (Ptr<TcpSocketState> tcb,
                                  QuicSubheader &ack,
                                  std::vector<Ptr<QuicSocketTxItem> > newAcks,
                                  const struct RateSample *rs)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (rs);

  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  tcbd->m_largestAckedPacket = SequenceNumber32 (
    ack.GetLargestAcknowledged ());

  // newAcks are ordered from the highest packet number to the smalles
  Ptr<QuicSocketTxItem> lastAcked = newAcks.at (0);

  NS_LOG_LOGIC ("Updating RTT estimate");
  // If the largest acked is newly acked, update the RTT.
  if (lastAcked->m_packetNumber == tcbd->m_largestAckedPacket)
    {
      tcbd->m_lastRtt = Now () - lastAcked->m_lastSent;
      UpdateRtt (tcbd, tcbd->m_lastRtt, Time (ack.GetAckDelay ()));
    }

  NS_LOG_LOGIC ("Processing acknowledged packets");
  // Process each acked packet
  for (auto it = newAcks.rbegin (); it != newAcks.rend (); ++it)
    {
      if ((*it)->m_acked)
        {
          OnPacketAcked (tcb, (*it));
        }
    }
}

void
QuicCongestionOps::UpdateRtt (Ptr<TcpSocketState> tcb, Time latestRtt,
                              Time ackDelay)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  // m_minRtt ignores ack delay.
  tcbd->m_minRtt = std::min (tcbd->m_minRtt, latestRtt);

  NS_LOG_LOGIC ("Correct for ACK delay");
  // Adjust for ack delay if it's plausible.
  if (latestRtt - tcbd->m_minRtt > ackDelay)
    {
      latestRtt -= ackDelay;
      // TODO check this condition
      // Only save into max ack delay if it's used for rtt calculation and is not ack only
//        if (!tcbd->m_sentPackets[tcbd->m_largestAckedPacket]->m_ackOnly)
//          {
//            tcbd->m_maxAckDelay = std::max (tcbd->m_maxAckDelay, ackDelay);
//          }
    }

  NS_LOG_LOGIC ("Update smoothed RTT");
  // Based on [RFC6298].
  if (tcbd->m_smoothedRtt == Seconds (0))
    {
      tcbd->m_smoothedRtt = latestRtt;
      tcbd->m_rttVar = latestRtt / 2;
    }
  else
    {
      Time rttVarSample = Time (
        std::abs ((tcbd->m_smoothedRtt - latestRtt).GetDouble ()));
      tcbd->m_rttVar = 3 / 4 * tcbd->m_rttVar + 1 / 4 * rttVarSample;
      tcbd->m_smoothedRtt = 7 / 8 * tcbd->m_smoothedRtt + 1 / 8 * latestRtt;
    }

}

void
QuicCongestionOps::OnPacketAcked (Ptr<TcpSocketState> tcb,
                                  Ptr<QuicSocketTxItem> ackedPacket)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  OnPacketAckedCC (tcbd, ackedPacket);

  NS_LOG_LOGIC ("Handle possible RTO");
  // If a packet sent prior to RTO was acked, then the RTO  was spurious. Otherwise, inform congestion control.
  if (tcbd->m_rtoCount > 0
      and ackedPacket->m_packetNumber > tcbd->m_largestSentBeforeRto)
    {
      OnRetransmissionTimeoutVerified (tcb);
    }
  tcbd->m_handshakeCount = 0;
  tcbd->m_tlpCount = 0;
  tcbd->m_rtoCount = 0;
}

bool
QuicCongestionOps::InRecovery (Ptr<TcpSocketState> tcb,
                               SequenceNumber32 packetNumber)
{
  NS_LOG_FUNCTION (this << packetNumber.GetValue ());
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  return packetNumber <= tcbd->m_endOfRecovery;
}

void
QuicCongestionOps::OnPacketAckedCC (Ptr<TcpSocketState> tcb,
                                    Ptr<QuicSocketTxItem> ackedPacket)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  NS_LOG_INFO ("Updating congestion window");
  if (InRecovery (tcb, ackedPacket->m_packetNumber))
    {
      NS_LOG_LOGIC ("In recovery");
      // Do not increase congestion window in recovery period.
      return;
    }
  if (tcbd->m_cWnd < tcbd->m_ssThresh)
    {
      NS_LOG_LOGIC ("In slow start");
      // Slow start.
      tcbd->m_cWnd += ackedPacket->m_packet->GetSize ();
    }
  else
    {
      NS_LOG_LOGIC ("In congestion avoidance");
      // Congestion Avoidance.
      tcbd->m_cWnd += tcbd->m_segmentSize * ackedPacket->m_packet->GetSize ()
        / tcbd->m_cWnd;
    }
}

void
QuicCongestionOps::OnPacketsLost (
  Ptr<TcpSocketState> tcb, std::vector<Ptr<QuicSocketTxItem> > lostPackets)
{
  NS_LOG_LOGIC (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  auto largestLostPacket = *(lostPackets.end () - 1);

  NS_LOG_INFO ("Go in recovery mode");
  // Start a new recovery epoch if the lost packet is larger than the end of the previous recovery epoch.
  if (!InRecovery (tcbd, largestLostPacket->m_packetNumber))
    {
      tcbd->m_endOfRecovery = tcbd->m_highTxMark;
      tcbd->m_cWnd *= tcbd->m_kLossReductionFactor;
      if (tcbd->m_cWnd < tcbd->m_kMinimumWindow)
        {
          tcbd->m_cWnd = tcbd->m_kMinimumWindow;
        }
      tcbd->m_ssThresh = tcbd->m_cWnd;
    }
}

void
QuicCongestionOps::OnRetransmissionTimeoutVerified (
  Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");
  NS_LOG_INFO ("Loss state");
  tcbd->m_cWnd = tcbd->m_kMinimumWindow;
  tcbd->m_congState = TcpSocketState::CA_LOSS;
}

} // namespace ns3
