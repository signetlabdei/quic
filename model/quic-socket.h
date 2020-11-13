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

#ifndef QUICSOCKET_H
#define QUICSOCKET_H

#include "ns3/socket.h"

/**
 * [IETF DRAFT 10 - Quic Transport: sec 4]
 *
 * QUIC versions are identified using a 32-bit unsigned number. The version 0x00000000 is
 * reserved to represent version negotiation. This version of the specification is
 * identified by 0x00000001. Other versions of QUIC might have different properties to
 * this version.
 *
 * Versions with the most significant 16 bits of the version number cleared are reserved
 * for use in future IETF consensus docs. Versions that follow the pattern 0x?a?a?a?a are
 * reserved for use in forcing version negotiation to be exercised. Version numbers used
 * to identify IETF drafts are created by adding the draft number to 0xff000000.
 *
 * Implementors are encouraged to register version numbers of QUIC that they are using for
 * private experimentation on the github wiki (here QUIC_UNI):
 * https://github.com/quicwg/base-drafts/wiki/QUIC-Versions
 */

#define QUIC_VERSION 0x00000001 // This version of the specification
#define QUIC_VERSION_NEGOTIATION 0x00000000 // Version negotiation
#define QUIC_VERSION_NEGOTIATION_PATTERN 0x0A0A0A0A // Version negotiation pattern
#define QUIC_VERSION_DRAFT_10 0xff00000A // Version IETF draft 10
#define QUIC_VERSION_NS3_IMPL 0xf1f1f1f1 // Version Unipd

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup quic
 *
 * \brief (abstract) base class of all QuicSockets
 *
 * This class exists solely for hosting QuicSocket attributes that can
 * be reused across different implementations.
 */
class QuicSocket : public Socket
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicSocket (void);
  QuicSocket (const QuicSocket&);
  virtual ~QuicSocket (void);

  /**
   * \brief Quic Socket types
   */
  typedef enum
  {
    CLIENT = 0,  //!< Client Socket
    SERVER,      //!< Server Socket
    NONE         //!< Unknown Socket
  } QuicSocketTypes_t;

  /**
   * \brief Quic Socket states
   */
  typedef enum
  {
    IDLE = 0,         //!< Idle (no state yet)
    LISTENING,        //!< Waiting for connection incoming
    CONNECTING_SVR,   //!< Connection being set-up, server side
    CONNECTING_CLT,   //!< Connection being set-up, client side
    OPEN,             //!< Connection open
    CLOSING,          //!< Termination of the connection
    LAST_STATE,       //!< Last State for debug
  } QuicStates_t;


  /**
   * \brief Literal names of QUIC Socket states for use in log messages
   */
  static const char* const QuicStateName[QuicSocket::LAST_STATE];

  /**
   * Get the socket type
   *
   * \return a QuicSocketTypes_t with the socket type
   */
  QuicSocketTypes_t GetQuicSocketType () const;

  /**
   * Set the socket type
   *
   * \param socketType a QuicSocketTypes_t with the socket type
   */
  void SetQuicSocketType (QuicSocketTypes_t socketType);

  /**
   * Check if the Quic version indicates a Version Negotiation request
   *
   * \param version the version to be checked
   * \return true if the version indicates a Version Negotiation request, false otherwise
   */
  bool CheckVersionNegotiation (uint32_t version);

protected:
  QuicSocketTypes_t m_socketType;  //!< Quic Socket type

private:
  // Indirect the attribute setting and getting through private virtual methods

  /**
   * \brief Set the send buffer size.
   * \param size the buffer size (in bytes)
   */
  //virtual void SetSndBufSize (uint32_t size) = 0;

  /**
   * \brief Get the send buffer size.
   * \returns the buffer size (in bytes)
   */
  //virtual uint32_t GetSndBufSize (void) const = 0;

  /**
   * \brief Set the receive buffer size.
   * \param size the buffer size (in bytes)
   */
  //virtual void SetRcvBufSize (uint32_t size) = 0;

  /**
   * \brief Get the receive buffer size.
   * \returns the buffer size (in bytes)
   */
  //virtual uint32_t GetRcvBufSize (void) const = 0;


  //eventuali set e get di altri parametri interessanti

};

} // namespace ns3

#endif /* QUIC_SOCKET_H */


