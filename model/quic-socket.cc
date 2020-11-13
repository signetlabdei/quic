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

#define __STDC_LIMIT_MACROS

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include "quic-socket.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicSocket");

NS_OBJECT_ENSURE_REGISTERED (QuicSocket);

const char* const
QuicSocket::QuicStateName[QuicSocket::LAST_STATE] = {
  "IDLE", "LISTENING", "CONNECTING_SVR",
  "CONNECTING_CLT", "OPEN", "CLOSING"
};

TypeId
QuicSocket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocket")
    .SetParent<Socket> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

QuicSocket::QuicSocket ()
  : Socket (),
  m_socketType (NONE)
{
  NS_LOG_FUNCTION_NOARGS ();
}

QuicSocket::QuicSocket (const QuicSocket& sock)
  : Socket (sock),
  m_socketType (sock.m_socketType)
{
  NS_LOG_FUNCTION_NOARGS ();
}

QuicSocket::~QuicSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

QuicSocket::QuicSocketTypes_t
QuicSocket::GetQuicSocketType () const
{
  return m_socketType;
}

void
QuicSocket::SetQuicSocketType (QuicSocketTypes_t m_socketType)
{
  QuicSocket::m_socketType = m_socketType;
}

bool
QuicSocket::CheckVersionNegotiation (uint32_t version)
{
  if (version == QUIC_VERSION_NEGOTIATION)
    {
      return true;
    }
  else if ((version & QUIC_VERSION_NEGOTIATION_PATTERN) == QUIC_VERSION_NEGOTIATION_PATTERN)
    {
      return true;
    }
  else
    {
      return false;
    }
}

} // namespace ns3
