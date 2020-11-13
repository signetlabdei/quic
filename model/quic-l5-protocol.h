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

#ifndef QUICL5PROTOCOL_H
#define QUICL5PROTOCOL_H

#include "quic-transport-parameters.h"
#include "quic-stream.h"
#include "quic-subheader.h"


namespace ns3 {

class QuicSocketBase;
class QuicStreamBase;

/**
 * This class handles the creation and management of QUIC streams
 * and is associated to a QuicSocketBase object
 */

/**
 * \ingroup quic
 * \brief QUIC stream creation and multiplexing/demultiplexing
 *
 * This class is responsible for
 * - the creation of QUIC streams
 * - the binding of the QUIC socket to the QUIC streams
 *
 * The creation of QuicStreams are handled in the method CreateStream.
 * Upon creation, this class is responsible to the stream initialization and
 * handle multiplexing/demultiplexing of data. Demultiplexing is done by
 * receiving packets from a QUIC Socket, and forwards them to its associated
 * streams. Multiplexing is done through the DispatchSend function, which sends
 * the frames down the stack.
 *
 * \see CreateStream
 * \see DispatchSend
*/
class QuicL5Protocol : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicL5Protocol ();
  virtual ~QuicL5Protocol ();

  /**
   * \brief Set the Quic Socket associated with this stack
   *
   * \param sock a smart pointer to the Quic Socket to be associated with
   */
  void SetSocket (Ptr<QuicSocketBase> sock);

  /**
   * \brief Set the node associated with this stack
   *
   * \param node the node to be associated with
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief Set the connection id associated with this stack
   *
   * \param connId the connection id to be associated with
   */
  void SetConnectionId (uint64_t connId);

  /**
   * \brief Send a packet to the streams associated to this L5 protocol
   *
   * The streams are created if not present. Stream 0 is not used (only for handshake)
   *
   * \param data a smart pointer to a packet
   * \return always 0
   */
  int DispatchSend (Ptr<Packet> data);

  /**
   * \brief Send a packet to a specific stream
   *
   * The streams is created if not present.
   *
   * \param data a smart pointer to a packet
   * \param streamId the stream ID for the packet
   * \return always 0
   */
  int DispatchSend (Ptr<Packet> data, uint64_t streamId);

  /**
   * \brief Receive a packet from the QUIC socket implementation
   *
   * The packet is disgregated into multiple frames.
   * If a frame needs to be processed by the socket, it is sent back to the socket,
   * otherwise is forwarded to the correct stream
   *
   * \param data a smart pointer to a Packet
   * \param address the sender address
   * \return 0 if the received packet was ACK-only, -1 in case of errors, 1 if everything was OK;
   */
  int DispatchRecv (Ptr<Packet> data, Address &address);

  //int DispatchRecv(Ptr<Packet> data, uint64_t streamId, Address &address);

  /**
   * \brief Method called by a stream implementation to return a proper stream frame
   * with a subheader for the frame
   *
   * \param frame a smart pointer to a packet
   * \return the size of the frame
   */
  int Send (Ptr<Packet> frame);

  /**
   * \brief Method called by a stream implementation to return a received frame (without header)
   *
   * \param frame a smart pointer to the frame
   * \param address the address of the sender
   * \return the frame size
   */
  int Recv (Ptr<Packet> frame, Address &address);

  /**
   * \brief Create a vector with fragments of packets to be sent in different streams
   *
   * \param data a smart pointer to a Packet
   * \return a vector of packet fragmets
   */
  std::vector<Ptr<Packet> > DisgregateSend (Ptr<Packet> data);

  /**
   * \brief Create a vector of frames, corresponding to frames of different streams aggregated in a single QUIC packet
   *
   * \param data a smart pointer to the received packet
   * \return a vector of pairs with frames as smart pointers to packets and subheaders
   */
  std::vector< std::pair<Ptr<Packet>, QuicSubheader> > DisgregateRecv (Ptr<Packet> data);

  /**
   * \brief get the stream associated to the ID
   *
   * \param streamId the ID of the stream
   * \return a smart pointer to the stream object
   */
  Ptr<QuicStreamBase> SearchStream (uint64_t streamId);

  /**
   * \brief Create a stream with ID equal to the number of already created streams
   *
   * \param streamDirectionType the stream direction
   */
  void CreateStream (const QuicStream::QuicStreamDirectionTypes_t streamDirectionType);

  /**
   * \brief create streamNum streams to be associated to this L5 object
   *
   * \param streamDirectionType the QUIC stream direction type,
   *   i.e., unidirectional or bidirectional
   * \param the number of streams to be created
   */
  void CreateStream (const QuicStream::QuicStreamDirectionTypes_t streamDirectionType, uint64_t streamNum);

  /**
   * \brief Get the maximum packet size from the underlying socket
   *
   * \return the maximum packet size in bytes
   */
  uint16_t GetMaxPacketSize () const;

  /**
   * \brief Check with the QUIC socket if the packet that has just been received could contain transport parameters
   *
   * \return a boolean, true if the packet that has just been received contains transport params
   */
  bool ContainsTransportParameters ();

  /**
   * \brief Pass the transportParameters to the the QUIC socket
   *
   * \param transportParameters the QuicTransportParameters
   */
  void OnReceivedTransportParameters (QuicTransportParameters transportParameters);

  /**
   * \brief call AbortConnection in the QUIC socket implementation
   *
   * \param transportErrorCode the error code
   * \param a sentence explaining the error
   */
  void SignalAbortConnection (uint16_t transportErrorCode, const char* reasonPhrase);

  /**
   * \brief Propagate the updated max stream data values to all the streams
   *
   * \param newMaxStreamData the updated value
   */
  void UpdateInitialMaxStreamData (uint32_t newMaxStreamData);

  /**
   * \brief Return MAX_DATA for flow control (i.e., the sum of MAX_STREAM_DATA for all streams)
   *
   * \returns the new max data value
   */
  uint64_t GetMaxData ();

private:
  Ptr<QuicSocketBase> m_socket;                 //!< The Quic socket this stack is associated with
  Ptr<Node> m_node;                             //!< The node this stack is associated with
  uint64_t m_connectionId;                      //!< The connection id this stack is associated with
  std::vector<Ptr<QuicStreamBase> > m_streams;  //!< The streams this stack is associated with
};

} // namespace ns3

#endif /* QUIC_L5_PROTOCOL_H */
