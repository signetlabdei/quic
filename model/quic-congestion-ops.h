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

#ifndef QUICCONGESTIONOPS_H
#define QUICCONGESTIONOPS_H

#include "ns3/timer.h"
#include "ns3/object.h"
#include "quic-subheader.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"
#include "quic-socket-base.h"
#include "quic-socket-tx-buffer.h"

namespace ns3 {

/**
 * \ingroup quic
 * \defgroup congestionOps Congestion Control Algorithms.
 *
 * The various congestion control algorithms.
 */

/**
 * \ingroup congestionOps
 *
 * \brief Congestion control abstract class
 *
 * The congestion control is splitted from the main socket code, and it is a
 * pluggable component. An interface has been defined; variables are maintained
 * in the QuicSocketState class, while subclasses of QuicCongestionOps operate
 * over an instance of that class.
 *
 * The design extends TcpNewReno to provide compatibility with the TCP congestion
 * control implementations, as well as the possibility of extending it with new
 * QUIC-related capabilities.
 *
 */
class QuicCongestionOps : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicCongestionOps ();
  QuicCongestionOps (const QuicCongestionOps& sock);
  ~QuicCongestionOps ();

  /**
   * \brief Get the name of the congestion control algorithm
   *
   * \return A string identifying the name
   */
  std::string GetName () const;

  /**
   * \brief Copy the congestion control algorithm across socket
   *
   * \return a pointer of the copied object
   */
  Ptr<TcpCongestionOps> Fork ();

  // QuicCongestionControl Draft10

  /**
   * \brief Method called when a packet is sent. It updates the quantities in the tcb
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param packetNumber the packet number
   * \param isAckOnly a flag to signal if the packet has only an ACK frame
   */
  virtual void OnPacketSent (Ptr<TcpSocketState> tcb, SequenceNumber32 packetNumber, bool isAckOnly);

  /**
   * \brief Method called when an ack is received. It process the received ack and updates
   *   the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param ack the received ACK
   * \param newAcks the newly acked packets
   * \param rs the connection RateSample
   */
  virtual void OnAckReceived (Ptr<TcpSocketState> tcb, QuicSubheader &ack, std::vector<Ptr<QuicSocketTxItem> > newAcks,
                              const struct RateSample *rs);

  /**
   * \brief Method called when a packet is lost. It process the lost packets and updates
   *   the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param lostPackets the lost packets
   */
  virtual void OnPacketsLost (Ptr<TcpSocketState> tcb, std::vector<Ptr<QuicSocketTxItem> > lostPackets);

protected:
  // QuicCongestionControl Draft10

  /**
   * \brief Method called to update the Rtt. It updates the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param latestRtt the latest Rtt
   * \param ackDelay the ack delay
   */
  void UpdateRtt (Ptr<TcpSocketState> tcb, Time latestRtt, Time ackDelay);

  /**
   * \brief Method called when a packet is acked. It process the acked packet and updates
   *   the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param ackedPacked the acked packet
   */
  virtual void OnPacketAcked (Ptr<TcpSocketState> tcb, Ptr<QuicSocketTxItem> ackedPacket);

  /**
   * \brief Check if in recovery period
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param packetNumber to be checked
   * \return true if in recovery, false otherwhise
   */
  bool InRecovery (Ptr<TcpSocketState> tcb, SequenceNumber32 packetNumber);

  /**
   * \brief Method called when a packet is acked. It updates the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   * \param ackedPacked the acked packet
   */
  void OnPacketAckedCC (Ptr<TcpSocketState> tcb, Ptr<QuicSocketTxItem> ackedPacket);

  /**
   * \brief Method called when retransmission timeout fires. It updates the quantities in the tcb.
   *
   * \param tcb a smart pointer to the SocketState (it accepts a QuicSocketState)
   */
  virtual void OnRetransmissionTimeoutVerified (Ptr<TcpSocketState> tcb);

};

}
#endif //QUIC_NS3_QUIC_CONGESTION_OPS_H_H
