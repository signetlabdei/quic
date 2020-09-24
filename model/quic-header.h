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

#ifndef QUICHEADER_H
#define QUICHEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

namespace ns3 {

/**
 * \ingroup quic
 * \brief Header for the QUIC Protocol
 *
 * This class has fields corresponding to those in a QUIC header
 * (connection id, packet number, version, flags, etc) as well
 * as methods for serialization to and deserialization from a buffer.
 */
class QuicHeader : public Header
{
public:
  /**
   * \brief Quic header form bit values
   */
  typedef enum
  {
    SHORT = 0,  //!< Short header
    LONG  = 1   //!< Long header
  } TypeFormat_t;

  /**
   * \brief Quic long header type byte values
   */
  typedef enum
  {
    VERSION_NEGOTIATION = 0,  //!< Version Negotiation
    INITIAL  = 1,             //!< Initial
    RETRY  = 2,               //!< Retry
    HANDSHAKE  = 3,           //!< Handshake
    ZRTT_PROTECTED  = 4,      //!< 0-Rtt Protected
    NONE = 5                  //!< No type byte
  } TypeLong_t;

  /**
   * \brief Quic header key phase bit values
   */
  typedef enum
  {
    PHASE_ZERO = 0,  //!< Phase 0
    PHASE_ONE  = 1   //!< Phase 1
  } KeyPhase_t;

  /**
   * \brief Quic packet number encodings for headers
   */
  typedef enum
  {
    ONE_OCTECT = 0x0,    //!< 1 Octet
    TWO_OCTECTS  = 0x1,  //!< 2 Octets
    FOUR_OCTECTS  = 0x2  //!< 4 Octects
  } TypeShort_t;

  QuicHeader ();
  virtual ~QuicHeader ();

  // Inherited from Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Print a Quic header into an output stream
   *
   * \param os output stream
   * \param tc Quic header to print
   * \return The ostream passed as first argument
   */
  friend std::ostream& operator<< (std::ostream& os, QuicHeader & tc);

  /**
   * \brief Converts the short and long type bytes into a human readable string description
   *
   * \return the generated string
   **/
  std::string TypeToString () const;

  /**
   * Create the header for the Initial client->server packet
   *
   * \param connectionId the ID of the connection
   * \param version the version of the connection
   * \param packetNumber the packet number
   * \return the generated QuicHeader
   */
  static QuicHeader CreateInitial (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber);

  /**
   * Create a Retry header
   *
   * \param connectionId the ID of the connection
   * \param version the version of the connection
   * \param packetNumber the packet number
   * \return the generated QuicHeader
   */
  static QuicHeader CreateRetry (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber);

  /**
   * Create the header for the Handshake server->client packet
   *
   * \param connectionId the ID of the connection
   * \param version the version of the connection
   * \param packetNumber the packet number
   * \return the generated QuicHeader
   */
  static QuicHeader CreateHandshake (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber);

  /**
   * Create the header for a 0-Rtt Protected packet
   *
   * \param connectionId the ID of the connection
   * \param version the version of the connection
   * \param packetNumber the packet number
   * \return the generated QuicHeader
   */
  static QuicHeader Create0RTT (uint64_t connectionId, uint32_t version, SequenceNumber32 packetNumber);

  /**
   * Create the header for a Version Negotiation packet, sends to the client a list of supported versions
   *
   * \param connectionId the ID of the connection
   * \param version the version of the connection
   * \param supportedVersions a vector of supported versions
   * \return the generated QuicHeader
   */
  static QuicHeader CreateVersionNegotiation (uint64_t connectionId, uint32_t version, std::vector<uint32_t>& supportedVersions);

  /**
   * Create a Short header
   *
   * \param connectionId the ID of the connection
   * \param packetNumber the packet number
   * \param connectionIdFlag a flag, if true the packet will carry the connection ID
   * \param keyPhaseBit the key phase, which allows a recipient of a packet to identify the packet protection keys that are used to protect the packet.
   * \return the generated QuicHeader
   */
  static QuicHeader CreateShort (uint64_t connectionId, SequenceNumber32 packetNumber, bool connectionIdFlag = true, bool keyPhaseBit = QuicHeader::PHASE_ZERO);

  // Getters, Setters and Controls

  /**
   * \brief Get the type byte
   * \return The type byte for this QuicHeader
   */
  uint8_t GetTypeByte () const;

  /**
   * \brief Set the type byte
   * \param typeByte the type byte for this QuicHeader
   */
  void SetTypeByte (uint8_t typeByte);

  /**
   * \brief Get the connection id
   * \return The connection id for this QuicHeader
   */
  uint64_t GetConnectionId () const;

  /**
   * \brief Set the connection id
   * \param connID the connection id for this QuicHeader
   */
  void SetConnectionID (uint64_t connID);

  /**
   * \brief Get the packet number
   * \return The packet number for this QuicHeader
   */
  SequenceNumber32 GetPacketNumber () const;

  /**
   * \brief Calculates the packet number length (in bits)
   *
   * Given the standard size of the header, the method checks for options
   * and calculates the real packet number length (in bits).
   *
   * \return packet number length in bits
   */
  uint32_t GetPacketNumLen ()  const;

  /**
   * \brief Set the packet number
   * \param packNumber the packet number for this QuicHeader
   */
  void SetPacketNumber (SequenceNumber32 packNumber);

  /**
   * \brief Get the version
   * \return The version for this QuicHeader
   */
  uint32_t GetVersion () const;

  /**
   * \brief Set the version
   * \param version the version for this QuicHeader
   */
  void SetVersion (uint32_t version);

  /**
   * \brief Get the key phase bit
   * \return The key phase bit for this QuicHeader
   */
  bool GetKeyPhaseBit () const;

  /**
   * \brief Set the key phase bit
   * \param keyPhaseBit the key phase bit for this QuicHeader
   */
  void SetKeyPhaseBit (bool keyPhaseBit);

  /**
   * \brief Get the form bit
   * \return The form bit for this QuicHeader
   */
  uint8_t GetFormat () const;

  /**
   * \brief Set the form bit
   * \param form the form bit for this QuicHeader
   */
  void SetFormat (bool form);

  /**
   * \brief Check if the header is Short
   * \return true if the header is Short, false otherwise
   */
  bool IsShort () const;

  /**
   * \brief Check if the header is Long
   * \return true if the header is Long, false otherwise
   */
  bool IsLong ()  const
  {
    return !IsShort ();
  }

  /**
   * \brief Check if the header is Version Negotiation
   * \return true if the header is Version Negotiation, false otherwise
   */
  bool IsVersionNegotiation () const;

  /**
   * \brief Check if the header is Initial
   * \return true if the header is Initial, false otherwise
   */
  bool IsInitial () const;

  /**
   * \brief Check if the header is Retry
   * \return true if the header is Retry, false otherwise
   */
  bool IsRetry () const;

  /**
   * \brief Check if the header is Handshake
   * \return true if the header is Handshake, false otherwise
   */
  bool IsHandshake () const;

  /**
   * \brief Check if the header is 0-Rtt Protected
   * \return true if the header is 0-Rtt Protected, false otherwise
   */
  bool IsORTT () const;

  /**
   * \brief Check if the header has the connection id
   * \return true if the header has the connection id, false otherwise
   */
  bool HasConnectionId ()  const;

  /**
   * \brief Check if the header has the version
   * \return true if the header has the version, false otherwise
   */
  bool HasVersion () const;

  /**
   * Comparison operator
   * \param lhs left operand
   * \param rhs right operand
   * \return true if the operands are equal
   */
  friend bool operator== (const QuicHeader &lhs, const QuicHeader &rhs);

private:
  /**
   * \brief Calculates the header length (in words)
   *
   * Given the standard size of the header, the method checks for options
   * and calculates the real length (in words).
   *
   * \return header length in 4-byte words
   */
  uint32_t CalculateHeaderLength () const;

  bool m_form;                      //!< Form bit
  bool m_c;                         //!< Connection id flag
  bool m_k;                         //!< Key phase bit
  uint8_t m_type;                   //!< Type byte
  uint64_t m_connectionId;          //!< Connection Id
  SequenceNumber32 m_packetNumber;  //!< Packet number
  uint32_t m_version;               //!< Version
};

} // namespace ns3

#endif /* QUIC_HEADER_H_ */
