/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Authors: Davide Marcato <davide.marcato.4@studenti.unipd.it>
 *          Stefano Ravazzolo <stefano.ravazzolo@studenti.unipd.it>
 *          Alvise De Biasio <alvise.debiasio@studenti.unipd.it>
 */

#ifndef QUIC_ECHO_SERVER_H
#define QUIC_ECHO_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup quicecho QuicEcho
 */

/**
 * \ingroup quicecho
 * \brief A Quic Echo server
 *
 * Every packet received is sent back.
 */
class QuicEchoServer : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  QuicEchoServer ();
  virtual ~QuicEchoServer ();

  Ptr<Socket> GetSocket ();

  /**
   * Set the ID of the stream to be used in the underlying QUIC socket
   *
   * \param streamId the ID of the stream (>0)
   */
  void SetStreamId (uint32_t streamId);

  /**
   * Get the stream ID to be used in the underlying QUIC socket
   *
   * \return the stream ID
   */
  uint32_t GetStreamId (void) const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address

  uint32_t m_streamId;
};

} // namespace ns3

#endif /* QUIC_ECHO_SERVER_H */

