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
#include "quic-stream.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicStream");

NS_OBJECT_ENSURE_REGISTERED (QuicStream);

const char* const
QuicStream::QuicStreamStateName[QuicStream::LAST_STATE] = {"IDLE", "OPEN", "SEND", "RECV", "SIZE_KNOWN", "DATA_SENT", "DATA_RECVD", "DATA_READ", "RESET_SENT", "RESET_RECVD", "RESET_READ"};

TypeId
QuicStream::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicStream")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

QuicStream::QuicStream ()
  : Object ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

QuicStream::~QuicStream ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

} // namespace ns3
