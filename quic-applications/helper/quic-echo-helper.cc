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

#include "quic-echo-helper.h"
#include "ns3/quic-echo-server.h"
#include "ns3/quic-echo-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("QuicEchoHelper");

namespace ns3 {

QuicEchoServerHelper::QuicEchoServerHelper (uint16_t port)
{
  m_factory.SetTypeId (QuicEchoServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
QuicEchoServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
QuicEchoServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
QuicEchoServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
QuicEchoServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
QuicEchoServerHelper::InstallPriv (Ptr<Node> node) const
{

  Ptr<Application> app = m_factory.Create<QuicEchoServer> ();
  node->AddApplication (app);
  NS_LOG_INFO ("Installing app " << app << " in node " << node);
  return app;
}

QuicEchoClientHelper::QuicEchoClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (QuicEchoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

QuicEchoClientHelper::QuicEchoClientHelper (Address address)
{
  m_factory.SetTypeId (QuicEchoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
QuicEchoClientHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
QuicEchoClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<QuicEchoClient>()->SetFill (fill);
}

void
QuicEchoClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<QuicEchoClient>()->SetFill (fill, dataLength);
}

void
QuicEchoClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<QuicEchoClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
QuicEchoClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
QuicEchoClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
QuicEchoClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
QuicEchoClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<QuicEchoClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
