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

#ifndef QUICSOCKETTXPFIFOSCHEDULER_H
#define QUICSOCKETTXPFIFOSCHEDULER_H

#include "quic-socket-tx-scheduler.h"
#include <queue>
#include <vector>

namespace ns3 {

/**
 * \brief The PFIFO implementation
 *
 * This class is a Priority FIFO implementation of the socket scheduler, which prioritizes streams with a lower stream number
 */
class QuicSocketTxPFifoScheduler : public QuicSocketTxScheduler
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicSocketTxPFifoScheduler ();
  QuicSocketTxPFifoScheduler (const QuicSocketTxPFifoScheduler &other);
  virtual ~QuicSocketTxPFifoScheduler (void);

  /**
   * Add a tx item to the scheduling list and assign priority
   *
   * \param item a smart pointer to a transmission item
   * \param retx true if the transmission item is being retransmitted
   *
   */
  void Add (Ptr<QuicSocketTxItem> item, bool retx) override;

private:
  bool m_retxFirst;
};

} // namepsace ns3

#endif /* QUIC_SOCKET_TX_PFIFO_SCHEDULER_H */
