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
#include "quic-client-server-helper.h"
#include "ns3/quic-server.h"
#include "ns3/quic-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

QuicServerHelper::QuicServerHelper ()
{
  m_factory.SetTypeId (QuicServer::GetTypeId ());
}

QuicServerHelper::QuicServerHelper (uint16_t port)
{
  m_factory.SetTypeId (QuicServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
QuicServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
QuicServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<QuicServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<QuicServer>
QuicServerHelper::GetServer (void)
{
  return m_server;
}

QuicClientHelper::QuicClientHelper ()
{
  m_factory.SetTypeId (QuicClient::GetTypeId ());
}

QuicClientHelper::QuicClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (QuicClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

QuicClientHelper::QuicClientHelper (Address address)
{
  m_factory.SetTypeId (QuicClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
QuicClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
QuicClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<QuicClient> client = m_factory.Create<QuicClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

} // namespace ns3
