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

#ifndef QUICSOCKETFACTORY_H
#define QUICSOCKETFACTORY_H

#include "ns3/socket-factory.h"
#include "quic-socket-base.h"
#include "ns3/node.h"
#include "quic-l4-protocol.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup quic
 *
 * \brief API to create QUIC socket instances
 *
 * This class defines the API for QUIC socket factory.
 * All QUIC socket factoty implementations must provide an implementation
 * of CreateSocket below.
 *
 */
class QuicSocketFactory : public SocketFactory
{
public:
  QuicSocketFactory ();
  virtual ~QuicSocketFactory ();

  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Implements a method to create a QUIC-based socket and return a base class smart pointer to the socket
   *
   * \return smart pointer to Socket
   */
  virtual Ptr<Socket> CreateSocket (void);

  /**
   * \brief Set the associated QUIC L4 protocol.
   *
   * \param quic the QUIC L4 protocol
   */
  void SetQuicL4 (Ptr<QuicL4Protocol> quic);

protected:
  virtual void DoDispose (void);

private:
  Ptr<QuicL4Protocol> m_quicl4;                 //!< The associated QUIC L4 protocol
  std::vector<Ptr<QuicSocketBase> > m_sockets;  //!< The list of QuicSocketBase

};

} // namespace ns3

#endif /* QUIC_SOCKET_FACTORY_H */
