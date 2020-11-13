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
#include "quic-header.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicHeader");

NS_OBJECT_ENSURE_REGISTERED (QuicHeader);

QuicHeader::QuicHeader ()
  : m_form (SHORT),
  m_c (false),
  m_k (PHASE_ZERO),
  m_type (0),
  m_connectionId (0),
  m_packetNumber (0),
  m_version (0)
{
}


QuicHeader::~QuicHeader ()
{
}

std::string
QuicHeader::TypeToString () const
{
  static const char* longTypeNames[6] = {
    "Version Negotiation",
    "Initial",
    "Retry",
    "Handshake",
    "0-RTT Protected",
    "None"
  };
  static const char* shortTypeNames[4] = {
    "1 Octet",
    "2 Octets",
    "4 Octets"
  };

  std::string typeDescription = "";

  if (IsLong ())
    {
      typeDescription.append (longTypeNames[m_type]);
    }
  else
    {
      typeDescription.append (shortTypeNames[m_type]);
    }
  return typeDescription;
}

TypeId
QuicHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicHeader")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicHeader> ()
  ;
  return tid;
}

TypeId
QuicHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
QuicHeader::GetSerializedSize (void) const
{
  NS_ASSERT (m_type != NONE or m_form == SHORT);

  uint32_t serializesSize = CalculateHeaderLength ();
  NS_LOG_INFO ("Serialized Size " << serializesSize);

  return serializesSize;
}

uint32_t
QuicHeader::CalculateHeaderLength () const
{
  uint32_t len;

  if (IsLong ())
    {
      len = 8 + 64 + 32 + 32;
    }
  else
    {
      len = 8 + 64 * HasConnectionId () + GetPacketNumLen ();
    }
  return len / 8;
}

uint32_t
QuicHeader::GetPacketNumLen () const
{
  if (IsLong ())
    {
      return 32;
    }
  else
    {
      switch (m_type)
        {
        case ONE_OCTECT:
          {
            return 8;
            break;
          }
        case TWO_OCTECTS:
          {
            return 16;
            break;
          }
        case FOUR_OCTECTS:
          {
            return 32;
            break;
          }
        }
    }
  NS_FATAL_ERROR ("Invalid conditions");
  return 0;
}

void
QuicHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_type != NONE or m_form == SHORT);
  NS_LOG_INFO ("Serialize::Serialized Size " << CalculateHeaderLength ());

  Buffer::Iterator i = start;

  uint8_t t = m_type + (m_form << 7);

  if (m_form)
    {
      i.WriteU8 (t);
      i.WriteHtonU64 (m_connectionId);
      i.WriteHtonU32 (m_version);
      if (!IsVersionNegotiation ())
        {
          i.WriteHtonU32 (m_packetNumber.GetValue ());
        }
    }
  else
    {
      t += (m_c << 6) + (m_k << 5);
      i.WriteU8 (t);

      if (m_c)
        {
          i.WriteHtonU64 (m_connectionId);
        }

      switch (m_type)
        {
        case ONE_OCTECT:
          i.WriteU8 ((uint8_t)m_packetNumber.GetValue ());
          break;
        case TWO_OCTECTS:
          i.WriteHtonU16 ((uint16_t)m_packetNumber.GetValue ());
          break;
        case FOUR_OCTECTS:
          i.WriteHtonU32 ((uint32_t)m_packetNumber.GetValue ());
          break;
        }
    }
}

uint32_t
QuicHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);

  Buffer::Iterator i = start;

  uint8_t t = i.ReadU8 ();

  m_form = (t & 0x80) >> 7;

  if (IsShort ())
    {
      m_c = (t & 0x40) >> 6;
      m_k = (t & 0x20) >> 5;
      SetTypeByte (t & 0x1F);
    }
  else
    {
      SetTypeByte (t & 0x7F);
    }
  NS_ASSERT (m_type != NONE or m_form == SHORT);

  if (HasConnectionId ())
    {
      SetConnectionID (i.ReadNtohU64 ());
    }

  if (IsLong ())
    {
      SetVersion (i.ReadNtohU32 ());
      if (!IsVersionNegotiation ())
        {
          SetPacketNumber (SequenceNumber32 (i.ReadNtohU32 ()));
        }
    }
  else
    {
      switch (m_type)
        {
        case ONE_OCTECT:
          SetPacketNumber (SequenceNumber32 (i.ReadU8 ()));
          break;
        case TWO_OCTECTS:
          SetPacketNumber (SequenceNumber32 (i.ReadNtohU16 ()));
          break;
        case FOUR_OCTECTS:
          SetPacketNumber (SequenceNumber32 (i.ReadNtohU32 ()));
          break;
        }
    }

  NS_LOG_INFO ("Deserialize::Serialized Size " << CalculateHeaderLength ());

  return GetSerializedSize ();
}

void
QuicHeader::Print (std::ostream &os) const
{
  NS_ASSERT (m_type != NONE or m_form == SHORT);

  os << "|" << m_form << "|";

  if (IsShort ())
    {
      os << m_c << "|" << m_k << "|" << "1|0|";
    }

  os << TypeToString () << "|\n|";

  if (HasConnectionId ())
    {
      os << "ConnectionID " << m_connectionId << "|\n|";
    }
  if (IsShort ())
    {
      os << "PacketNumber " << m_packetNumber << "|\n";
    }
  else
    {
      os << "Version " << (uint64_t)m_version << "|\n";
      os << "PacketNumber " << m_packetNumber << "|\n|";
    }

}

QuicHeader
QuicHeader::CreateInitial (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber)
{
  NS_LOG_INFO ("Create Initial Helper called");

  QuicHeader head;
  head.SetFormat (QuicHeader::LONG);
  head.SetTypeByte (QuicHeader::INITIAL);
  head.SetConnectionID (connectionId);
  head.SetVersion (version);
  head.SetPacketNumber (packetNumber);

  return head;
}


QuicHeader
QuicHeader::CreateRetry (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber)
{
  NS_LOG_INFO ("Create Retry Helper called");

  QuicHeader head;
  head.SetFormat (QuicHeader::LONG);
  head.SetTypeByte (QuicHeader::RETRY);
  head.SetConnectionID (connectionId);
  head.SetVersion (version);
  head.SetPacketNumber (packetNumber);

  return head;
}

QuicHeader
QuicHeader::CreateHandshake (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber)
{
  NS_LOG_INFO ("Create Handshake Helper called ");

  QuicHeader head;
  head.SetFormat (QuicHeader::LONG);
  head.SetTypeByte (QuicHeader::HANDSHAKE);
  head.SetConnectionID (connectionId);
  head.SetVersion (version);
  head.SetPacketNumber (packetNumber);

  return head;
}

QuicHeader
QuicHeader::Create0RTT (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber)
{
  NS_LOG_INFO ("Create 0RTT Helper called");

  QuicHeader head;
  head.SetFormat (QuicHeader::LONG);
  head.SetTypeByte (QuicHeader::ZRTT_PROTECTED);
  head.SetConnectionID (connectionId);
  head.SetVersion (version);
  head.SetPacketNumber (packetNumber);

  return head;
}

QuicHeader
QuicHeader::CreateShort (uint64_t connectionId, SequenceNumber32 packetNumber, bool connectionIdFlag, bool keyPhaseBit)
{
  NS_LOG_INFO ("Create Short Helper called");

  QuicHeader head;
  head.SetFormat (QuicHeader::SHORT);
  head.SetKeyPhaseBit (keyPhaseBit);
  head.SetPacketNumber (packetNumber);

  if (connectionIdFlag)
    {
      head.SetConnectionID (connectionId);
    }

  return head;
}

QuicHeader
QuicHeader::CreateVersionNegotiation (uint64_t connectionId, uint32_t version, std::vector<uint32_t>& supportedVersions)
{
  NS_LOG_INFO ("Create Version Negotiation Helper called");

  QuicHeader head;
  head.SetFormat (QuicHeader::LONG);
  head.SetTypeByte (QuicHeader::VERSION_NEGOTIATION);
  head.SetConnectionID (connectionId);
  head.SetVersion (version);

//	TODO: SetVersions(m)
//	head.SetVersions(m_supportedVersions);
//
//   uint8_t *buffer = new uint8_t[4 * m_supportedVersions.size()];
//
//    for (uint8_t i = 0; i < (uint8_t) m_supportedVersions.size(); i++) {
//
//	    buffer[4*i] = (m_supportedVersions[i]) ;
//	    buffer[4*i+1] = (m_supportedVersions[i] >> 8);
//	    buffer[4*i+2] = (m_supportedVersions[i] >> 16);
//	    buffer[4*i+3] = (m_supportedVersions[i] >> 24);
//
//    }
//
//    Ptr<Packet> payload = Create<Packet> (buffer, 4 * m_supportedVersions.size());

  return head;
}

uint8_t
QuicHeader::GetTypeByte () const
{
  return m_type;
}

void
QuicHeader::SetTypeByte (uint8_t typeByte)
{
  m_type = typeByte;
}

uint8_t
QuicHeader::GetFormat () const
{
  return m_form;
}

void
QuicHeader::SetFormat (bool form)
{
  m_form = form;
}

uint64_t
QuicHeader::GetConnectionId () const
{
  NS_ASSERT (HasConnectionId ());
  return m_connectionId;
}

void
QuicHeader::SetConnectionID (uint64_t connID)
{
  m_connectionId = connID;
  if (IsShort ())
    {
      m_c = true;
    }
}

SequenceNumber32
QuicHeader::GetPacketNumber () const
{
  return m_packetNumber;
}

void
QuicHeader::SetPacketNumber (SequenceNumber32 packNum)
{
  NS_LOG_INFO (packNum);
  m_packetNumber = packNum;
  if (IsShort ())
    {
      if (packNum.GetValue () < 256)
        {
          SetTypeByte (ONE_OCTECT);
        }
      else if (packNum.GetValue () < 65536)
        {
          SetTypeByte (TWO_OCTECTS);
        }
      else
        {
          SetTypeByte (FOUR_OCTECTS);
        }
    }
}

uint32_t
QuicHeader::GetVersion () const
{
  NS_ASSERT (HasVersion ());
  return m_version;
}

void
QuicHeader::SetVersion (uint32_t version)
{
  NS_ASSERT (HasVersion ());
  m_version = version;
}

bool
QuicHeader::GetKeyPhaseBit () const
{
  NS_ASSERT (IsShort ());
  return m_k;
}

void
QuicHeader::SetKeyPhaseBit (bool keyPhaseBit)
{
  NS_ASSERT (IsShort ());
  m_k = keyPhaseBit;
}

bool QuicHeader::IsShort () const
{
  return m_form == SHORT;
}

bool
QuicHeader::IsVersionNegotiation () const
{
  return m_type == VERSION_NEGOTIATION;
}

bool
QuicHeader::IsInitial () const
{
  return m_type == INITIAL;
}

bool
QuicHeader::IsRetry () const
{
  return m_type == RETRY;
}

bool
QuicHeader::IsHandshake () const
{
  return m_type == HANDSHAKE;
}

bool
QuicHeader::IsORTT () const
{
  return m_type == ZRTT_PROTECTED;
}

bool QuicHeader::HasVersion () const
{
  return IsLong ();
}

bool QuicHeader::HasConnectionId () const
{
  return not (IsShort () and m_c == false);
}

bool
operator== (const QuicHeader &lhs, const QuicHeader &rhs)
{
  return (
    lhs.m_form == rhs.m_form
    && lhs.m_c == rhs.m_c
    && lhs.m_k  == rhs.m_k
    && lhs.m_type == rhs.m_type
    && lhs.m_connectionId == rhs.m_connectionId
    && lhs.m_packetNumber == rhs.m_packetNumber
    && lhs.m_version == rhs.m_version
    );
}

std::ostream&
operator<< (std::ostream& os, QuicHeader& tc)
{
  tc.Print (os);
  return os;
}

} // namespace ns3

