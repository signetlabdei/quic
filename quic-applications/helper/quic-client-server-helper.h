/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */
#ifndef QUIC_CLIENT_SERVER_HELPER_H
#define QUIC_CLIENT_SERVER_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/quic-server.h"
#include "ns3/quic-client.h"
namespace ns3 {
/**
 * \ingroup quicclientserver
 * \brief Create a server application which waits for input QUIC packets
 *        and uses the information carried into their payload to compute
 *        delay and to determine if some packets are lost.
 */
class QuicServerHelper
{
public:
  /**
   * Create QuicServerHelper which will make life easier for people trying
   * to set up simulations with quic-client-server application.
   *
   */
  QuicServerHelper ();

  /**
   * Create QuicServerHelper which will make life easier for people trying
   * to set up simulations with quic-client-server application.
   *
   * \param port The port the server will wait on for incoming packets
   */
  QuicServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create one QUIC server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);

  /**
   * \brief Return the last created server.
   *
   * This function is mainly used for testing.
   *
   * \returns a Ptr to the last created server application
   */
  Ptr<QuicServer> GetServer (void);

private:
  ObjectFactory m_factory; //!< Object factory.
  Ptr<QuicServer> m_server; //!< The last created server application
};

/**
 * \ingroup quicclientserver
 * \brief Create a client application which sends QUIC packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class QuicClientHelper
{

public:
  /**
   * Create QuicClientHelper which will make life easier for people trying
   * to set up simulations with quic-client-server.
   *
   */
  QuicClientHelper ();

  /**
   *  Create QuicClientHelper which will make life easier for people trying
   * to set up simulations with quic-client-server. Use this variant with
   * addresses that do not include a port value (e.g., Ipv4Address and
   * Ipv6Address).
   *
   * \param ip The IP address of the remote QUIC server
   * \param port The port number of the remote QUIC server
   */

  QuicClientHelper (Address ip, uint16_t port);
  /**
   *  Create QuicClientHelper which will make life easier for people trying
   * to set up simulations with quic-client-server. Use this variant with
   * addresses that do include a port value (e.g., InetSocketAddress and
   * Inet6SocketAddress).
   *
   * \param addr The address of the remote QUIC server
   */

  QuicClientHelper (Address addr);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
     * \param c the nodes
     *
     * Create one QUIC client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* QUIC_CLIENT_SERVER_H */
