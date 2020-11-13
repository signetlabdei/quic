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
 *          Umberto Paro <umberto.paro@me.com>
 *
 */

#ifndef QUICSOCKETBASE_H
#define QUICSOCKETBASE_H

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/socket.h"
#include "ns3/traced-value.h"
#include "quic-socket.h"
#include "ns3/event-id.h"
#include "quic-socket-rx-buffer.h"
#include "quic-socket-tx-buffer.h"
#include "quic-header.h"
#include "quic-subheader.h"
#include "quic-transport-parameters.h"
// #include "ns3/ipv4-end-point.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-congestion-ops.h"
#include "quic-socket-tx-scheduler.h"

namespace ns3 {

class QuicL5Protocol;
class QuicL4Protocol;

/**
 * \brief Data structure that records the congestion state of a connection
 *
 * In this data structure, basic informations that should be passed between
 * socket and the congestion control algorithm are saved.
 */
class QuicSocketState : public TcpSocketState
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicSocketState ();
  QuicSocketState (const QuicSocketState &other);
  virtual  ~QuicSocketState (void)
  {}

  // Loss Detection variables of interest
  EventId m_lossDetectionAlarm;            //!< Multi-modal alarm used for loss detection.
  uint32_t m_handshakeCount;               /**< The number of times the handshake packets have been retransmitted
                                            *   without receiving an ack. */
  uint32_t m_tlpCount;                     /**< The number of times a tail loss probe has been sent without
                                            *   receiving an ack. */
  uint32_t m_rtoCount;                      //!< The number of times an rto has been sent without receiving an ack.
  SequenceNumber32 m_largestSentBeforeRto;  //!< The last packet number sent prior to the first retransmission timeout.
  Time m_timeOfLastSentPacket;              //!< The time the most recent packet was sent.
  SequenceNumber32 m_largestAckedPacket;    //!< The largest packet number acknowledged in an ACK frame.
  Time m_latestRtt;                         /**< The most recent RTT measurement made when receiving an ack for a
                                             *   previously unacked packet. */
  Time m_smoothedRtt;                       //!< The smoothed RTT of the connection, computed as described in [RFC6298].
  Time m_rttVar;                            //!< The RTT variance, computed as described in [RFC6298].
  Time m_minRtt;                            //!< The minimum RTT seen in the connection, ignoring ack delay.
  Time m_maxAckDelay;                       /**< The maximum ack delay in an incoming ACK frame for this connection.
                                             *   Excludes ack delays for ack only packets and those that create an
                                             *   RTT sample less than m_minRtt. */
  Time m_lossTime;                          /**< The time at which the next packet will be considered lost based
                                             *   on early transmit or exceeding the reordering window in time. */

  // Congestion Control constants of interests
  uint32_t m_kMinimumWindow;      //!< Default minimum congestion window.
  double m_kLossReductionFactor;  //!< Reduction in congestion window when a new loss event is detected.

  // Congestion Control variables of interests
  SequenceNumber32 m_endOfRecovery;  /**< The largest packet number sent when QUIC detects a loss. When a larger packet
                                      *   is acknowledged, QUIC exits recovery. */

  // Loss Detection constants of interest
  uint32_t m_kMaxTLPs;                          //!< Maximum number of tail loss probes before an RTO fires.
  uint32_t m_kReorderingThreshold;              /**< Maximum reordering in packet number space before FACK style loss
                                                 *   detection considers a packet lost. */
  double m_kTimeReorderingFraction;               /**< Maximum reordering in time space before time based loss detection
                                                 *   considers a packet lost. In fraction of an RTT. */
  bool m_kUsingTimeLossDetection;               /**< Whether time based loss detection is in use. If false, uses FACK
                                                 *   style loss detection. */
  Time m_kMinTLPTimeout;                        //!< Minimum time in the future a tail loss probe alarm may be set for.
  Time m_kMinRTOTimeout;                        //!< Minimum time in the future an RTO alarm may be set for.
  Time m_kDelayedAckTimeout;                    //!< The lenght of the peer's delayed ack timer.
  uint8_t m_alarmType;                          //!< The type of the next alarm
  Time m_nextAlarmTrigger;                      //<! Time of the next alarm
  Time m_kDefaultInitialRtt;                    //!< The default RTT used before an RTT sample is taken.
  uint32_t m_kMaxPacketsReceivedBeforeAckSend;  //!< The number of packets to be received before an ACK is triggered

  // RateSample variables of interest
  uint64_t              m_delivered       {0};              //!< The total amount of data in bytes delivered so far
  Time                  m_deliveredTime   {Seconds (0)};    //!< Simulation time when m_delivered was last updated
  Time                  m_firstSentTime   {Seconds (0)};    //!< The send time of the packet that was most recently marked as delivered
  uint64_t              m_appLimitedUntil {0};              //!< Connection is application-limited until m_appLimitedUntil > m_delivered
  uint32_t              m_txItemDelivered {0};              /**< amount of data (in bytes) delivered when last packet
                                                                marked asdelivered was first sent */
  uint32_t              m_lastAckedSackedBytes {0};         //!< Size of data sacked in the last ack
  uint32_t              m_ackBytesSent    {0};              //!< amount of ACK-only bytes sent
};

/**
 * \ingroup socket
 * \ingroup quic
 *
 * \brief A base class for implementation a QUIC socket.
 *
 * This class mimics the TcpSocketBase class implementation (and documentation).
 * It contains the essential components of QUIC, as well as a sockets
 * interface for upper layers to call. This class provides capabilities to
 * set up a connection, possibly with multiple streams, and manages the life cycle of
 * the connection. As for the TCP implementation, congestion control
 * is delegated to subclasses of QuicCongestionOps. In order to mimic the
 * TcpSocketBase implementation and avoid repeating code, the implementation is
 * not perfectly compliant with the QUIC Internet Draft; e.g., sequence numbers
 * are 32 bits long instead of 64. Deviations from the standard are commented
 *
 * Congestion control interface
 * ---------------------------
 *
 * Congestion control, as in the TCP implementation, has been split from
 * QuicSocketBase. The goal is to provide compatibility with the TCP congestion
 * control implementations, as well as the possibility of extending it with
 * new QUIC-related capabilities
 *
 * Transmission Control Block (TCB)
 * --------------------------------
 *
 * Taking as a reference the TCP implementation, the variables needed to congestion
 * control classes to operate correctly have been moved inside the QuicSocketState
 * class. Extending TcpSocketState, the class contains information on both the Quic
 * and the TCP windows and thresholds as well as the Congestion state machine of TCP.
 *
 * Streams
 * --------------------------------
 *
 * Streams in QUIC constitute a lightweight, ordered byte-stream abstraction within a
 * QUIC connection. The multiplexing/demultiplexing of frames in streams is handled
 * through L5 Protocol that plays a dispatcher role. Frames are buffered in reception
 * and in transmission and are processed in QuicStreams according to QUIC semantics.
 *
 */
class QuicSocketBase : public QuicSocket
{
public:
  static const uint16_t MIN_INITIAL_PACKET_SIZE;

  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the instance TypeId
   * \return the instance TypeId
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Build an object. InitializeScheduling() must be called after construction to instantiate the frame scheduler, or the construction will fail
   *
   */
  QuicSocketBase (void);
  QuicSocketBase (const QuicSocketBase&);

  virtual ~QuicSocketBase (void);

  /**
   * \brief Install a congestion control algorithm on this socket
   *
   * \param algo Algorithm to be installed
   */
  void SetCongestionControlAlgorithm (Ptr<TcpCongestionOps> algo);

  /**
   * \brief Common part of the two Bind(), i.e. set callback to receive data
   *
   * \returns 0 on success, -1 on failure
   */
  int SetupCallback (void);

  /**
   * \brief Send the initial handshake command to the other endpoint
   *
   * This method sends packets to initiate different handshakes,
   * according to the type parameters
   *
   * \param type the type of handshake (from QuicHeader)
   * \param quicHeader the QUIC header to be used
   * \param packet a packet
   */
  void SendInitialHandshake (uint8_t type, const QuicHeader &quicHeader,
                             Ptr<Packet> packet);

  /**
   * \brief Called by QuicL5Protocol to forward to the socket a control frame.
   * In this implementation, only ACK, CONNECTION_CLOSE and APPLICATION_CLOSE
   * are supported.
   *
   * \param sub the QuicSubheader of the control frame
   */
  void OnReceivedFrame (QuicSubheader &sub);

  /**
   * \brief Called when an ACK frame is received
   *
   * \param sub the QuicSubheader of the ACK frame
   */
  void OnReceivedAckFrame (QuicSubheader &sub);

  /**
   * \brief Called on sending an ACK frame
   *
   * \return the generated ACK frame
   */
  Ptr<Packet> OnSendingAckFrame ();

  /**
   * \brief Return an object with the transport parameters of this socket
   *
   * \return the transport parameters of the socket
   */
  QuicTransportParameters OnSendingTransportParameters ();

  /**
   * \brief Called when Quic transport parameters are received
   *
   * \param transportParameters the received transport parameters
   */
  void OnReceivedTransportParameters (QuicTransportParameters transportParameters);

  /**
   * \brief Add a stream frame to the TX buffer and call SendPendingData
   *
   * \param frame a smart pointer to a packet
   * \return the size of the frame
   */
  int AppendingTx (Ptr<Packet> frame);

  /**
   * \brief Add a stream frame to the RX buffer and call NotifyDataRecv
   *
   * \param frame a smart pointer to a packet
   * \param the RX address
   * \return the size of the frame added, 0 if the buffer is full
   */
  int AppendingRx (Ptr<Packet> frame, Address &address);

  /**
   * \brief Set the L4 Protocol
   *
   * \param quic the L4 Protocol to be set
   */
  void SetQuicL4 (Ptr<QuicL4Protocol> quic);

  /**
   * \brief Initialize socket TX buffer scheduler
   */
  void InitializeScheduling ();

  /**
   * \brief Set the connection ID, e.g., for client-initiated connections
   *
   * \param connectionId the connection ID
   */
  void SetConnectionId (uint64_t connectionId);

  /**
   * \brief Get the connection ID
   *
   * \return the connection ID
   */
  uint64_t GetConnectionId (void) const;

  /**
   * \brief Set the Quic protocol version
   *
   * \param version the Quic protocol version
   */
  void SetVersion (uint32_t version);

  /**
   * \brief Get the available window
   *
   * \return the available window
   */
  uint32_t AvailableWindow () const;

  /**
   * \brief Get the connection window
   *
   * \return the connection window
   */
  uint32_t ConnectionWindow () const;



  /**
   * \brief Return total bytes in flight
   *
   * \returns total bytes in flight
   */
  uint32_t BytesInFlight () const;

  /**
   * \brief Get the maximum amount of data that can be sent on the connection
   *
   * \return the maximum amount of data that can be sent on the connection
   */
  uint32_t GetConnectionMaxData () const;

  /**
   * \brief Set the maximum amount of data that can be sent on the connection
   *
   * \param maxData the maximum amount of data that can be sent on the connection
   */
  void SetConnectionMaxData (uint32_t maxData);

  /**
   * \brief Get the maximum amount of data per stream
   *
   * 0 means unlimited
   *
   * \return the maximum amount of data per stream
   */
  uint32_t GetInitialMaxStreamData () const;

  /**
   * \brief Get the state in the Congestion state machine
   *
   * \return the state in the Congestion state machine
   */
  QuicSocket::QuicStates_t GetSocketState () const;

  /**
   * \brief abort the connection due to an error
   *
   * \param transportErrorCode the error code
   * \param a sentence explaining the error
   * \param applicationClose a bool that signals that the application trigger the abortion
   */
  void AbortConnection (uint16_t transportErrorCode, const char* reasonPhrase,
                        bool applicationClose = false);

  /**
   * \brief Check if transport parameters are ever being received
   *
   * \return true if received, false otherwise
   */
  bool GetReceivedTransportParametersFlag () const;

  /**
   * \brief check if the data received in this connection exceeds MAX_DATA
   *
   * \param a vector of pairs with received frames and subheaders
   * \return a boolean, true if the limit was exceeded
   */
  bool CheckIfPacketOverflowMaxDataLimit (std::vector<std::pair<Ptr<Packet>, QuicSubheader> > disgregated);

  /**
   * \brief Get the maximum of stream ID (i.e., number of streams - 1)
   *
   * Consider the maximum between the unidirectional and bidirectional
   * stream IDs
   *
   * \return the maximum stream ID
   */
  uint32_t GetMaxStreamId () const;

  /**
   * \brief return true if the packet that has just been received contains transport params
   *
   * \return a boolean, true if the packet that has just been received contains transport params
   */
  bool CouldContainTransportParameters () const;

  /**
   * \brief Get the maximum of stream ID for bidirectional streams (i.e., number of streams - 1)
   *
   * Consider bidirectional stream IDs
   *
   * \return the maximum bidirectional stream ID
   */
  uint32_t GetMaxStreamIdBidirectional () const;

  /**
   * \brief Get the maximum of stream ID for unidirectional streams (i.e., number of streams - 1)
   *
   * Consider unidirectional stream IDs
   *
   * \return the maximum unidirectional stream ID
   */
  uint32_t GetMaxStreamIdUnidirectional () const;

  /**
   * \brief Set the socket TX buffer size.
   *
   * \param size the buffer size (in bytes)
   */
  void SetSocketSndBufSize (uint32_t size);

  /**
   * \brief Get the socket TX buffer size.
   é
   * \returns the buffer size (in bytes)
   */
  uint32_t GetSocketSndBufSize (void) const;

  /**
   * \brief Set the socket RX buffer size.
   *
   * \param size the buffer size (in bytes)
   */
  void SetSocketRcvBufSize (uint32_t size);

  /**
   * \brief Get the socket RX buffer size.
   *
   * \returns the buffer size (in bytes)
   */
  uint32_t GetSocketRcvBufSize (void) const;

  /**
   * \brief Schedule a queue ACK has if needed
   */
  void MaybeQueueAck ();

  /**
   * \brief Callback function to hook to QuicSocketState congestion window
   *
   * \param oldValue old cWnd value
   * \param newValue new cWnd value
   */
  void UpdateCwnd (uint32_t oldValue, uint32_t newValue);

  /**
   * \brief Callback function to hook to QuicSocketState slow start threshold
   *
   * \param oldValue old ssTh value
   * \param newValue new ssTh value
   */
  void UpdateSsThresh (uint32_t oldValue, uint32_t newValue);

  /**
   * \brief Callback function to hook to QuicSocketState congestion state
   *
   * \param oldValue old congestion state value
   * \param newValue new congestion state value
   */
  void UpdateCongState (TcpSocketState::TcpCongState_t oldValue,
                        TcpSocketState::TcpCongState_t newValue);

  /**
   * \brief Callback function to hook to QuicSocketState high tx mark
   *
   * \param oldValue old high tx mark
   * \param newValue new high tx mark
   */
  void UpdateHighTxMark (SequenceNumber32 oldValue, SequenceNumber32 newValue);

  /**
   * \brief Callback function to hook to QuicSocketState next tx sequence
   *
   * \param oldValue old nextTxSeq value
   * \param newValue new nextTxSeq value
   */
  void UpdateNextTxSequence (SequenceNumber32 oldValue, SequenceNumber32 newValue);

  /**
   * \brief Set the initial Slow Start Threshold.
   *
   * \param threshold the Slow Start Threshold (in bytes)
   */
  void SetInitialSSThresh (uint32_t threshold);

  /**
   * \brief Get the initial Slow Start Threshold.
   *
   * \returns the Slow Start Threshold (in bytes)
   */
  uint32_t GetInitialSSThresh (void) const;

  /**
   * \brief Set the size of initial packet of the handshake
   * It must be at least 1200 bytes
   *
   * \param the size in bytes
   */
  void SetInitialPacketSize (uint32_t size);

  /**
   * \brief Get the size of initial packet of the handshake
   *
   * \returns the size (in bytes)
   */
  uint32_t GetInitialPacketSize (void) const;

  // Implementation of ns3::Socket virtuals

  /**
   * Send a packet on a stream
   *
   * \param p a smart pointer for the packet
   * \param flags the streamId
   */
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  /**
   * Send a packet without specifying the stream
   * Load balancing of the different streams will be applied
   *
   * \param p a smart pointer for the packet
   */
  virtual int Send (Ptr<Packet> p);
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress);
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress);
  virtual int Bind (void);  // Bind a socket by setting up the UDP socket in QuicL4Protocol
  virtual int Bind (const Address &address);
  virtual int Bind6 (void);
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice);
  virtual void SetSegSize (uint32_t size);
  virtual uint32_t GetSegSize (void) const;
  virtual int Listen (void);
  virtual int Connect (const Address &address);  // Setup endpoint and create the QUIC L5 protocol
  virtual int Close (void);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);
  virtual void SetNode (Ptr<Node> node);
  virtual Ptr<Node> GetNode (void) const;
  virtual int GetSockName (Address &address) const;
  virtual int GetPeerName (Address &address) const;
  virtual uint32_t GetTxAvailable (void) const;
  virtual uint32_t GetRxAvailable (void) const;
  virtual enum SocketErrno GetErrno (void) const;
  virtual enum SocketType GetSocketType (void) const;

  /**
   * Set the latency bound for a specified stream
   *
   * \param streamId The stream ID
   * \param latency The stream's maximum latency
   */
  void SetLatency (uint32_t streamId, Time latency);

  /**
   * Get the latency bound for a specified stream
   *
   * \param streamId The stream ID
   * \return The stream's maximum latency, or 0 if the stream is not registered
   */
  Time GetLatency (uint32_t streamId);

  /**
   * Set the default latency bound
   *
   * \param latency The default maximum latency
   */
  void SetDefaultLatency (Time latency);

  /**
   * Get the default latency bound
   *
   * \param streamId The stream ID
   * \return The default maximum latency
   */
  Time GetDefaultLatency ();

  /**
   * \brief TracedCallback signature for QUIC packet transmission or reception events.
   *
   * \param [in] packet The packet.
   * \param [in] header The QuicHeaders
   * \param [in] socket This socket
   */
  typedef void (*QuicTxRxTracedCallback)(const Ptr<const Packet> packet, const QuicHeader& header,
                                         const Ptr<const QuicSocketBase> socket);

protected:
  // Implementation of QuicSocket virtuals
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  virtual bool GetAllowBroadcast (void) const;

  /**
   * \brief Creates a Quic L5 Protocol
   *
   * \return the created Quic L5 Protocol
   */
  Ptr<QuicL5Protocol> CreateStreamController ();

  /**
   * \brief Set the RTO timer (called when packets or ACKs are sent)
   */
  void SetReTxTimeout ();

  /**
   * \brief Handle what happens in case of an RTO
   */
  void ReTxTimeout ();

  /**
   * \brief Handle retransmission after loss
   */
  void DoRetransmit (std::vector<Ptr<QuicSocketTxItem> > lostPackets);

  /**
   * \brief Extract at most maxSize bytes from the TxBuffer at sequence packetNumber, add the
   *        QUIC header, and send to QuicL4Protocol
   *
   * According to the internal state of the socket, a different header is built.
   * Sequence numbers should be 64 bits in the QUIC standard, but we use 32 to
   * be compatible with the TcpSocketBase class
   *
   * \param seq the sequence number
   * \param maxSize the maximum data block to be transmitted (in bytes)
   * \param withAck forces an ACK to be sent
   * \returns the number of bytes sent
   */
  uint32_t SendDataPacket (SequenceNumber32 packetNumber, uint32_t maxSize,
                           bool withAck);

  /**
   * \brief Send a Connection Close frame
   *
   * \param errorCode the error code that caused the connection close event
   * \param phrase human-readable error description
   * \return 0 on success
   */
  uint32_t SendConnectionClosePacket (uint16_t errorCode, std::string phrase);

  /**
   * \brief Send as much pending data as possible according to the Tx window.
   *
   * \param withAck forces an ACK to be sent
   * \return the number of packets sent
   */
  uint32_t SendPendingData (bool withAck = false);

  /**
   * \brief Perform the real connection tasks: start the initial handshake for non-0-RTT
   *
   * \return 0 on success
   */
  int DoConnect (void);

  /**
   * \brief Perform the real connection tasks: start the initial 0-RTT handshake
   *
   * \return 0 on success
   */
  int DoFastConnect (void);

  /**
   * \brief Set the socket to IDLE, nullify the callbacks and remove this socket from the QuicL4Protocol
   *
   * \return 1 on success
   */
  int DoClose (void);

  /**
   * \brief receive a QUIC packet
   *
   * \param p a smart pointer to a packet
   * \param quicHeader the header of the packet
   * \param address the Address from which the packet was received
   */
  void ReceivedData (Ptr<Packet> p, const QuicHeader& quicHeader,
                     Address &address);

  /**
   * \brief Update the state of the internal state machine
   *
   * \param the new state
   */
  void SetState (TracedValue<QuicStates_t> state);

  /**
   * \brief Check if a version is supported by the QuicSocket
   *
   * \param version the version to be checked
   */
  bool IsVersionSupported (uint32_t version);

  /**
   * \brief Check if there are missing packets in the m_receivedPacketNumbers list
   *
   * \return true if there are missing packets
   */
  bool HasReceivedMissing ();

  /**
   * \brief Send an ACK packet
   */
  void SendAck ();

  /**
   * \brief Call Socket::NotifyConnectionSucceeded()
   */
  void ConnectionSucceeded (void);

  /**
   * \brief Notify Pacing
   */
  void NotifyPacingPerformed (void);
  /**
   * Send the connection close packet and schedule
   * the DoClose method
   */
  void ScheduleCloseAndSendConnectionClosePacket ();

  // Connections to other layers of the Stack
  Ipv4EndPoint* m_endPoint;      //!< the IPv4 endpoint
  Ipv6EndPoint* m_endPoint6;     //!< the IPv6 endpoint
  Ptr<Node> m_node;              //!< The associated node
  Ptr<QuicL4Protocol> m_quicl4;  //!< The associated L4 Protocol
  Ptr<QuicL5Protocol> m_quicl5;  //!< The associated L5 Protocol

  // Rx and Tx buffer management
  Ptr<QuicSocketRxBuffer> m_rxBuffer;                     //!< RX buffer
  Ptr<QuicSocketTxBuffer> m_txBuffer;                     //!< TX buffer
  uint32_t m_socketTxBufferSize;                          //!< Size of the socket TX buffer
  uint32_t m_socketRxBufferSize;                          //!< Size of the socket RX buffer
  std::vector<SequenceNumber32> m_receivedPacketNumbers;  //!< Received packet number vector
  TypeId m_schedulingTypeId;                                                      //!< The socket type of the packet scheduler
  Time m_defaultLatency;                                                                  //!< The default latency bound (only used by the EDF scheduler)

  // State-related attributes
  TracedValue<QuicStates_t> m_socketState;  //!< State in the Congestion state machine
  uint16_t m_transportErrorCode;            //!< Quic transport error code
  bool m_serverBusy;                        //!< If true, server too busy to accept new connections
  mutable enum SocketErrno m_errno;         //!< Socket error code
  bool m_connected;                         //!< Check if connection is established
  uint64_t m_connectionId;                  //!< Connection id
  uint32_t m_vers;                          //!< Quic protocol version
  QuicHeader::KeyPhase_t m_keyPhase;        //!< Key phase
  Time m_lastReceived;                      //!< Time of last received packet

  // Transport Parameters values
  uint32_t m_initial_max_stream_data;    //!< The initial value for the maximum data that can be sent on any newly created stream
  uint32_t m_max_data;                   //!< The maximum amount of data that can be sent on the connection
  uint32_t m_initial_max_stream_id_bidi; //!< The the initial maximum number of application-owned bidirectional streams the peer may initiate
  TracedValue<Time> m_idleTimeout;       //!< The idle timeout value in seconds
  bool m_omit_connection_id;             //!< The flag that indicates if the connection id is required in the upcoming connection
/*uint128_t  m_stateless_reset_token;*/  //!< The stateless reset token
  uint8_t m_ack_delay_exponent;          //!< The exponent used to decode the ack delay field in the ACK frame
  uint32_t m_initial_max_stream_id_uni;  //!< The initial maximum number of application-owned unidirectional streams the peer may initiate
  uint32_t m_maxTrackedGaps;             //!< The maximum number of gaps in an ACK

  // Transport Parameters management
  bool m_receivedTransportParameters;      //!< Check if Transport Parameters are already been received
  bool m_couldContainTransportParameters;  //!< Check if in the actual conditions can receive Transport Parameters

  // Timers and Events
  EventId m_sendPendingDataEvent;             //!< Micro-delay event to send pending data
  EventId m_retxEvent;                        //!< Retransmission event
  EventId m_idleTimeoutEvent;                 //!< Event triggered upon receiving or sending a packet, when it expires the connection closes
  EventId m_drainingPeriodEvent;              //!< Event triggered upon idle timeout or immediate connection close, when it expires all closes
  TracedValue<Time> m_rto;                    //!< Retransmit timeout
  TracedValue<Time> m_drainingPeriodTimeout;  //!< Draining Period timeout
  EventId m_sendAckEvent;                     //!< Send ACK timeout event
  EventId m_delAckEvent;                      //!< Delayed ACK timeout event
  bool m_flushOnClose;                        //!< Control behavior on connection close
  bool m_closeOnEmpty;                        //!< True if the socket will close after sending the buffered packets

  // Congestion Control
  Ptr<QuicSocketState> m_tcb;                     //!< Congestion control informations
  Ptr<TcpCongestionOps> m_congestionControl;      //!< Congestion control
  TracedValue<Time> m_lastRtt;                                 //!< Latest measured RTT
  bool m_quicCongestionControlLegacy;             //!< Quic Congestion control if true, TCP Congestion control if false
  bool m_queue_ack;                               //!< Indicates a request for a queue ACK if true
  uint32_t m_numPacketsReceivedSinceLastAckSent;  //!< Number of packets received since last ACK sent
  uint32_t m_lastMaxData;                                                 //!< Last MaxData ACK
  uint32_t m_maxDataInterval;                                     //!< Interval between successive MaxData frames in ACKs

  uint32_t m_initialPacketSize; //!< size of the first packet to be sent durin the handshake (at least 1200 bytes, per RFC)

  // Pacing timer
  Timer m_pacingTimer       {Timer::REMOVE_ON_DESTROY}; //!< Pacing Event

  /**
  * \brief Callback pointer for cWnd trace chaining
  */
  TracedCallback<uint32_t, uint32_t> m_cWndTrace;

  /**
  * \brief Callback pointer for ssTh trace chaining
  */
  TracedCallback<uint32_t, uint32_t> m_ssThTrace;

  /**
  * \brief Callback pointer for congestion state trace chaining
  */
  TracedCallback<TcpSocketState::TcpCongState_t, TcpSocketState::TcpCongState_t> m_congStateTrace;

  /**
  * \brief Callback pointer for high transmission mark trace chaining
  */
  TracedCallback<uint32_t, uint32_t> m_highTxMarkTrace;

  /**
  * \brief Callback pointer for tx sequence trace chaining
  */
  TracedCallback<uint32_t, uint32_t> m_nextTxSequenceTrace;

  // The following two traces pass a packet with a QUIC header
  TracedCallback<Ptr<const Packet>, const QuicHeader&,
                 Ptr<const QuicSocketBase> > m_txTrace; //!< Trace of transmitted packets

  TracedCallback<Ptr<const Packet>, const QuicHeader&,
                 Ptr<const QuicSocketBase> > m_rxTrace; //!< Trace of received packets

};

} //namespace ns3
#endif /* QUIC_SOCKET_BASE_H_ */
