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

#ifndef QUICSTREAM_H
#define QUICSTREAM_H

//#include "ns3/stream.h"
#include "ns3/node.h"


namespace ns3 {

/**
 * \ingroup quic
 *
 * \see QuicStreamBase
 * \brief (abstract) base class of all QuicStreams
 *
 * This class constitutes a basic implementation of a Quic Stream.
 *
 */
class QuicStream : public Object
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicStream (void);
  virtual ~QuicStream (void);

  /**
   * \brief Quic Stream types, which are represented by the 2 lsb of stream Id
   */
  typedef enum
  {
    CLIENT_INITIATED_BIDIRECTIONAL = 0,  //!< Client Initiated Bidirectional Stream
    SERVER_INITIATED_BIDIRECTIONAL,      //!< Server Initiated Bidirectional Stream
    CLIENT_INITIATED_UNIDIRECTIONAL,     //!< Client Initiated Unidirectional Stream
    SERVER_INITIATED_UNIDIRECTIONAL,     //!< Server Initiated Unidirectional Stream
    NONE                                 //!< Unknown Stream types
  } QuicStreamTypes_t;

  /**
   * \brief Quic Stream direction types
   */
  typedef enum
  {
    SENDER = 0,     //!< Sender Stream
    RECEIVER,       //!< Receiver Stream
    BIDIRECTIONAL,  //!< Bidirectional Stream
    UNKNOWN         //!< Unknown direction type
  } QuicStreamDirectionTypes_t;

  /**
   * \brief Quic Stream states
   */
  typedef enum
  {
    IDLE = 0,     //!< Idle (no state yet)
    OPEN,         //!< Initial state for a Sender Stream [Sender]
    SEND,         //!< Endpoint transmits data in STREAM frames [Sender]
    RECV,         //!< Initial state for a Receiver Stream [Receiver]
    SIZE_KNOWN,   //!< Final offset of stream data received is known (FIN received) [Receiver]
    DATA_SENT,    //!< Application indicates that stream data sent is complete (FIN sent) [Sender]
    DATA_RECVD,   //!< All stream data has been successfully acknowledged [Sender] / all data for the stream has been received [Receiver]
    DATA_READ,    //!< All data has been delivered to the application [Receiver]
    RESET_SENT,   //!< Abandon transmission of stream data [Sender]
    RESET_RECVD,  //!< Packet containing a RST_STREAM has been acknowledged [Sender] / delivery of stream data to the application to be interrupted [Receiver]
    RESET_READ,   //!< Application has been delivered the signal indicating that the Receiver stream has been reset [Receiver]
    LAST_STATE    //!< Last State for debug
  } QuicStreamStates_t;

  /**
   * \brief return a string with the QuicStreamDirectionTypes_t
   *
   * \return a string with the QuicStreamDirectionTypes_t
   */
  virtual std::string StreamDirectionTypeToString () const = 0;

  /**
   * \brief Literal names of Quic Stream states for use in log messages
   */
  static const char* const QuicStreamStateName[QuicStream::LAST_STATE];

  /**
   * Set the stream direction
   *
   * \param streamDirectionType a QuicStreamDirectionTypes_t with the stream direction
   */
  virtual void SetStreamDirectionType (const QuicStreamDirectionTypes_t& streamDirectionType) = 0;

  /**
   * Get the stream direction
   *
   * \return a QuicStreamDirectionTypes_t with the stream direction
   */
  virtual QuicStreamDirectionTypes_t GetStreamDirectionType () = 0;

  /**
   * \brief Set the stream type
   *
   * \param streamType the stream type
   */
  virtual void SetStreamType (const QuicStreamTypes_t& streamType) = 0;

  /**
   * \brief check the input condition, and, if true, set the input stream state for the send stream
   *
   * \param condition a boolean condition
   * \param streamState the new QuicStreamStates_t to be applied if the condition holds
   */
  virtual void SetStreamStateSendIf (bool condition, const QuicStreamStates_t& streamState) = 0;

  /**
   * \brief set the input stream state for the send stream
   *
   * \param streamState the new QuicStreamStates_t to be applied
   */
  virtual void SetStreamStateSend (const QuicStreamStates_t& streamState) = 0;

  /**
   * \brief set the input stream state for the rx stream
   *
   * \param streamState the new QuicStreamStates_t to be applied
   */
  virtual void SetStreamStateRecv (const QuicStreamStates_t& streamState) = 0;

  /**
   * \brief check the input condition, and, if true, set the input stream state for the rx stream
   *
   * \param condition a boolean condition
   * \param streamState the new QuicStreamStates_t to be applied if the condition holds
   */
  virtual void SetStreamStateRecvIf (bool condition, const QuicStreamStates_t& streamState) = 0;

  /**
   * Set the node of this stream
   *
   * \param node a smart pointer to the node
   */
  virtual void SetNode (Ptr<Node> node) = 0;

  /**
   * Set the connection ID in the stream
   *
   * \param connId the connection ID
   */
  virtual void SetConnectionId (uint64_t connId) = 0;

  /**
   * \brief Set the stream ID and the stream type using the 2 least significant bits
   *
   * \param streamId the stream ID
   */
  virtual void SetStreamId (uint64_t streamId) = 0;

  /**
   * \brief Get the stream ID
   *
   * \return the stream ID
   */
  virtual uint64_t GetStreamId (void) = 0;

  /**
   * \brief Get available space left in the TX buffer
   *
   * \return the available space left in the TX buffer
   */
  virtual uint32_t GetStreamTxAvailable (void) const = 0;

};

} // namespace ns3

#endif /* QUIC_STREAM_H */


