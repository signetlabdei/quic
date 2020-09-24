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

#ifndef QUICTRANSPORTPARAMETERS_H
#define QUICTRANSPORTPARAMETERS_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

namespace ns3 {

/**
 * \ingroup quic
 * \brief Transport Parameters for the QUIC Protocol
 *
 * This class has fields corresponding to those in QUIC Transport Parameters
 * as well as methods for serialization to and deserialization from a buffer.
 *
 * Transport Parameters [Quic IETF Draft 13 Transport - sec. 6.6]
 * --------------------------------------------------------------
 *
 * During connection establishment, both endpoints make authenticated
 * declarations of their transport parameters. These declarations are
 * made unilaterally by each endpoint. Endpoints are required to comply
 * of with the restrictions implied by these parameters; the description
 * each parameter includes rules for its handling. QUIC encodes
 * transport parameters into a sequence of octets, which are then
 * included in the cryptographic handshake. Once the handshake
 * completes, the transport parameters declared by the peer are
 * available. Each endpoint validates the value provided by its peer.
 *
 */
class QuicTransportParameters : public Header
{
public:
  QuicTransportParameters ();
  virtual ~QuicTransportParameters ();

  // Inherited from Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Print Quic Transport Parameters into an output stream
   *
   * \param os output stream
   * \param tc Quic Transport Parameters to print
   * \return The ostream passed as first argument
   */
  friend std ::ostream& operator<< (std::ostream& os, QuicTransportParameters & tc);

  /**
   * Create the Transport Parameters block
   *
   * \param initial_max_stream_data the initial value for the maximum data that can be sent on any newly created stream
   * \param initial_max_data the initial value for the maximum amount of data that can be sent on the connection
   * \param initial_max_stream_id_bidi the the initial maximum number of application-owned bidirectional streams the peer may initiate
   * \param idleTimeout the idle timeout value in seconds
   * \param omit_connection the flag that indicates if the connection id is required in the upcoming connection
   * \param max_packet_size the limit on the size of packets that the endpoint is willing to receive
   * \param stateless_reset_token the stateless reset token
   * \param ack_delay_exponent the exponent used to decode the ack delay field in the ACK frame
   * \param initial_max_stream_id_uni the initial maximum number of application-owned unidirectional streams the peer may initiate
   * \return the generated QuicTransportParameters
   */
  static QuicTransportParameters CreateTransportParameters (uint32_t initial_max_stream_data, uint32_t initial_max_data, uint32_t initial_max_stream_id_bidi, uint16_t idleTimeout,
                                                            uint8_t omit_connection, uint16_t max_packet_size, /*uint128_t stateless_reset_token,*/ uint8_t ack_delay_exponent, uint32_t initial_max_stream_id_uni);

  // Getters, Setters and Controls

  /**
   * \brief Get the ack delay exponent
   * \return The ack delay exponent for this QuicTransportParameters
   */
  uint8_t GetAckDelayExponent () const;

  /**
   * \brief Set the ack delay exponent
   * \param ackDelayExponent the ack delay exponent for this QuicTransportParameters
   */
  void SetAckDelayExponent (uint8_t ackDelayExponent);

  /**
   * \brief Get the idle timeout
   * \return The idle timeout for this QuicTransportParameters
   */
  uint16_t GetIdleTimeout () const;

  /**
   * \brief Set the idle timeout
   * \param idleTimeout the idle timeout for this QuicTransportParameters
   */
  void SetIdleTimeout (uint16_t idleTimeout);

  /**
   * \brief Get the initial max data limit
   * \return The initial max data limit for this QuicTransportParameters
   */
  uint32_t GetInitialMaxData () const;

  /**
   * \brief Set the initial max data limit
   * \param initialMaxData the initial max data limit for this QuicTransportParameters
   */
  void SetInitialMaxData (uint32_t initialMaxData);

  /**
   * \brief Get the initial max stream data limit
   * \return The initial max stream data limit for this QuicTransportParameters
   */
  uint32_t GetInitialMaxStreamData () const;

  /**
   * \brief Set the initial max stream data limit
   * \param initialMaxStreamData the initial max stream data limit for this QuicTransportParameters
   */
  void SetInitialMaxStreamData (uint32_t initialMaxStreamData);

  /**
   * \brief Get the initial max bidirectional stream id limit
   * \return The initial max bidirectional stream id limit for this QuicTransportParameters
   */
  uint32_t GetInitialMaxStreamIdBidi () const;

  /**
   * \brief Set the initial max bidirectional stream id limit
   * \param initialMaxStreamIdBidi the initial max bidirectional stream id limit for this QuicTransportParameters
   */
  void SetInitialMaxStreamIdBidi (uint32_t initialMaxStreamIdBidi);

  /**
   * \brief Get the initial max unidirectional stream id limit
   * \return The initial max unidirectional stream id limit for this QuicTransportParameters
   */
  uint32_t GetInitialMaxStreamIdUni () const;

  /**
   * \brief Set the initial max unidirectional stream id limit
   * \param initialMaxStreamIdUni the initial max unidirectional stream id limit for this QuicTransportParameters
   */
  void SetInitialMaxStreamIdUni (uint32_t initialMaxStreamIdUni);

  /**
   * \brief Get the max packet size limit
   * \return The max packet size limit for this QuicTransportParameters
   */
  uint16_t GetMaxPacketSize () const;

  /**
   * \brief Set the max packet size limit
   * \param maxPacketSize the max packet size limit for this QuicTransportParameters
   */
  void SetMaxPacketSize (uint16_t maxPacketSize);

  /**
   * \brief Get the omit connection id flag
   * \return The omit connection id flag for this QuicTransportParameters
   */
  uint8_t GetOmitConnection () const;

  /**
   * \brief Set the omit connection id flag
   * \param omitConnection the omit connection id flag for this QuicTransportParameters
   */
  void SetOmitConnection (uint8_t omitConnection);

  /**
   * Comparison operator
   * \param lhs left operand
   * \param rhs right operand
   * \return true if the operands are equal
   */
  friend bool operator== (const QuicTransportParameters &lhs, const QuicTransportParameters &rhs);

private:
  /**
   * \brief Calculates the Transpor Parameters block length (in words)
   *
   * Given the standard size of the Transport Parameters block, the method checks for options
   * and calculates the real length (in words).
   *
   * \return Transpor Parameters block length in 4-byte words
   */
  uint32_t CalculateHeaderLength () const;

  uint32_t m_initial_max_stream_data;     //!< The initial value for the maximum data that can be sent on any newly created stream
  uint32_t m_initial_max_data;            //!< The initial value for the maximum amount of data that can be sent on the connection
  uint32_t m_initial_max_stream_id_bidi;  //!< The the initial maximum number of application-owned bidirectional streams the peer may initiate
  uint16_t m_idleTimeout;                 //!< The idle timeout value in seconds
  uint8_t m_omit_connection;              //!< The flag that indicates if the connection id is required in the upcoming connection
  uint16_t m_max_packet_size;             //!< The limit on the size of packets that the endpoint is willing to receive
  //uint128_t m_stateless_reset_token;    //!< The stateless reset token
  uint8_t m_ack_delay_exponent;           //!< The exponent used to decode the ack delay field in the ACK frame
  uint32_t m_initial_max_stream_id_uni;   //!< The initial maximum number of application-owned unidirectional streams the peer may initiate
};

} // namespace ns3

#endif /* QUIC_TRANSPORT_PARAMETERS_H_ */