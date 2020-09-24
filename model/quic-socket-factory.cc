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

#include "quic-socket-factory.h"
#include "ns3/socket.h"
#include "ns3/assert.h"
#include "ns3/log.h"


namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("QuicSocketFactory");

NS_OBJECT_ENSURE_REGISTERED (QuicSocketFactory);

TypeId QuicSocketFactory::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketFactory")
    .SetParent<SocketFactory> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicSocketFactory> ()
  ;
  return tid;
}

QuicSocketFactory::QuicSocketFactory ()
  : m_quicl4 (0)
{
  NS_LOG_INFO ("Creating QuicSocketFactory");
  m_sockets = std::vector<Ptr<QuicSocketBase> > ();
}

QuicSocketFactory::~QuicSocketFactory ()
{
  NS_LOG_INFO ("Destroying QuicSocketFactory");
  NS_ASSERT (m_quicl4 == 0);
}

void
QuicSocketFactory::SetQuicL4 (Ptr<QuicL4Protocol> quic)
{
  m_quicl4 = quic;
}

Ptr<Socket>
QuicSocketFactory::CreateSocket (void)
{
  NS_LOG_INFO ("QuicSocketFactory -- creating socket");
  return m_quicl4->CreateSocket ();
}

void
QuicSocketFactory::DoDispose (void)
{
  m_quicl4 = 0;
  SocketFactory::DoDispose ();
}

} // namespace ns3
