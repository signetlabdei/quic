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

#include <stdint.h>
#include <iostream>
#include "quic-transport-parameters.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicTransportParameters");

NS_OBJECT_ENSURE_REGISTERED (QuicTransportParameters);

QuicTransportParameters::QuicTransportParameters ()
  : m_initial_max_stream_data (0),
  m_initial_max_data (0),
  m_initial_max_stream_id_bidi (0),
  m_idleTimeout (300),
  m_omit_connection (false),
  m_max_packet_size (65527),
  //m_stateless_reset_token(0),
  m_ack_delay_exponent (3),
  m_initial_max_stream_id_uni (0)
{
}


QuicTransportParameters::~QuicTransportParameters ()
{
}

TypeId
QuicTransportParameters::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicTransportParameters")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicTransportParameters> ()
  ;
  return tid;
}

TypeId
QuicTransportParameters::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
QuicTransportParameters::GetSerializedSize (void) const
{
  uint32_t serializesSize = CalculateHeaderLength ();
  NS_LOG_INFO ("Serialized Size " << serializesSize);

  return serializesSize;
}

uint32_t
QuicTransportParameters::CalculateHeaderLength () const
{
  uint32_t len = 32 * 4 + 16 * 2 + 8 * 2;

  return len / 8;
}


void
QuicTransportParameters::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Serialize::Serialized Size " << CalculateHeaderLength ());

  Buffer::Iterator i = start;

  i.WriteHtonU32 (m_initial_max_stream_data);
  i.WriteHtonU32 (m_initial_max_data);
  i.WriteHtonU32 (m_initial_max_stream_id_bidi);
  i.WriteHtonU16 (m_idleTimeout);
  i.WriteU8 (m_omit_connection);
  i.WriteHtonU16 (m_max_packet_size);
  //i.WriteHtonU128(m_stateless_reset_token);
  i.WriteU8 (m_ack_delay_exponent);
  i.WriteHtonU32 (m_initial_max_stream_id_uni);

}

uint32_t
QuicTransportParameters::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);

  Buffer::Iterator i = start;

  m_initial_max_stream_data = i.ReadNtohU32 ();
  m_initial_max_data = i.ReadNtohU32 ();
  m_initial_max_stream_id_bidi = i.ReadNtohU32 ();
  m_idleTimeout = i.ReadNtohU16 ();
  m_omit_connection = i.ReadU8 ();
  m_max_packet_size = i.ReadNtohU16 ();
  //m_stateless_reset_token = i.ReadNtohU128();
  m_ack_delay_exponent = i.ReadU8 ();
  m_initial_max_stream_id_uni = i.ReadNtohU32 ();

  NS_LOG_INFO ("Deserialize::Serialized Size " << CalculateHeaderLength ());

  return GetSerializedSize ();
}

void
QuicTransportParameters::Print (std::ostream &os) const
{
  os << "[initial_max_stream_data " << m_initial_max_stream_data << "|\n";
  os << "|initial_max_data " << m_initial_max_data << "|\n";
  os << "|initial_max_stream_id_bidi " << m_initial_max_stream_id_bidi << "|\n";
  os << "|idleTimeout " << m_idleTimeout << "|\n";
  os << "|omit_connection " << (uint16_t)m_omit_connection << "|\n";
  os << "|max_packet_size " << m_max_packet_size << "|\n";
  //os << "|stateless_reset_token " << m_stateless_reset_token << "|\n";
  os << "|ack_delay_exponent " << (uint16_t)m_ack_delay_exponent << "|\n";
  os << "|initial_max_stream_id_uni " << m_initial_max_stream_id_uni << "]\n";
}

QuicTransportParameters
QuicTransportParameters::CreateTransportParameters (uint32_t initial_max_stream_data, uint32_t initial_max_data, uint32_t initial_max_stream_id_bidi, uint16_t idleTimeout,
                                                    uint8_t omit_connection, uint16_t max_packet_size, /*uint128_t stateless_reset_token,*/ uint8_t ack_delay_exponent, uint32_t initial_max_stream_id_uni)
{
  NS_LOG_INFO ("Create Transport Parameters Helper called");

  QuicTransportParameters transport;
  transport.SetInitialMaxStreamData (initial_max_stream_data);
  transport.SetInitialMaxData (initial_max_data);
  transport.SetInitialMaxStreamIdBidi (initial_max_stream_id_bidi);
  transport.SetIdleTimeout (idleTimeout);
  transport.SetOmitConnection (omit_connection);
  transport.SetMaxPacketSize (max_packet_size);
  //transport.SetStatelessResetToken(stateless_reset_token);
  transport.SetAckDelayExponent (ack_delay_exponent);
  transport.SetInitialMaxStreamIdUni (initial_max_stream_id_uni);

  return transport;
}


bool
operator== (const QuicTransportParameters &lhs, const QuicTransportParameters &rhs)
{
  return (
    lhs.m_initial_max_stream_data == rhs.m_initial_max_stream_data
    && lhs.m_initial_max_data == rhs.m_initial_max_data
    && lhs.m_initial_max_stream_id_bidi  == rhs.m_initial_max_stream_id_bidi
    && lhs.m_idleTimeout == rhs.m_idleTimeout
    && lhs.m_omit_connection == rhs.m_omit_connection
    && lhs.m_max_packet_size == rhs.m_max_packet_size
    //&& lhs.m_stateless_reset_token == rhs.m_stateless_reset_token
    && lhs.m_ack_delay_exponent == rhs.m_ack_delay_exponent
    && lhs.m_initial_max_stream_id_uni == rhs.m_initial_max_stream_id_uni
    );
}

std::ostream&
operator<< (std::ostream& os, QuicTransportParameters& tc)
{
  tc.Print (os);
  return os;
}

uint8_t QuicTransportParameters::GetAckDelayExponent () const
{
  return m_ack_delay_exponent;
}

void QuicTransportParameters::SetAckDelayExponent (uint8_t ackDelayExponent)
{
  m_ack_delay_exponent = ackDelayExponent;
}

uint16_t QuicTransportParameters::GetIdleTimeout () const
{
  return m_idleTimeout;
}

void QuicTransportParameters::SetIdleTimeout (uint16_t idleTimeout)
{
  m_idleTimeout = idleTimeout;
}

uint32_t QuicTransportParameters::GetInitialMaxData () const
{
  return m_initial_max_data;
}

void QuicTransportParameters::SetInitialMaxData (uint32_t initialMaxData)
{
  m_initial_max_data = initialMaxData;
}

uint32_t QuicTransportParameters::GetInitialMaxStreamData () const
{
  return m_initial_max_stream_data;
}

void QuicTransportParameters::SetInitialMaxStreamData (uint32_t initialMaxStreamData)
{
  m_initial_max_stream_data = initialMaxStreamData;
}

uint32_t QuicTransportParameters::GetInitialMaxStreamIdBidi () const
{
  return m_initial_max_stream_id_bidi;
}

void QuicTransportParameters::SetInitialMaxStreamIdBidi (uint32_t initialMaxStreamIdBidi)
{
  m_initial_max_stream_id_bidi = initialMaxStreamIdBidi;
}

uint32_t QuicTransportParameters::GetInitialMaxStreamIdUni () const
{
  return m_initial_max_stream_id_uni;
}

void QuicTransportParameters::SetInitialMaxStreamIdUni (uint32_t initialMaxStreamIdUni)
{
  m_initial_max_stream_id_uni = initialMaxStreamIdUni;
}

uint16_t QuicTransportParameters::GetMaxPacketSize () const
{
  return m_max_packet_size;
}

void QuicTransportParameters::SetMaxPacketSize (uint16_t maxPacketSize)
{
  m_max_packet_size = maxPacketSize;
}

uint8_t QuicTransportParameters::GetOmitConnection () const
{
  return m_omit_connection;
}

void QuicTransportParameters::SetOmitConnection (uint8_t omitConnection)
{
  m_omit_connection = omitConnection;
}

} // namespace ns3

