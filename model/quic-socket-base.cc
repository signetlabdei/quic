/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 SIGNET Lab, Department of Information Engineering, University of Padova
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
/*
 #define NS_LOG_APPEND_CONTEXT \
  if (m_node and m_connectionId) { std::clog << " [node " << m_node->GetId () << " socket " << m_connectionId << "] "; }
*/

#include "ns3/abort.h"
#include "ns3/node.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/simulation-singleton.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "quic-socket-base.h"
#include "quic-congestion-ops.h"
#include "ns3/tcp-congestion-ops.h"
#include "quic-header.h"
#include "quic-l4-protocol.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/ipv6-end-point.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-option-winscale.h"
#include "ns3/tcp-option-ts.h"
#include "ns3/tcp-option-sack-permitted.h"
#include "ns3/tcp-option-sack.h"
#include "ns3/rtt-estimator.h"
#include "quic-socket-tx-edf-scheduler.h"
#include <math.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <ns3/core-module.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicSocketBase");

NS_OBJECT_ENSURE_REGISTERED (QuicSocketBase);
NS_OBJECT_ENSURE_REGISTERED (QuicSocketState);

const uint16_t QuicSocketBase::MIN_INITIAL_PACKET_SIZE = 1200;

TypeId
QuicSocketBase::GetInstanceTypeId () const
{
  return QuicSocketBase::GetTypeId ();
}

TypeId
QuicSocketBase::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicSocketBase")
    .SetParent<QuicSocket> ()
    .SetGroupName ("Internet")
    .AddConstructor<QuicSocketBase> ()
    .AddAttribute ("InitialVersion",
                   "Quic Version. The default value starts a version negotiation procedure",
                   UintegerValue (QUIC_VERSION_NEGOTIATION),
                   MakeUintegerAccessor (&QuicSocketBase::m_vers),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IdleTimeout",
                   "Idle timeout value after which the socket is closed",
                   TimeValue (Seconds (300)),
                   MakeTimeAccessor (&QuicSocketBase::m_idleTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("MaxStreamData",
                   "Stream Maximum Data",
                   UintegerValue (4294967295),      // according to the QUIC RFC this value should default to 0, and be increased by the client/server
                   MakeUintegerAccessor (&QuicSocketBase::m_initial_max_stream_data),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxData",
                   "Connection Maximum Data",
                   UintegerValue (4294967295),      // according to the QUIC RFC this value should default to 0, and be increased by the client/server
                   MakeUintegerAccessor (&QuicSocketBase::m_max_data),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxStreamIdBidi",
                   "Maximum StreamId for Bidirectional Streams",
                   UintegerValue (2),                   // according to the QUIC RFC this value should default to 0, and be increased by the client/server
                   MakeUintegerAccessor (&QuicSocketBase::m_initial_max_stream_id_bidi),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxStreamIdUni", "Maximum StreamId for Unidirectional Streams",
                   UintegerValue (2),                                  // according to the QUIC RFC this value should default to 0, and be increased by the client/server
                   MakeUintegerAccessor (&QuicSocketBase::m_initial_max_stream_id_uni),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxTrackedGaps", "Maximum number of gaps in an ACK",
                   UintegerValue (20),
                   MakeUintegerAccessor (&QuicSocketBase::m_maxTrackedGaps),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("OmitConnectionId", "Omit ConnectionId field in Short QuicHeader format",
                   BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketBase::m_omit_connection_id),
                   MakeBooleanChecker ())
    .AddAttribute ("MaxPacketSize", "Maximum Packet Size",
                   UintegerValue (1460),
                   MakeUintegerAccessor (&QuicSocketBase::GetSegSize,
                                         &QuicSocketBase::SetSegSize),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("SocketSndBufSize", "QuicSocketBase maximum transmit buffer size (bytes)",
                   UintegerValue (131072),                                  // 128k
                   MakeUintegerAccessor (&QuicSocketBase::GetSocketSndBufSize,
                                         &QuicSocketBase::SetSocketSndBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SocketRcvBufSize", "QuicSocketBase maximum receive buffer size (bytes)",
                   UintegerValue (131072),                                  // 128k
                   MakeUintegerAccessor (&QuicSocketBase::GetSocketRcvBufSize,
                                         &QuicSocketBase::SetSocketRcvBufSize),
                   MakeUintegerChecker<uint32_t> ())
    //	.AddAttribute ("StatelessResetToken, "Stateless Reset Token",
    //				   UintegerValue (0),
    //				   MakeUintegerAccessor (&QuicSocketBase::m_stateless_reset_token),
    //				   MakeUintegerChecker<uint128_t> ())
    .AddAttribute ("AckDelayExponent", "Ack Delay Exponent", 
                   UintegerValue (3),
                   MakeUintegerAccessor (&QuicSocketBase::m_ack_delay_exponent),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("FlushOnClose", "Determines the connection close behavior",
                   BooleanValue (true),
                   MakeBooleanAccessor (&QuicSocketBase::m_flushOnClose),
                   MakeBooleanChecker ())
    .AddAttribute ("kMaxTLPs",
                   "Maximum number of tail loss probes before an RTO fires",
                   UintegerValue (2),
                   MakeUintegerAccessor (&QuicSocketState::m_kMaxTLPs),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("kReorderingThreshold", "Maximum reordering in packet number space before FACK style loss detection considers a packet lost",
                   UintegerValue (3),
                   MakeUintegerAccessor (&QuicSocketState::m_kReorderingThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("kTimeReorderingFraction", "Maximum reordering in time space before time based loss detection considers a packet lost",
                   DoubleValue (9 / 8),
                   MakeDoubleAccessor (&QuicSocketState::m_kTimeReorderingFraction),
                   MakeDoubleChecker<double> (0))
    .AddAttribute ("kUsingTimeLossDetection", "Whether time based loss detection is in use", 
                   BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketState::m_kUsingTimeLossDetection),
                   MakeBooleanChecker ())
    .AddAttribute ("kMinTLPTimeout", "Minimum time in the future a tail loss probe alarm may be set for",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&QuicSocketState::m_kMinTLPTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kMinRTOTimeout", "Minimum time in the future an RTO alarm may be set for",
                   TimeValue (MilliSeconds (200)),
                   MakeTimeAccessor (&QuicSocketState::m_kMinRTOTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kDelayedAckTimeout", "The length of the peer's delayed ACK timer",
                   TimeValue (MilliSeconds (25)),
                   MakeTimeAccessor (&QuicSocketState::m_kDelayedAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kDefaultInitialRtt", "The default RTT used before an RTT sample is taken",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QuicSocketState::m_kDefaultInitialRtt),
                   MakeTimeChecker ())
    .AddAttribute ("InitialSlowStartThreshold",
                   "QUIC initial slow start threshold (bytes)",
                   UintegerValue (INT32_MAX),
                   MakeUintegerAccessor (&QuicSocketBase::GetInitialSSThresh,
                                         &QuicSocketBase::SetInitialSSThresh),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("InitialPacketSize",
                   "QUIC initial slow start threshold (bytes)",
                   UintegerValue (1200),
                   MakeUintegerAccessor (&QuicSocketBase::GetInitialPacketSize,
                                         &QuicSocketBase::SetInitialPacketSize),
                   MakeUintegerChecker<uint32_t> (
                     QuicSocketBase::MIN_INITIAL_PACKET_SIZE, UINT32_MAX))
    .AddAttribute ("SchedulingPolicy",
                   "Scheduling policy among streams",
                   TypeIdValue (QuicSocketTxScheduler::GetTypeId ()),
                   MakeTypeIdAccessor (&QuicSocketBase::m_schedulingTypeId),
                   MakeTypeIdChecker ())
    .AddAttribute ("DefaultLatency",
                   "Default latency bound for the EDF scheduler",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QuicSocketBase::m_defaultLatency),
                   MakeTimeChecker ())
    .AddAttribute ("LegacyCongestionControl", "When true, use TCP implementations for the congestion control",
                   BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketBase::m_quicCongestionControlLegacy),
                   MakeBooleanChecker ())
    .AddAttribute ("TCB",
                   "The connection's QuicSocketState",
                   PointerValue (),
                   MakePointerAccessor (&QuicSocketBase::m_tcb),
                   MakePointerChecker<QuicSocketState> ())
    // .AddTraceSource ("RTO", "Retransmission timeout",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_rto),
    //                  "ns3::Time::TracedValueCallback").AddTraceSource (
    //     "IdleTO", "Idle timeout",
    //     MakeTraceSourceAccessor (&QuicSocketBase::m_idleTimeout),
    //     "ns3::Time::TracedValueCallback").AddTraceSource (
    //     "DrainingPeriodTO", "Draining Period timeout",
    //     MakeTraceSourceAccessor (&QuicSocketBase::m_drainingPeriodTimeout),
    //     "ns3::Time::TracedValueCallback");
    .AddTraceSource ("RTO",
                     "Retransmission timeout",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_rto),
                     "ns3::Time::TracedValueCallback")
    .AddTraceSource ("RTT",
                     "Last RTT sample",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_lastRtt),
                     "ns3::Time::TracedValueCallback")
    .AddTraceSource ("NextTxSequence",
                     "Next sequence number to send (SND.NXT)",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_nextTxSequenceTrace),
                     "ns3::SequenceNumber32TracedValueCallback")
    .AddTraceSource ("HighestSequence",
                     "Highest sequence number ever sent in socket's life time",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_highTxMarkTrace),
                     "ns3::SequenceNumber32TracedValueCallback")
    // .AddTraceSource ("State",
    //                  "TCP state",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_state),
    //                  "ns3::TcpStatesTracedValueCallback")
    .AddTraceSource ("CongState",
                     "TCP Congestion machine state",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_congStateTrace),
                     "ns3::TcpSocketState::TcpCongStatesTracedValueCallback")
    // .AddTraceSource ("AdvWND",
    //                  "Advertised Window Size",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_advWnd),
    //                  "ns3::TracedValueCallback::Uint32")
    // .AddTraceSource ("RWND",
    //                  "Remote side's flow control window",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_rWnd),
    //                  "ns3::TracedValueCallback::Uint32")
    // .AddTraceSource ("BytesInFlight",
    //                  "Socket estimation of bytes in flight",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_bytesInFlight),
    //                  "ns3::TracedValueCallback::Uint32")
    // .AddTraceSource ("HighestRxSequence",
    //                  "Highest sequence number received from peer",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_highRxMark),
    //                  "ns3::SequenceNumber32TracedValueCallback")
    // .AddTraceSource ("HighestRxAck",
    //                  "Highest ack received from peer",
    //                  MakeTraceSourceAccessor (&QuicSocketBase::m_highRxAckMark),
    //                  "ns3::SequenceNumber32TracedValueCallback")
    .AddTraceSource ("CongestionWindow",
                     "The QUIC connection's congestion window",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_cWndTrace),
                     "ns3::TracedValueCallback::Uint32")
    .AddTraceSource ("SlowStartThreshold",
                     "TCP slow start threshold (bytes)",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_ssThTrace),
                     "ns3::TracedValueCallback::Uint32")
    .AddTraceSource ("Tx",
                     "Send QUIC packet to UDP protocol",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_txTrace),
                     "ns3::QuicSocketBase::QuicTxRxTracedCallback")
    .AddTraceSource ("Rx",
                     "Receive QUIC packet from UDP protocol",
                     MakeTraceSourceAccessor (&QuicSocketBase::m_rxTrace),
                     "ns3::QuicSocketBase::QuicTxRxTracedCallback")
  ;
  return tid;
}

TypeId
QuicSocketState::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::QuicSocketState")
    .SetParent<TcpSocketState> ()
    .SetGroupName ("Internet")
    .AddAttribute ("kMaxTLPs",
                   "Maximum number of tail loss probes before an RTO fires",
                   UintegerValue (2),
                   MakeUintegerAccessor (&QuicSocketState::m_kMaxTLPs),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("kReorderingThreshold",
                   "Maximum reordering in packet number space before FACK style loss detection considers a packet lost",
                   UintegerValue (3),
                   MakeUintegerAccessor (&QuicSocketState::m_kReorderingThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("kTimeReorderingFraction",
                   "Maximum reordering in time space before time based loss detection considers a packet lost",
                   DoubleValue (9 / 8),
                   MakeDoubleAccessor (&QuicSocketState::m_kTimeReorderingFraction),
                   MakeDoubleChecker<double> (0))
    .AddAttribute ("kUsingTimeLossDetection",
                   "Whether time based loss detection is in use", BooleanValue (false),
                   MakeBooleanAccessor (&QuicSocketState::m_kUsingTimeLossDetection),
                   MakeBooleanChecker ())
    .AddAttribute ("kMinTLPTimeout",
                   "Minimum time in the future a tail loss probe alarm may be set for",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&QuicSocketState::m_kMinTLPTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kMinRTOTimeout",
                   "Minimum time in the future an RTO alarm may be set for",
                   TimeValue (MilliSeconds (200)),
                   MakeTimeAccessor (&QuicSocketState::m_kMinRTOTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kDelayedAckTimeout", "The lenght of the peer's delayed ack timer",
                   TimeValue (MilliSeconds (25)),
                   MakeTimeAccessor (&QuicSocketState::m_kDelayedAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("kDefaultInitialRtt",
                   "The default RTT used before an RTT sample is taken",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QuicSocketState::m_kDefaultInitialRtt),
                   MakeTimeChecker ())
    .AddAttribute ("kMaxPacketsReceivedBeforeAckSend",
                   "The maximum number of packets without sending an ACK",
                   UintegerValue (20),
                   MakeUintegerAccessor (&QuicSocketState::m_kMaxPacketsReceivedBeforeAckSend),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

QuicSocketState::QuicSocketState ()
  : TcpSocketState (),
    m_lossDetectionAlarm (),
    m_handshakeCount (0),
    m_tlpCount (
      0),
    m_rtoCount (0),
    m_largestSentBeforeRto (0),
    m_timeOfLastSentPacket (
      Seconds (0)),
    m_largestAckedPacket (0),
    m_smoothedRtt (Seconds (0)),
    m_rttVar (0),
    m_minRtt (
      Seconds (0)),
    m_maxAckDelay (Seconds (0)),
    m_lossTime (Seconds (0)),
    m_kMinimumWindow (
      2 * m_segmentSize),
    m_kLossReductionFactor (0.5),
    m_endOfRecovery (0),
    m_kMaxTLPs (
      2),
    m_kReorderingThreshold (3),
    m_kTimeReorderingFraction (9 / 8),
    m_kUsingTimeLossDetection (
      false),
    m_kMinTLPTimeout (MilliSeconds (10)),
    m_kMinRTOTimeout (
      MilliSeconds (200)),
    m_kDelayedAckTimeout (MilliSeconds (25)),
    m_alarmType (0),
    m_nextAlarmTrigger (Seconds (100)),
    m_kDefaultInitialRtt (
      MilliSeconds (100)),
    m_kMaxPacketsReceivedBeforeAckSend (20)
{
  m_lossDetectionAlarm.Cancel ();
}

QuicSocketState::QuicSocketState (const QuicSocketState &other)
  : TcpSocketState (other),
    m_lossDetectionAlarm (other.m_lossDetectionAlarm),
    m_handshakeCount (
      other.m_handshakeCount),
    m_tlpCount (other.m_tlpCount),
    m_rtoCount (
      other.m_rtoCount),
    m_largestSentBeforeRto (
      other.m_largestSentBeforeRto),
    m_timeOfLastSentPacket (
      other.m_timeOfLastSentPacket),
    m_largestAckedPacket (
      other.m_largestAckedPacket),
    m_smoothedRtt (
      other.m_smoothedRtt),
    m_rttVar (other.m_rttVar),
    m_minRtt (
      other.m_minRtt),
    m_maxAckDelay (other.m_maxAckDelay),
    m_lossTime (
      other.m_lossTime),
    m_kMinimumWindow (other.m_kMinimumWindow),
    m_kLossReductionFactor (
      other.m_kLossReductionFactor),
    m_endOfRecovery (
      other.m_endOfRecovery),
    m_kMaxTLPs (other.m_kMaxTLPs),
    m_kReorderingThreshold (
      other.m_kReorderingThreshold),
    m_kTimeReorderingFraction (
      other.m_kTimeReorderingFraction),
    m_kUsingTimeLossDetection (
      other.m_kUsingTimeLossDetection),
    m_kMinTLPTimeout (
      other.m_kMinTLPTimeout),
    m_kMinRTOTimeout (other.m_kMinRTOTimeout),
    m_kDelayedAckTimeout (
      other.m_kDelayedAckTimeout),
    m_kDefaultInitialRtt (
      other.m_kDefaultInitialRtt),
    m_kMaxPacketsReceivedBeforeAckSend (other.m_kMaxPacketsReceivedBeforeAckSend)
{
  m_lossDetectionAlarm.Cancel ();
}

QuicSocketBase::QuicSocketBase (void)
  : QuicSocket (),
    m_endPoint (0),
    m_endPoint6 (0),
    m_node (0),
    m_quicl4 (0),
    m_quicl5 (0),
    m_socketState (
      IDLE),
    m_transportErrorCode (
      QuicSubheader::TransportErrorCodes_t::NO_ERROR),
    m_serverBusy (false),
    m_errno (
      ERROR_NOTERROR),
    m_connected (false),
    m_connectionId (0),
    m_vers (
      QUIC_VERSION_NS3_IMPL),
    m_keyPhase (QuicHeader::PHASE_ZERO),
    m_lastReceived (Seconds (0.0)),
    m_initial_max_stream_data (
      0),
    m_max_data (0),
    m_initial_max_stream_id_bidi (0),
    m_idleTimeout (
      Seconds (300.0)),
    m_omit_connection_id (false),
    m_ack_delay_exponent (
      3),
    m_initial_max_stream_id_uni (0),
    m_maxTrackedGaps (20),
    m_receivedTransportParameters (
      false),
    m_couldContainTransportParameters (true),
    m_rto (
      Seconds (30.0)),
    m_drainingPeriodTimeout (Seconds (90.0)),
    m_closeOnEmpty (false),
    m_congestionControl (
      0),
    m_lastRtt (Seconds (0.0)),
    m_queue_ack (false),
    m_numPacketsReceivedSinceLastAckSent (0),
    m_pacingTimer (Timer::REMOVE_ON_DESTROY)
{
  NS_LOG_FUNCTION (this);


  m_rxBuffer = CreateObject<QuicSocketRxBuffer> ();
  m_txBuffer = CreateObject<QuicSocketTxBuffer> ();
  m_receivedPacketNumbers = std::vector<SequenceNumber32> ();

  m_tcb = CreateObject<QuicSocketState> ();
  m_tcb->m_cWnd = m_tcb->m_initialCWnd;
  m_tcb->m_ssThresh = m_tcb->m_initialSsThresh;
  m_quicCongestionControlLegacy = false;
  m_txBuffer->SetQuicSocketState (m_tcb);

  m_tcb->m_pacingRate = m_tcb->m_maxPacingRate;
  m_pacingTimer.SetFunction (&QuicSocketBase::NotifyPacingPerformed, this);

  /**
   * [IETF DRAFT 10 - Quic Transport: sec 5.7.1]
   *
   * The initial number for a packet number MUST be selected randomly from a range between
   * 0 and 2^32 -1025 (inclusive).
   * However, in this implementation, we set the sequence number to 0
   *
   */
  if (!m_quicCongestionControlLegacy)
    {
      Ptr<UniformRandomVariable> rand =
        CreateObject<UniformRandomVariable> ();
      m_tcb->m_nextTxSequence = SequenceNumber32 (0);
      // (uint32_t) rand->GetValue (0, pow (2, 32) - 1025));
    }

  // connect callbacks
  bool ok;
  ok = m_tcb->TraceConnectWithoutContext ("CongestionWindow",
                                          MakeCallback (&QuicSocketBase::UpdateCwnd, this));
  NS_ASSERT_MSG (ok == true, "Failed connection to CWND trace");

  ok = m_tcb->TraceConnectWithoutContext ("SlowStartThreshold",
                                          MakeCallback (&QuicSocketBase::UpdateSsThresh, this));
  NS_ASSERT_MSG (ok == true, "Failed connection to SSTHR trace");

  ok = m_tcb->TraceConnectWithoutContext ("CongState",
                                          MakeCallback (&QuicSocketBase::UpdateCongState, this));
  NS_ASSERT_MSG (ok == true, "Failed connection to CongState trace");

  ok = m_tcb->TraceConnectWithoutContext ("NextTxSequence",
                                          MakeCallback (&QuicSocketBase::UpdateNextTxSequence, this));
  NS_ASSERT_MSG (ok == true, "Failed connection to TxSequence trace");

  ok = m_tcb->TraceConnectWithoutContext ("HighestSequence",
                                          MakeCallback (&QuicSocketBase::UpdateHighTxMark, this));
  NS_ASSERT_MSG (ok == true, "Failed connection to highest sequence trace");

}

QuicSocketBase::QuicSocketBase (const QuicSocketBase& sock)   // Copy constructor
  : QuicSocket (sock),
    m_endPoint (0),
    m_endPoint6 (0),
    m_node (sock.m_node),
    m_quicl4 (sock.m_quicl4),
    m_quicl5 (0),
    m_socketState (LISTENING),
    m_transportErrorCode (sock.m_transportErrorCode),
    m_serverBusy (sock.m_serverBusy),
    m_errno (sock.m_errno),
    m_connected (sock.m_connected),
    m_connectionId (0),
    m_vers (sock.m_vers),
    m_keyPhase (QuicHeader::PHASE_ZERO),
    m_lastReceived (sock.m_lastReceived),
    m_initial_max_stream_data (sock.m_initial_max_stream_data),
    m_max_data (sock.m_max_data),
    m_initial_max_stream_id_bidi (sock.m_initial_max_stream_id_bidi),
    m_idleTimeout (sock.m_idleTimeout),
    m_omit_connection_id (sock.m_omit_connection_id),
    m_ack_delay_exponent (sock.m_ack_delay_exponent),
    m_initial_max_stream_id_uni (sock.m_initial_max_stream_id_uni),
    m_maxTrackedGaps (sock.m_maxTrackedGaps),
    m_receivedTransportParameters (sock.m_receivedTransportParameters),
    m_couldContainTransportParameters (sock.m_couldContainTransportParameters),
    m_rto (sock.m_rto),
    m_drainingPeriodTimeout (sock.m_drainingPeriodTimeout),
    m_closeOnEmpty (sock.m_closeOnEmpty),
    m_lastRtt (sock.m_lastRtt),
    m_quicCongestionControlLegacy (sock.m_quicCongestionControlLegacy),
    m_queue_ack (sock.m_queue_ack),
    m_numPacketsReceivedSinceLastAckSent (sock.m_numPacketsReceivedSinceLastAckSent),
    m_lastMaxData(0),
    m_maxDataInterval(10),
    m_pacingTimer (Timer::REMOVE_ON_DESTROY),
    m_txTrace (sock.m_txTrace),
    m_rxTrace (sock.m_rxTrace)
{
  NS_LOG_FUNCTION (this);

//  Callback<void, Ptr< Socket > > vPS = MakeNullCallback<void, Ptr<Socket> > ();
//  Callback<void, Ptr<Socket>, const Address &> vPSA = MakeNullCallback<void, Ptr<Socket>, const Address &> ();
//  Callback<void, Ptr<Socket>, uint32_t> vPSUI = MakeNullCallback<void, Ptr<Socket>, uint32_t> ();
//  SetConnectCallback (vPS, vPS);
//  SetDataSentCallback (vPSUI);
//  SetSendCallback (vPSUI);
//  SetRecvCallback (vPS);
  m_txBuffer = CopyObject (sock.m_txBuffer);
  m_rxBuffer = CopyObject (sock.m_rxBuffer);
  m_receivedPacketNumbers = std::vector<SequenceNumber32> ();

  m_tcb = CopyObject (sock.m_tcb);
  if (sock.m_congestionControl)
    {
      m_congestionControl = sock.m_congestionControl->Fork ();
    }
  m_quicCongestionControlLegacy = sock.m_quicCongestionControlLegacy;
  m_txBuffer->SetQuicSocketState (m_tcb);

  m_tcb->m_pacingRate = m_tcb->m_maxPacingRate;
  m_pacingTimer.SetFunction (&QuicSocketBase::NotifyPacingPerformed, this);

  /**
   * [IETF DRAFT 10 - Quic Transport: sec 5.7.1]
   *
   * The initial value for a packet number MUST be selected randomly from a range between
   * 0 and 2^32 -1025 (inclusive).
   *
   */
  if (!m_quicCongestionControlLegacy)
    {
      Ptr<UniformRandomVariable> rand =
        CreateObject<UniformRandomVariable> ();
      m_tcb->m_nextTxSequence = SequenceNumber32 (0);
      // (uint32_t) rand->GetValue (0, pow (2, 32) - 1025));
    }
}

QuicSocketBase::~QuicSocketBase (void)
{
  NS_LOG_FUNCTION (this);

  m_node = 0;
  if (m_endPoint != nullptr)
    {
      NS_ASSERT (m_quicl4 != nullptr);
      NS_ASSERT (m_endPoint != nullptr);
      m_quicl4->DeAllocate (m_endPoint);
      NS_ASSERT (m_endPoint == nullptr);
    }
  if (m_endPoint6 != nullptr)
    {
      NS_ASSERT (m_quicl4 != nullptr);
      NS_ASSERT (m_endPoint6 != nullptr);
      m_quicl4->DeAllocate (m_endPoint6);
      NS_ASSERT (m_endPoint6 == nullptr);
    }
  m_quicl4 = 0;
  //CancelAllTimers ();
  m_pacingTimer.Cancel ();
}

/* Inherit from Socket class: Bind socket to an end-point in QuicL4Protocol */
int
QuicSocketBase::Bind (void)
{
  //NS_LOG_FUNCTION (this);
  m_endPoint = m_quicl4->Allocate ();
  if (0 == m_endPoint)
    {
      m_errno = ERROR_ADDRNOTAVAIL;
      return -1;
    }

  m_quicl4->UdpBind (this);
  return SetupCallback ();
}

int
QuicSocketBase::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this);
  if (InetSocketAddress::IsMatchingType (address))
    {
      InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
      Ipv4Address ipv4 = transport.GetIpv4 ();
      uint16_t port = transport.GetPort ();
      //SetIpTos (transport.GetTos ());
      if (ipv4 == Ipv4Address::GetAny () && port == 0)
        {
          m_endPoint = m_quicl4->Allocate ();
        }
      else if (ipv4 == Ipv4Address::GetAny () && port != 0)
        {
          m_endPoint = m_quicl4->Allocate (GetBoundNetDevice (), port);
        }
      else if (ipv4 != Ipv4Address::GetAny () && port == 0)
        {
          m_endPoint = m_quicl4->Allocate (ipv4);
        }
      else if (ipv4 != Ipv4Address::GetAny () && port != 0)
        {
          m_endPoint = m_quicl4->Allocate (GetBoundNetDevice (), ipv4, port);
        }
      if (0 == m_endPoint)
        {
          m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
          return -1;
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (address))
    {
      Inet6SocketAddress transport = Inet6SocketAddress::ConvertFrom (address);
      Ipv6Address ipv6 = transport.GetIpv6 ();
      uint16_t port = transport.GetPort ();
      if (ipv6 == Ipv6Address::GetAny () && port == 0)
        {
          m_endPoint6 = m_quicl4->Allocate6 ();
        }
      else if (ipv6 == Ipv6Address::GetAny () && port != 0)
        {
          m_endPoint6 = m_quicl4->Allocate6 (GetBoundNetDevice (), port);
        }
      else if (ipv6 != Ipv6Address::GetAny () && port == 0)
        {
          m_endPoint6 = m_quicl4->Allocate6 (ipv6);
        }
      else if (ipv6 != Ipv6Address::GetAny () && port != 0)
        {
          m_endPoint6 = m_quicl4->Allocate6 (GetBoundNetDevice (), ipv6, port);
        }
      if (0 == m_endPoint6)
        {
          m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
          return -1;
        }
    }
  else
    {
      m_errno = ERROR_INVAL;
      return -1;
    }

  m_quicl4->UdpBind (address, this);
  return SetupCallback ();
}

int
QuicSocketBase::Bind6 (void)
{
  NS_LOG_FUNCTION (this);
  m_endPoint6 = m_quicl4->Allocate6 ();
  if (0 == m_endPoint6)
    {
      m_errno = ERROR_ADDRNOTAVAIL;
      return -1;
    }

  m_quicl4->UdpBind6 (this);
  return SetupCallback ();
}

/* Inherit from Socket class: Bind this socket to the specified NetDevice */
void
QuicSocketBase::BindToNetDevice (Ptr<NetDevice> netdevice)
{
  NS_LOG_FUNCTION (this);

  m_quicl4->BindToNetDevice (this, netdevice);
}

int
QuicSocketBase::Listen (void)
{
  NS_LOG_FUNCTION (this);
  if (m_socketType == NONE)
    {
      m_socketType = SERVER;
    }

  if (m_socketState != IDLE and m_socketState != QuicSocket::CONNECTING_SVR)
    {
      //m_errno = ERROR_INVAL;
      return -1;
    }

  bool res = m_quicl4->SetListener (this);
  NS_ASSERT (res);

  SetState (LISTENING);

  return 0;
}

int
QuicSocketBase::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this);

  if (InetSocketAddress::IsMatchingType (address))
    {
      if (m_endPoint == nullptr)
        {
          if (Bind () == -1)
            {
              NS_ASSERT (m_endPoint == nullptr);
              return -1; // Bind() failed
            }
          NS_ASSERT (m_endPoint != nullptr);
        }
      InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
      m_endPoint->SetPeer (transport.GetIpv4 (), transport.GetPort ());
      //SetIpTos (transport.GetTos ());
      m_endPoint6 = nullptr;

      // Get the appropriate local address and port number from the routing protocol and set up endpoint
      /*if (SetupEndpoint () != 0)
        {
          NS_LOG_ERROR ("Route to destination does not exist ?!");
          return -1;
        }*/
    }
  else if (Inet6SocketAddress::IsMatchingType (address))
    {
      // If we are operating on a v4-mapped address, translate the address to
      // a v4 address and re-call this function
      Inet6SocketAddress transport = Inet6SocketAddress::ConvertFrom (address);
      Ipv6Address v6Addr = transport.GetIpv6 ();
      if (v6Addr.IsIpv4MappedAddress () == true)
        {
          Ipv4Address v4Addr = v6Addr.GetIpv4MappedAddress ();
          return Connect (InetSocketAddress (v4Addr, transport.GetPort ()));
        }

      if (m_endPoint6 == nullptr)
        {
          if (Bind6 () == -1)
            {
              NS_ASSERT (m_endPoint6 == nullptr);
              return -1; // Bind() failed
            }
          NS_ASSERT (m_endPoint6 != nullptr);
        }
      m_endPoint6->SetPeer (v6Addr, transport.GetPort ());
      m_endPoint = nullptr;

      // Get the appropriate local address and port number from the routing protocol and set up endpoint
      /*if (SetupEndpoint6 () != 0)
        {
          NS_LOG_ERROR ("Route to destination does not exist ?!");
          return -1;
        }*/
    }
  else
    {
      m_errno = ERROR_INVAL;
      return -1;
    }


  if (m_socketType == NONE)
    {
      m_socketType = CLIENT;
    }

  if (m_quicl5 == 0)
    {
      m_quicl5 = CreateStreamController ();
      m_quicl5->CreateStream (QuicStream::BIDIRECTIONAL, 0);   // Create Stream 0 (necessary)
    }

  // check if the address is in a list of known and authenticated addresses
  auto result = std::find (
    m_quicl4->GetAuthAddresses ().begin (), m_quicl4->GetAuthAddresses ().end (),
    InetSocketAddress::ConvertFrom (address).GetIpv4 ());

  if (result != m_quicl4->GetAuthAddresses ().end ()
      || m_quicl4->Is0RTTHandshakeAllowed ())
    {
      NS_LOG_INFO (
        "CONNECTION AUTHENTICATED Client found the Server " << InetSocketAddress::ConvertFrom (address).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (address).GetPort () << " in authenticated list");
      // connect the underlying UDP socket
      m_quicl4->UdpConnect (address, this);
      return DoFastConnect ();
    }
  else
    {
      NS_LOG_INFO (
        "CONNECTION not authenticated: cannot perform 0-RTT Handshake");
      // connect the underlying UDP socket
      m_quicl4->UdpConnect (address, this);
      return DoConnect ();
    }

}

/* Inherit from Socket class: Invoked by upper-layer application */
int
QuicSocketBase::Send (Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << flags);
  int data = 0;

  if (m_drainingPeriodEvent.IsRunning ())
    {
      NS_LOG_INFO ("Socket in draining state, cannot send packets");
      return 0;
    }

  if (flags == 0)
    {
      data = Send (p);
    }
  else
    {
      data = m_quicl5->DispatchSend (p, flags);
    }
  return data;
}

int
QuicSocketBase::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  if (m_drainingPeriodEvent.IsRunning ())
    {
      NS_LOG_INFO ("Socket in draining state, cannot send packets");
      return 0;
    }

  int data = m_quicl5->DispatchSend (p);

  return data;
}

int
QuicSocketBase::AppendingTx (Ptr<Packet> frame)
{
  NS_LOG_FUNCTION (this);

  if (m_socketState != IDLE)
    {
      bool done = m_txBuffer->Add (frame);
      if (!done)
        {
          NS_LOG_INFO ("Exceeding Socket Tx Buffer Size");
          m_errno = ERROR_MSGSIZE;
        }
      else
        {
          uint32_t win = AvailableWindow ();
          NS_LOG_DEBUG (
            "Added packet to the buffer - txBufSize = " << m_txBuffer->AppSize ()
                                                        << " AvailableWindow = " << win << " state " << QuicStateName[m_socketState]);
        }


      if (m_socketState != IDLE)
        {
          if (!m_sendPendingDataEvent.IsRunning ())
            {
              m_sendPendingDataEvent = Simulator::Schedule (
                TimeStep (1), &QuicSocketBase::SendPendingData, this,
                m_connected);
            }
        }
      if (done)
        {
          return frame->GetSize ();
        }
      return -1;
    }
  else
    {

      NS_ABORT_MSG ("Sending in state" << QuicStateName[m_socketState]);
      return -1;
    }
}

uint32_t
QuicSocketBase::SendPendingData (bool withAck)
{
  NS_LOG_FUNCTION (this << withAck);

  if (m_txBuffer->AppSize () == 0)
    {
      if (m_closeOnEmpty)
        {
          m_drainingPeriodEvent.Cancel ();
          SendConnectionClosePacket (0, "Scheduled connection close - no error");
        }
      NS_LOG_INFO ("Nothing to send");
      return false;
    }

  uint32_t nPacketsSent = 0;

  // prioritize stream 0
  while (m_txBuffer->GetNumFrameStream0InBuffer () > 0)
    {
      // check pacing timer
      if (m_tcb->m_pacing)
        {
          NS_LOG_DEBUG ("Pacing is enabled");
          if (m_pacingTimer.IsRunning ())
            {
              NS_LOG_INFO ("Skipping Packet due to pacing - for " << m_pacingTimer.GetDelayLeft ());
              break;
            }
          NS_LOG_DEBUG ("Pacing Timer is not running");
        }

      NS_LOG_DEBUG ("Send a frame for stream 0");
      SequenceNumber32 next = ++m_tcb->m_nextTxSequence;
      NS_LOG_INFO ("SN " << m_tcb->m_nextTxSequence);

      uint32_t win = AvailableWindow ();
      uint32_t connWin = ConnectionWindow ();
      uint32_t bytesInFlight = BytesInFlight ();
      NS_LOG_DEBUG (
        "BEFORE stream 0 Available Window " << win
                                            << " Connection RWnd " << connWin
                                            << " BytesInFlight " << bytesInFlight
                                            << " BufferedSize " << m_txBuffer->AppSize ()
                                            << " MaxPacketSize " << GetSegSize ());

      // uint32_t sz =
      SendDataPacket (next, 0, m_queue_ack);

      win = AvailableWindow ();
      connWin = ConnectionWindow ();
      bytesInFlight = BytesInFlight ();
      NS_LOG_DEBUG (
        "AFTER stream 0 Available Window " << win
                                           << " Connection RWnd " << connWin
                                           << " BytesInFlight " << bytesInFlight
                                           << " BufferedSize " << m_txBuffer->AppSize ()
                                           << " MaxPacketSize " << GetSegSize ());

      ++nPacketsSent;
    }
  uint32_t availableWindow = AvailableWindow ();

  while (availableWindow > 0 and m_txBuffer->AppSize () > 0)
    {
      // check draining period
      if (m_drainingPeriodEvent.IsRunning ())
        {
          NS_LOG_INFO ("Draining period: no packets can be sent");
          return false;
        }

      // check pacing timer
      if (m_tcb->m_pacing)
        {
          NS_LOG_DEBUG ("Pacing is enabled");
          if (m_pacingTimer.IsRunning ())
            {
              NS_LOG_INFO ("Skipping Packet due to pacing - for " << m_pacingTimer.GetDelayLeft ());
              break;
            }
          NS_LOG_DEBUG ("Pacing Timer is not running");
        }

      // check the state of the socket!
      if (m_socketState == CONNECTING_CLT || m_socketState == CONNECTING_SVR)
        {
          NS_LOG_INFO ("CONNECTING_CLT and CONNECTING_SVR state; no data to transmit");
          break;
        }

      uint32_t availableData = m_txBuffer->AppSize ();

      if (availableData < availableWindow and !m_closeOnEmpty)
        {
          NS_LOG_INFO ("Ask the app for more data before trying to send");
          NotifySend (GetTxAvailable ());
        }

      if (availableWindow < GetSegSize () and availableData > availableWindow and !m_closeOnEmpty)
        {
          NS_LOG_INFO ("Preventing Silly Window Syndrome. Wait to Send.");
          break;
        }

      SequenceNumber32 next = ++m_tcb->m_nextTxSequence;

      uint32_t s = std::min (availableWindow, GetSegSize ());

      uint32_t win = AvailableWindow ();
      uint32_t connWin = ConnectionWindow ();
      uint32_t bytesInFlight = BytesInFlight ();
      NS_LOG_DEBUG (
        "BEFORE Available Window " << win
                                   << " Connection RWnd " << connWin
                                   << " BytesInFlight " << bytesInFlight
                                   << " BufferedSize " << m_txBuffer->AppSize ()
                                   << " MaxPacketSize " << GetSegSize ());

      // uint32_t sz =
      SendDataPacket (next, s, withAck);

      win = AvailableWindow ();
      connWin = ConnectionWindow ();
      bytesInFlight = BytesInFlight ();
      NS_LOG_DEBUG (
        "AFTER Available Window " << win
                                  << " Connection RWnd " << connWin
                                  << " BytesInFlight " << bytesInFlight
                                  << " BufferedSize " << m_txBuffer->AppSize ()
                                  << " MaxPacketSize " << GetSegSize ());

      ++nPacketsSent;

      availableWindow = AvailableWindow ();
    }

  if (nPacketsSent > 0)
    {
      NS_LOG_INFO ("SendPendingData sent " << nPacketsSent << " packets");
    }
  else
    {
      NS_LOG_INFO ("SendPendingData no packets sent");
    }

  return nPacketsSent;
}

void
QuicSocketBase::SetSegSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  NS_ABORT_MSG_UNLESS (m_socketState == IDLE || m_tcb->m_segmentSize == size,
                       "Cannot change segment size dynamically.");

  m_tcb->m_segmentSize = size;
  // Update minimum congestion window
  m_tcb->m_initialCWnd = 2 * size;
  m_tcb->m_kMinimumWindow = 2 * size;
}

uint32_t
QuicSocketBase::GetSegSize (void) const
{
  return m_tcb->m_segmentSize;
}

void
QuicSocketBase::MaybeQueueAck ()
{
  NS_LOG_FUNCTION (this);
  ++m_numPacketsReceivedSinceLastAckSent;
  NS_LOG_INFO ("m_numPacketsReceivedSinceLastAckSent " << m_numPacketsReceivedSinceLastAckSent << " m_queue_ack " << m_queue_ack);

  // handle the list of m_receivedPacketNumbers
  if (m_receivedPacketNumbers.empty ())
    {
      NS_LOG_INFO ("Nothing to ACK");
      m_queue_ack = false;
      return;
    }

  // if(m_txBuffer->AppSize() > 0)
  // {
  //   NS_LOG_INFO("There are packets to be transmitted in the TX buffer, piggyback the ACK");
  //   return;
  // }

  if (m_numPacketsReceivedSinceLastAckSent > m_tcb->m_kMaxPacketsReceivedBeforeAckSend)
    {
      NS_LOG_INFO ("immediately send ACK - max number of unacked packets reached");
      m_queue_ack = true;
      if (!m_sendAckEvent.IsRunning ())
        {
          m_sendAckEvent = Simulator::Schedule (TimeStep (1), &QuicSocketBase::SendAck, this);
        }
    }

  if (HasReceivedMissing ())  // immediately queue the ACK
    {
      NS_LOG_INFO ("immediately send ACK - some packets have been received out of order");
      m_queue_ack = true;
      if (!m_sendAckEvent.IsRunning ())
        {
          m_sendAckEvent = Simulator::Schedule (TimeStep (1), &QuicSocketBase::SendAck, this);
        }
    }

  if (!m_queue_ack)
    {
      if (m_numPacketsReceivedSinceLastAckSent > 2) // QUIC decimation option
        {
          NS_LOG_INFO ("immediately send ACK - more than 2 packets received");
          m_queue_ack = true;
          if (!m_sendAckEvent.IsRunning ())
            {
              m_sendAckEvent = Simulator::Schedule (TimeStep (1), &QuicSocketBase::SendAck, this);
            }
        }
      else
        {
          if (!m_delAckEvent.IsRunning ())
            {
              NS_LOG_INFO ("Schedule a delayed ACK");
              // schedule a delayed ACK
              m_delAckEvent = Simulator::Schedule (
                m_tcb->m_kDelayedAckTimeout, &QuicSocketBase::SendAck, this);
            }
          else
            {
              NS_LOG_INFO ("Delayed ACK timer already running");
            }
        }
    }
}

bool
QuicSocketBase::HasReceivedMissing ()
{
  // TODO implement this
  return false;
}

void
QuicSocketBase::SendAck ()
{
  NS_LOG_FUNCTION (this);
  m_delAckEvent.Cancel ();
  m_sendAckEvent.Cancel ();
  m_queue_ack = false;

  m_numPacketsReceivedSinceLastAckSent = 0;

  Ptr<Packet> p = Create<Packet> ();
  p->AddAtEnd (OnSendingAckFrame ());
  SequenceNumber32 packetNumber = ++m_tcb->m_nextTxSequence;

  QuicHeader head;

  head = QuicHeader::CreateShort (m_connectionId, packetNumber,
                                  !m_omit_connection_id, m_keyPhase);

  // if (m_socketState == CONNECTING_SVR)
  //   {
  //     m_connected = true;
  //     head = QuicHeader::CreateHandshake (m_connectionId, m_vers,
  //                                         packetNumber);
  //   }
  // else if (m_socketState == CONNECTING_CLT)
  //   {
  //     head = QuicHeader::CreateInitial (m_connectionId, m_vers, packetNumber);
  //   }
  // else if (m_socketState == OPEN)
  //   {
  //     if (!m_connected and !m_quicl4->Is0RTTHandshakeAllowed ())
  //       {
  //         m_connected = true;
  //         head = QuicHeader::CreateHandshake (m_connectionId, m_vers,
  //                                             packetNumber);
  //       }
  //     else if (!m_connected and m_quicl4->Is0RTTHandshakeAllowed ())
  //       {
  //         head = QuicHeader::Create0RTT (m_connectionId, m_vers,
  //                                        packetNumber);
  //         m_connected = true;
  //         m_keyPhase == QuicHeader::PHASE_ONE ? m_keyPhase =
  //                                                   QuicHeader::PHASE_ZERO :
  //                                               m_keyPhase =
  //                                                   QuicHeader::PHASE_ONE;
  //       }
  //     else
  //       {
  //         head = QuicHeader::CreateShort (m_connectionId, packetNumber,
  //                                         !m_omit_connection_id, m_keyPhase);
  //       }
  //   }
  // else
  //   {
  //     NS_FATAL_ERROR("ACK not possible in this state");
  //   }

  m_txBuffer->UpdateAckSent (packetNumber, p->GetSerializedSize () + head.GetSerializedSize ());

  NS_LOG_INFO ("Send ACK packet with header " << head);
  m_quicl4->SendPacket (this, p, head);
  m_txTrace (p, head, this);
}

uint32_t
QuicSocketBase::SendDataPacket (SequenceNumber32 packetNumber,
                                uint32_t maxSize, bool withAck)
{
  NS_LOG_FUNCTION (this << packetNumber << maxSize << withAck);

  if (!m_drainingPeriodEvent.IsRunning ())
    {
      m_idleTimeoutEvent.Cancel ();
      NS_LOG_LOGIC (
        this << " SendDataPacket Schedule Close at time " << Simulator::Now ().GetSeconds () << " to expire at time " << (Simulator::Now () + m_idleTimeout.Get ()).GetSeconds ());
      m_idleTimeoutEvent = Simulator::Schedule (m_idleTimeout,
                                                &QuicSocketBase::Close, this);
    }
  else
    {
      NS_LOG_INFO ("Draining period event running");
      return -1;
    }

  Ptr<Packet> p;

  if (m_txBuffer->GetNumFrameStream0InBuffer () > 0)
    {
      p = m_txBuffer->NextStream0Sequence (packetNumber);
      NS_ABORT_MSG_IF (p == 0, "No packet for stream 0 in the buffer!");
    }
  else
    {
      NS_LOG_LOGIC (
        this << " SendDataPacket - sending packet " << packetNumber.GetValue () << " of size " << maxSize << " at time " << Simulator::Now ().GetSeconds ());
      m_idleTimeoutEvent = Simulator::Schedule (m_idleTimeout,
                                                &QuicSocketBase::Close, this);
      p = m_txBuffer->NextSequence (maxSize, packetNumber);
    }

  uint32_t sz = p->GetSize ();

  // check whether the connection is appLimited, i.e. not enough data to fill a packet
  if (sz < maxSize and m_txBuffer->AppSize () == 0 and m_tcb->m_bytesInFlight.Get () < m_tcb->m_cWnd)
    {
      NS_LOG_LOGIC ("Connection is Application-Limited. sz = " << sz << " < maxSize = " << maxSize);
      m_tcb->m_appLimitedUntil = m_tcb->m_delivered + m_tcb->m_bytesInFlight.Get () ? : 1U;
    }

  // perform pacing
  if (m_tcb->m_pacing)
    {
      NS_LOG_DEBUG ("Pacing is enabled");
      if (m_pacingTimer.IsExpired ())
        {
          NS_LOG_DEBUG ("Current Pacing Rate " << m_tcb->m_pacingRate);
          NS_LOG_DEBUG ("Pacing Timer is in expired state, activate it. Expires in " <<
                        m_tcb->m_pacingRate.Get ().CalculateBytesTxTime (sz));
          m_pacingTimer.Schedule (m_tcb->m_pacingRate.Get ().CalculateBytesTxTime (sz));
        }
      else
        {
          NS_LOG_INFO ("Pacing Timer is already in running state");
        }
    }

  bool isAckOnly = ((sz == 0) & (withAck));

  if (withAck && !m_receivedPacketNumbers.empty ())
    {
      p->AddAtEnd (OnSendingAckFrame ());
    }


  QuicHeader head;

  if (m_socketState == CONNECTING_SVR)
    {
      m_connected = true;
      head = QuicHeader::CreateHandshake (m_connectionId, m_vers,
                                          packetNumber);
    }
  else if (m_socketState == CONNECTING_CLT)
    {
      head = QuicHeader::CreateInitial (m_connectionId, m_vers, packetNumber);
    }
  else if (m_socketState == OPEN)
    {
      if (!m_connected and !m_quicl4->Is0RTTHandshakeAllowed ())
        {
          m_connected = true;
          head = QuicHeader::CreateHandshake (m_connectionId, m_vers,
                                              packetNumber);
        }
      else if (!m_connected and m_quicl4->Is0RTTHandshakeAllowed ())
        {
          head = QuicHeader::Create0RTT (m_connectionId, m_vers,
                                         packetNumber);
          m_connected = true;
          m_keyPhase == QuicHeader::PHASE_ONE ? m_keyPhase =
            QuicHeader::PHASE_ZERO :
            m_keyPhase =
              QuicHeader::PHASE_ONE;
        }
      else
        {
          head = QuicHeader::CreateShort (m_connectionId, packetNumber,
                                          !m_omit_connection_id, m_keyPhase);
        }
    }
  else
    {
      // 0 bytes sent - the socket is closed!
      return 0;
    }

  NS_LOG_INFO ("SendDataPacket of size " << p->GetSize ());
  m_quicl4->SendPacket (this, p, head);
  m_txTrace (p, head, this);
  NotifyDataSent (sz);

  m_txBuffer->UpdatePacketSent (packetNumber, sz);

  if (!m_quicCongestionControlLegacy)
    {
      DynamicCast<QuicCongestionOps> (m_congestionControl)->OnPacketSent (
        m_tcb, packetNumber, isAckOnly);
    }
  if (!isAckOnly)
    {
      SetReTxTimeout ();
    }

  return sz;
}

void
QuicSocketBase::SetReTxTimeout ()
{
  //TODO check for special packets
  NS_LOG_FUNCTION (this);

  // Don't arm the alarm if there are no packets with retransmittable data in flight.
  //if (numRetransmittablePacketsOutstanding == 0)
  if (false)
    {
      m_tcb->m_lossDetectionAlarm.Cancel ();
      return;
    }

  if (m_tcb->m_kUsingTimeLossDetection)
    {
      m_tcb->m_lossTime = Simulator::Now () + m_tcb->m_kTimeReorderingFraction * m_tcb->m_smoothedRtt;
    }

  Time alarmDuration;
  // Handshake packets are outstanding
  if (m_socketState == CONNECTING_CLT || m_socketState == CONNECTING_SVR)
    {
      NS_LOG_INFO ("Connecting, set alarm");
      // Handshake retransmission alarm.
      if (m_tcb->m_smoothedRtt == Seconds (0))
        {
          alarmDuration = 2 * m_tcb->m_kDefaultInitialRtt;
        }
      else
        {
          alarmDuration = 2 * m_tcb->m_smoothedRtt;
        }
      alarmDuration = std::max (alarmDuration + m_tcb->m_maxAckDelay,
                                m_tcb->m_kMinTLPTimeout);
      alarmDuration = alarmDuration * (2 ^ m_tcb->m_handshakeCount);
      m_tcb->m_alarmType = 0;
    }
  else if (m_tcb->m_lossTime != Seconds (0))
    {
      NS_LOG_INFO ("Early retransmit timer");
      // Early retransmit timer or time loss detection.
      alarmDuration = m_tcb->m_lossTime - m_tcb->m_timeOfLastSentPacket;
      m_tcb->m_alarmType = 1;
    }
  else if (m_tcb->m_tlpCount < m_tcb->m_kMaxTLPs)
    {
      NS_LOG_LOGIC ("m_tcb->m_tlpCount < m_tcb->m_kMaxTLPs");
      // Tail Loss Probe
      alarmDuration = std::max (
        (3 / 2) * m_tcb->m_smoothedRtt + m_tcb->m_maxAckDelay,
        m_tcb->m_kMinTLPTimeout);
      m_tcb->m_alarmType = 2;
    }
  else
    {
      NS_LOG_LOGIC ("RTO");
      alarmDuration = m_tcb->m_smoothedRtt + 4 * m_tcb->m_rttVar
        + m_tcb->m_maxAckDelay;
      alarmDuration = std::max (alarmDuration, m_tcb->m_kMinRTOTimeout);
      alarmDuration = alarmDuration * (2 ^ m_tcb->m_rtoCount);
      m_tcb->m_alarmType = 3;
    }
  NS_LOG_INFO ("Schedule ReTxTimeout at time " << Simulator::Now ().GetSeconds () << " to expire at time " << (Simulator::Now () + alarmDuration).GetSeconds ());
  NS_LOG_INFO ("Alarm after " << alarmDuration.GetSeconds () << " seconds");
  m_tcb->m_lossDetectionAlarm = Simulator::Schedule (alarmDuration,
                                                     &QuicSocketBase::ReTxTimeout, this);
  m_tcb->m_nextAlarmTrigger = Simulator::Now () + alarmDuration;
}

void
QuicSocketBase::DoRetransmit (std::vector<Ptr<QuicSocketTxItem> > lostPackets)
{
  NS_LOG_FUNCTION (this);
  // Get packets to retransmit
  SequenceNumber32 next = ++m_tcb->m_nextTxSequence;
  uint32_t toRetx = m_txBuffer->Retransmission (next);
  NS_LOG_INFO (toRetx << " bytes to retransmit");
  NS_LOG_DEBUG ("Send the retransmitted frame");
  uint32_t win = AvailableWindow ();
  uint32_t connWin = ConnectionWindow ();
  uint32_t bytesInFlight = BytesInFlight ();
  NS_LOG_DEBUG (
    "BEFORE Available Window " << win
                               << " Connection RWnd " << connWin
                               << " BytesInFlight " << bytesInFlight
                               << " BufferedSize " << m_txBuffer->AppSize ()
                               << " MaxPacketSize " << GetSegSize ());

  // Send the retransmitted data
  NS_LOG_INFO ("Retransmitted packet, next sequence number " << m_tcb->m_nextTxSequence);
  SendDataPacket (next, toRetx, m_connected);
}

void
QuicSocketBase::ReTxTimeout ()
{
  if (Simulator::Now () < m_tcb->m_nextAlarmTrigger)
    {
      NS_LOG_INFO ("Canceled alarm");
      return;
    }
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  // Handshake packets are outstanding)
  if (m_tcb->m_alarmType == 0 && (m_socketState == CONNECTING_CLT || m_socketState == CONNECTING_SVR))
    {
      // Handshake retransmission alarm.
      //TODO retransmit handshake packets
      //RetransmitAllHandshakePackets();
      m_tcb->m_handshakeCount++;
    }
  else if (m_tcb->m_alarmType == 1 && m_tcb->m_lossTime != Seconds (0))
    {
      std::vector<Ptr<QuicSocketTxItem> > lostPackets = m_txBuffer->DetectLostPackets ();
      NS_LOG_INFO ("RTO triggered: early retransmit");
      // Early retransmit or Time Loss Detection.
      if (m_quicCongestionControlLegacy)
        {
          // TCP early retransmit logic [RFC 5827]: enter recovery (RFC 6675, Sec. 5)
          if (m_tcb->m_congState != TcpSocketState::CA_RECOVERY)
            {
              m_tcb->m_congState = TcpSocketState::CA_RECOVERY;
              m_tcb->m_cWnd = m_tcb->m_ssThresh;
              m_tcb->m_endOfRecovery = m_tcb->m_highTxMark;
              m_congestionControl->CongestionStateSet (
                m_tcb, TcpSocketState::CA_RECOVERY);
              m_tcb->m_ssThresh = m_congestionControl->GetSsThresh (
                m_tcb, BytesInFlight ());
            }
        }
      else
        {
          Ptr<QuicCongestionOps> cc = dynamic_cast<QuicCongestionOps*> (&(*m_congestionControl));
          cc->OnPacketsLost (m_tcb, lostPackets);
        }
      // Retransmit all lost packets immediately
      DoRetransmit (lostPackets);
    }
  else if (m_tcb->m_alarmType == 2 && m_tcb->m_tlpCount < m_tcb->m_kMaxTLPs)
    {
      // Tail Loss Probe. Send one new data packet, do not retransmit - IETF Draft QUIC Recovery, Sec. 4.3.2
      SequenceNumber32 next = ++m_tcb->m_nextTxSequence;
      NS_LOG_INFO ("TLP triggered");
      uint32_t s = std::min (ConnectionWindow (), GetSegSize ());

      // cancel pacing to send packet immediately
      m_pacingTimer.Cancel ();

      SendDataPacket (next, s, m_connected);
      m_tcb->m_tlpCount++;
    }
  else if (m_tcb->m_alarmType == 3)
    {
      // RTO.
      if (m_tcb->m_rtoCount == 0)
        {
          m_tcb->m_largestSentBeforeRto = m_tcb->m_highTxMark;
        }
      // RTO. Send two new data packets, do not retransmit - IETF Draft QUIC Recovery, Sec. 4.3.3
      NS_LOG_INFO ("RTO triggered");
      SequenceNumber32 next = ++m_tcb->m_nextTxSequence;
      uint32_t s = std::min (AvailableWindow (), GetSegSize ());

      // cancel pacing to send packet immediately
      m_pacingTimer.Cancel ();

      SendDataPacket (next, s, m_connected);
      next = ++m_tcb->m_nextTxSequence;

      s = std::min (AvailableWindow (), GetSegSize ());

      // cancel pacing, again
      m_pacingTimer.Cancel ();

      SendDataPacket (next, s, m_connected);

      m_tcb->m_rtoCount++;
    }
}

uint32_t
QuicSocketBase::AvailableWindow () const
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("m_max_data " << m_max_data << " m_tcb->m_cWnd.Get () " << m_tcb->m_cWnd.Get ());
  uint32_t win = std::min (m_max_data, m_tcb->m_cWnd.Get ());   // Number of bytes allowed to be outstanding
  uint32_t inflight = BytesInFlight ();   // Number of outstanding bytes

  if (inflight > win)
    {
      NS_LOG_INFO (
        "InFlight=" << inflight << ", Win=" << win << " availWin=0");
      return 0;
    }

  NS_LOG_INFO (
    "InFlight=" << inflight << ", Win=" << win << " availWin=" << win - inflight);
  return win - inflight;

}

uint32_t
QuicSocketBase::ConnectionWindow () const
{
  NS_LOG_FUNCTION (this);

  uint32_t inFlight = BytesInFlight ();

  NS_LOG_INFO (
    "Returning calculated Connection: MaxData " << m_max_data << " InFlight: " << inFlight);

  return (inFlight > m_max_data) ? 0 : m_max_data - inFlight;
}

uint32_t
QuicSocketBase::BytesInFlight () const
{
  NS_LOG_FUNCTION (this);

  uint32_t bytesInFlight = m_txBuffer->BytesInFlight ();

  NS_LOG_INFO ("Returning calculated bytesInFlight: " << bytesInFlight);
  m_tcb->m_bytesInFlight = bytesInFlight;
  return bytesInFlight;
}

/* Inherit from Socket class: In QuicSocketBase, it is same as Send() call */
int
QuicSocketBase::SendTo (Ptr<Packet> p, uint32_t flags, const Address &address)
{
  NS_LOG_FUNCTION (this);

  return Send (p, flags);
}

/* Inherit from Socket class: Return data to upper-layer application. Parameter flags
 is not used. Data is returned as a packet of size no larger than maxSize */
Ptr<Packet>
QuicSocketBase::Recv (uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (flags,
                   "use of flags is not supported in QuicSocketBase::Recv()");

  if (m_rxBuffer->Size () == 0 && m_socketState == CLOSING)
    {
      return Create<Packet> ();
    }
  Ptr<Packet> outPacket = m_rxBuffer->Extract (maxSize);
  return outPacket;
}

/* Inherit from Socket class: Recv and return the remote's address */
Ptr<Packet>
QuicSocketBase::RecvFrom (uint32_t maxSize, uint32_t flags,
                          Address &fromAddress)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = m_rxBuffer->Extract (maxSize);

  if (packet != nullptr && packet->GetSize () != 0)
    {
      if (m_endPoint != nullptr)
        {
          fromAddress = InetSocketAddress (m_endPoint->GetPeerAddress (), m_endPoint->GetPeerPort ());
        }
      else if (m_endPoint6 != nullptr)
        {
          fromAddress = Inet6SocketAddress (m_endPoint6->GetPeerAddress (), m_endPoint6->GetPeerPort ());
        }
      else
        {
          fromAddress = InetSocketAddress (Ipv4Address::GetZero (), 0);
        }
    }

  return packet;
}

void
QuicSocketBase::ScheduleCloseAndSendConnectionClosePacket ()
{
  m_drainingPeriodEvent.Cancel ();
  NS_LOG_LOGIC (this << " Close Schedule DoClose at time " << Simulator::Now ().GetSeconds () << " to expire at time " << (Simulator::Now () + m_drainingPeriodTimeout.Get ()).GetSeconds ());
  m_drainingPeriodEvent = Simulator::Schedule (m_drainingPeriodTimeout, &QuicSocketBase::DoClose, this);
  SendConnectionClosePacket (0, "Scheduled connection close - no error");
}


int
QuicSocketBase::Close (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (this << " Close at time " << Simulator::Now ().GetSeconds ());

  m_receivedTransportParameters = false;

  if (m_idleTimeoutEvent.IsRunning () and m_socketState != IDLE
      and m_socketState != CLOSING)   //Connection Close from application signal
    {
      SetState (CLOSING);
      if (m_flushOnClose)
        {
          m_closeOnEmpty = true;
        }
      else
        {
          ScheduleCloseAndSendConnectionClosePacket ();
        }
    }
  else if (m_idleTimeoutEvent.IsExpired () and m_socketState != CLOSING
           and m_socketState != IDLE and m_socketState != LISTENING) //Connection Close due to Idle Period termination
    {
      SetState (CLOSING);
      m_drainingPeriodEvent.Cancel ();
      NS_LOG_LOGIC (
        this << " Close Schedule DoClose at time " << Simulator::Now ().GetSeconds () << " to expire at time " << (Simulator::Now () + m_drainingPeriodTimeout.Get ()).GetSeconds ());
      m_drainingPeriodEvent = Simulator::Schedule (m_drainingPeriodTimeout,
                                                   &QuicSocketBase::DoClose,
                                                   this);
    }
  else if (m_idleTimeoutEvent.IsExpired ()
           and m_drainingPeriodEvent.IsExpired () and m_socketState != CLOSING
           and m_socketState != IDLE) //close last listening sockets
    {
      NS_LOG_LOGIC (this << " Closing listening socket");
      DoClose ();
    }
  else if (m_idleTimeoutEvent.IsExpired ()
           and m_drainingPeriodEvent.IsExpired () and m_socketState == IDLE)
    {
      NS_LOG_LOGIC (this << " Has already been closed");
    }

  return 0;
}

/* Send a CONNECTION_CLOSE frame */
uint32_t
QuicSocketBase::SendConnectionClosePacket (uint16_t errorCode, std::string phrase)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> p = Create<Packet> ();
  SequenceNumber32 packetNumber = ++m_tcb->m_nextTxSequence;

  QuicSubheader qsb = QuicSubheader::CreateConnectionClose (errorCode, phrase.c_str ());
  p->AddHeader (qsb);


  QuicHeader head;

  head = QuicHeader::CreateShort (m_connectionId, packetNumber,
                                  !m_omit_connection_id, m_keyPhase);


  NS_LOG_DEBUG ("Send Connection Close packet with header " << head);
  m_quicl4->SendPacket (this, p, head);
  m_txTrace (p, head, this);

  return 0;
}

/* Inherit from Socket class: Signal a termination of send */
int
QuicSocketBase::ShutdownSend (void)
{
  NS_LOG_FUNCTION (this);



  return 0;
}

/* Inherit from Socket class: Signal a termination of receive */
int
QuicSocketBase::ShutdownRecv (void)
{
  NS_LOG_FUNCTION (this);

  return 0;
}

void
QuicSocketBase::SetNode (Ptr<Node> node)
{
//NS_LOG_FUNCTION (this);

  m_node = node;
}

Ptr<Node>
QuicSocketBase::GetNode (void) const
{
//NS_LOG_FUNCTION_NOARGS ();

  return m_node;
}

/* Inherit from Socket class: Return local address:port */
int
QuicSocketBase::GetSockName (Address &address) const
{
  NS_LOG_FUNCTION (this);

  return m_quicl4->GetSockName (this, address);
}

int
QuicSocketBase::GetPeerName (Address &address) const
{
  NS_LOG_FUNCTION (this);

  return m_quicl4->GetPeerName (this, address);
}

/* Inherit from Socket class: Get the max number of bytes an app can send */
uint32_t
QuicSocketBase::GetTxAvailable (void) const
{
  NS_LOG_FUNCTION (this);

  return m_txBuffer->Available ();
}

/* Inherit from Socket class: Get the max number of bytes an app can read */
uint32_t
QuicSocketBase::GetRxAvailable (void) const
{
  NS_LOG_FUNCTION (this);

  return m_rxBuffer->Available ();
}

/* Inherit from Socket class: Returns error code */
enum Socket::SocketErrno
QuicSocketBase::GetErrno (void) const
{
  return m_errno;
}

/* Inherit from Socket class: Returns socket type, NS3_SOCK_STREAM */
enum Socket::SocketType
QuicSocketBase::GetSocketType (void) const
{
  return NS3_SOCK_STREAM;
}

//////////////////////////////////////////////////////////////////////////////////////

/* Clean up after Bind. Set up callback functions in the end-point. */
int
QuicSocketBase::SetupCallback (void)
{
  NS_LOG_FUNCTION (this);

  if (m_quicl4 == 0)
    {
      return -1;
    }
  else
    {
      m_quicl4->SetRecvCallback (
        MakeCallback (&QuicSocketBase::ReceivedData, this), this);
    }

  return 0;
}

int
QuicSocketBase::AppendingRx (Ptr<Packet> frame, Address &address)
{

  NS_LOG_FUNCTION (this);

  if (!m_rxBuffer->Add (frame))
    {
      // Insert failed: No data or RX buffer full
      NS_LOG_INFO ("Dropping packet due to full RX buffer");
      return 0;
    }
  else
    {
      NS_LOG_INFO ("Notify Data Recv");
      NotifyDataRecv ();   // trigger the application method
    }

  return frame->GetSize ();
}

void
QuicSocketBase::SetQuicL4 (Ptr<QuicL4Protocol> quic)
{
  NS_LOG_FUNCTION (this);

  m_quicl4 = quic;
}

void
QuicSocketBase::SetConnectionId (uint64_t connectionId)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_connectionId = connectionId;
}

void
QuicSocketBase::InitializeScheduling ()
{
  ObjectFactory schedulerFactory;
  schedulerFactory.SetTypeId (m_schedulingTypeId);
  Ptr<QuicSocketTxScheduler> sched = schedulerFactory.Create<QuicSocketTxScheduler> ();
  m_txBuffer->SetScheduler (sched);
  SetDefaultLatency (m_defaultLatency);
}

uint64_t
QuicSocketBase::GetConnectionId (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_connectionId;
}

void
QuicSocketBase::SetVersion (uint32_t version)
{
  NS_LOG_FUNCTION (this);

  m_vers = version;
  return;
}

//////////////////////////////////////////////////////////////////////////////////////

bool
QuicSocketBase::SetAllowBroadcast (bool allowBroadcast)
{
  NS_LOG_FUNCTION (this);

  return (!allowBroadcast);
}

bool
QuicSocketBase::GetAllowBroadcast (void) const
{
  return false;
}

Ptr<QuicL5Protocol>
QuicSocketBase::CreateStreamController ()
{
  NS_LOG_FUNCTION (this);

  Ptr<QuicL5Protocol> quicl5 = CreateObject<QuicL5Protocol> ();

  quicl5->SetSocket (this);
  quicl5->SetNode (m_node);
  quicl5->SetConnectionId (m_connectionId);

  return quicl5;
}

void
QuicSocketBase::SendInitialHandshake (uint8_t type,
                                      const QuicHeader &quicHeader,
                                      Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << m_vers);

  if (type == QuicHeader::VERSION_NEGOTIATION)
    {
      NS_LOG_INFO ("Create VERSION_NEGOTIATION");
      m_receivedTransportParameters = false;
      m_couldContainTransportParameters = true;

      std::vector<uint32_t> supportedVersions;
      supportedVersions.push_back (QUIC_VERSION);
      supportedVersions.push_back (QUIC_VERSION_DRAFT_10);
      supportedVersions.push_back (QUIC_VERSION_NS3_IMPL);

      uint8_t *buffer = new uint8_t[4 * supportedVersions.size ()];

      Ptr<Packet> payload = Create<Packet> (buffer,
                                            4 * supportedVersions.size ());

      for (uint8_t i = 0; i < (uint8_t) supportedVersions.size (); i++)
        {

          buffer[4 * i] = (supportedVersions[i]);
          buffer[4 * i + 1] = (supportedVersions[i] >> 8);
          buffer[4 * i + 2] = (supportedVersions[i] >> 16);
          buffer[4 * i + 3] = (supportedVersions[i] >> 24);
          //NS_LOG_INFO(" " << (uint64_t) buffer[4*i] << " " << (uint64_t)buffer[4*i+1] << " " << (uint64_t)buffer[4*i+2] << " " << (uint64_t)buffer[4*i+3] );

        }

      Ptr<Packet> p = Create<Packet> (buffer, 4 * supportedVersions.size ());
      QuicHeader head = QuicHeader::CreateVersionNegotiation (
        quicHeader.GetConnectionId (),
        QUIC_VERSION_NEGOTIATION,
        supportedVersions);

      // Set initial congestion window and Ssthresh
      m_tcb->m_cWnd = m_tcb->m_initialCWnd;
      m_tcb->m_ssThresh = m_tcb->m_initialSsThresh;

      m_quicl4->SendPacket (this, p, head);
      m_txTrace (p, head, this);
      NotifyDataSent (p->GetSize ());

    }
  else if (type == QuicHeader::INITIAL)
    {
      // Set initial congestion window and Ssthresh
      m_tcb->m_cWnd = m_tcb->m_initialCWnd;
      m_tcb->m_ssThresh = m_tcb->m_initialSsThresh;

      NS_LOG_INFO ("Create INITIAL");
      Ptr<Packet> p = Create<Packet> ();
      p->AddHeader (OnSendingTransportParameters ());
      // the RFC says that
      // "Clients MUST ensure that the first Initial packet they
      // send is sent in a UDP datagram that is at least 1200 octets."
      Ptr<Packet> payload = Create<Packet> (
        GetInitialPacketSize () - p->GetSize ());
      p->AddAtEnd (payload);

      m_quicl5->DispatchSend (p, 0);

    }
  else if (type == QuicHeader::RETRY)
    {
      NS_LOG_INFO ("Create RETRY");
      Ptr<Packet> p = Create<Packet> ();
      p->AddHeader (OnSendingTransportParameters ());
      Ptr<Packet> payload = Create<Packet> (
        GetInitialPacketSize () - p->GetSize ());
      p->AddAtEnd (payload);

      m_quicl5->DispatchSend (p, 0);
    }
  else if (type == QuicHeader::HANDSHAKE)
    {
      NS_LOG_INFO ("Create HANDSHAKE");
      Ptr<Packet> p = Create<Packet> ();
      if (m_socketState == CONNECTING_SVR)
        {
          p->AddHeader (OnSendingTransportParameters ());
        }

      Ptr<Packet> payload = Create<Packet> (
        GetInitialPacketSize () - p->GetSize ());
      p->AddAtEnd (payload);

      m_quicl5->DispatchSend (p, 0);
      m_congestionControl->CongestionStateSet (m_tcb,
                                               TcpSocketState::CA_OPEN);
    }
  else if (type == QuicHeader::ZRTT_PROTECTED)
    {
      NS_LOG_INFO ("Create ZRTT_PROTECTED");
      Ptr<Packet> p = Create<Packet> ();
      p->AddHeader (OnSendingTransportParameters ());

      m_quicl5->DispatchSend (p, 0);

    }
  else
    {

      NS_LOG_INFO ("Wrong Handshake Type");

      return;

    }
}

void
QuicSocketBase::OnReceivedFrame (QuicSubheader &sub)
{
  NS_LOG_FUNCTION (this << (uint64_t)sub.GetFrameType ());

  uint8_t frameType = sub.GetFrameType ();

  switch (frameType)
    {

      case QuicSubheader::ACK:
        NS_LOG_INFO ("Received ACK frame");
        OnReceivedAckFrame (sub);
        break;

      case QuicSubheader::CONNECTION_CLOSE:
        NS_LOG_INFO ("Received CONNECTION_CLOSE frame");
        Close ();
        break;

      case QuicSubheader::APPLICATION_CLOSE:
        NS_LOG_INFO ("Received APPLICATION_CLOSE frame");
        DoClose ();
        break;

      case QuicSubheader::PADDING:
        NS_LOG_INFO ("Received PADDING frame");
        // no need to do anything
        break;

      case QuicSubheader::MAX_DATA:
        // set the maximum amount of data that can be sent
        // on this connection
        NS_LOG_INFO ("Received MAX_DATA frame");
        SetConnectionMaxData (sub.GetMaxData ());
        break;

      case QuicSubheader::MAX_STREAM_ID:
        // TODO update the maximum stream ID
        NS_LOG_INFO ("Received MAX_STREAM_ID frame");
        break;

      case QuicSubheader::PING:
        // TODO
        NS_LOG_INFO ("Received PING frame");
        break;

      case QuicSubheader::BLOCKED:
        // TODO
        NS_LOG_INFO ("Received BLOCKED frame");
        break;

      case QuicSubheader::STREAM_ID_BLOCKED:
        // TODO
        NS_LOG_INFO ("Received STREAM_ID_BLOCKED frame");
        break;

      case QuicSubheader::NEW_CONNECTION_ID:
        // TODO
        NS_LOG_INFO ("Received NEW_CONNECTION_ID frame");
        break;

      case QuicSubheader::PATH_CHALLENGE:
        // TODO reply with a PATH_RESPONSE with the same value
        // as that carried by the PATH_CHALLENGE
        NS_LOG_INFO ("Received PATH_CHALLENGE frame");
        break;

      case QuicSubheader::PATH_RESPONSE:
        // TODO check if it matches what was sent in a PATH_CHALLENGE
        // otherwise abort with a UNSOLICITED_PATH_RESPONSE error
        NS_LOG_INFO ("Received PATH_RESPONSE frame");
        break;

      default:
        AbortConnection (
          QuicSubheader::TransportErrorCodes_t::PROTOCOL_VIOLATION,
          "Received Corrupted Frame");
        return;
    }

}

Ptr<Packet>
QuicSocketBase::OnSendingAckFrame ()
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_MSG_IF (m_receivedPacketNumbers.empty (),
                   " Sending Ack Frame without packets to acknowledge");

//m_delAckEvent.Cancel();
//m_delAckCount = 0;

  NS_LOG_INFO ("Attach an ACK frame to the packet");

  std::sort (m_receivedPacketNumbers.begin (), m_receivedPacketNumbers.end (),
             std::greater<SequenceNumber32> ());

  SequenceNumber32 largestAcknowledged = *(m_receivedPacketNumbers.begin ());

  uint32_t ackBlockCount = 0;
  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;

  std::vector<SequenceNumber32>::const_iterator curr_rec_it =
    m_receivedPacketNumbers.begin ();
  std::vector<SequenceNumber32>::const_iterator next_rec_it =
    m_receivedPacketNumbers.begin () + 1;

  for (; next_rec_it != m_receivedPacketNumbers.end ();
       ++curr_rec_it, ++next_rec_it)
    {

      if (((*curr_rec_it) - (*next_rec_it) - 1 > 0)
          and ((*curr_rec_it) != (*next_rec_it)))
        {
          //std::clog << "curr " << (*curr_rec_it) << " next " << (*next_rec_it) << " ";
          additionalAckBlocks.push_back ((*next_rec_it).GetValue ());
          gaps.push_back ((*curr_rec_it).GetValue () - 1);
          ackBlockCount++;
        }
      // Limit the number of gaps that are sent in an ACK (older packets have already been retransmitted)
      if (ackBlockCount >= m_maxTrackedGaps)
        {
          break;
        }
    }


  Time delay = Simulator::Now () - m_lastReceived;
  uint64_t ack_delay = delay.GetMicroSeconds ();
  QuicSubheader sub = QuicSubheader::CreateAck (
    largestAcknowledged.GetValue (), ack_delay, largestAcknowledged.GetValue (),
    gaps, additionalAckBlocks);

  Ptr<Packet> ackFrame = Create<Packet> ();
  ackFrame->AddHeader (sub);

  if (m_lastMaxData < m_maxDataInterval)
    {
      m_lastMaxData++;
    }
  else
    {
      QuicSubheader maxData = QuicSubheader::CreateMaxData (m_quicl5->GetMaxData ());
      ackFrame->AddHeader (maxData);
      m_lastMaxData = 0;
    }

  return ackFrame;
}

void
QuicSocketBase::OnReceivedAckFrame (QuicSubheader &sub)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Process ACK");

  // Generate RateSample
  struct RateSample * rs = m_txBuffer->GetRateSample ();
  rs->m_priorInFlight = m_tcb->m_bytesInFlight.Get ();

  uint32_t lostOut = m_txBuffer->GetLost ();
  uint32_t delivered = m_tcb->m_delivered;

  uint32_t previousWindow = m_txBuffer->BytesInFlight ();

  std::vector<uint32_t> additionalAckBlocks = sub.GetAdditionalAckBlocks ();
  std::vector<uint32_t> gaps = sub.GetGaps ();
  uint32_t largestAcknowledged = sub.GetLargestAcknowledged ();
  m_tcb->m_lastAckedSeq = largestAcknowledged;
  uint32_t ackBlockCount = sub.GetAckBlockCount ();

  NS_ABORT_MSG_IF (
    ackBlockCount != additionalAckBlocks.size ()
    and ackBlockCount != gaps.size (),
    "Received Corrupted Ack Frame.");

  std::vector<Ptr<QuicSocketTxItem> > ackedPackets = m_txBuffer->OnAckUpdate (
    m_tcb, largestAcknowledged, additionalAckBlocks, gaps);

  // Count newly acked bytes
  uint32_t ackedBytes = previousWindow - m_txBuffer->BytesInFlight ();

  m_txBuffer->GenerateRateSample ();
  rs->m_packetLoss = std::abs ((int) lostOut - (int) m_txBuffer->GetLost ());
  m_tcb->m_lastAckedSackedBytes = m_tcb->m_delivered - delivered;

  // RTO packet acknowledged - IETF Draft QUIC Recovery, Sec. 4.3.3
  if (m_tcb->m_rtoCount > 0)
    {
      // Packets after the RTO have been acknowledged
      if (m_tcb->m_largestSentBeforeRto.GetValue () < largestAcknowledged)
        {

          uint32_t newPackets = (largestAcknowledged
                                 - m_tcb->m_largestSentBeforeRto.GetValue ()) / GetSegSize ();
          uint32_t inFlightBeforeRto = m_txBuffer->BytesInFlight ();
          m_txBuffer->ResetSentList (newPackets);
          std::vector<Ptr<QuicSocketTxItem> > lostPackets =
            m_txBuffer->DetectLostPackets ();
          if (m_quicCongestionControlLegacy && !lostPackets.empty ())
            {
              // Reset congestion window and go into loss mode
              m_tcb->m_cWnd = m_tcb->m_kMinimumWindow;
              m_tcb->m_endOfRecovery = m_tcb->m_highTxMark;
              m_tcb->m_ssThresh = m_congestionControl->GetSsThresh (
                m_tcb, inFlightBeforeRto);
              m_tcb->m_congState = TcpSocketState::CA_LOSS;
              m_congestionControl->CongestionStateSet (
                m_tcb, TcpSocketState::CA_LOSS);
            }
        }
      else
        {
          m_tcb->m_rtoCount = 0;
        }
    }

  // Tail loss probe packet acknowledged - IETF Draft QUIC Recovery, Sec. 4.3.2
  if (m_tcb->m_tlpCount > 0 && !ackedPackets.empty ())
    {
      m_tcb->m_tlpCount = 0;
    }

  // Find lost packets
  std::vector<Ptr<QuicSocketTxItem> > lostPackets =
    m_txBuffer->DetectLostPackets ();
  // Recover from losses
  if (!lostPackets.empty ())
    {
      if (m_quicCongestionControlLegacy)
        {
          //Enter recovery (RFC 6675, Sec. 5)
          if (m_tcb->m_congState != TcpSocketState::CA_RECOVERY)
            {
              m_tcb->m_congState = TcpSocketState::CA_RECOVERY;
              m_tcb->m_endOfRecovery = m_tcb->m_highTxMark;
              m_congestionControl->CongestionStateSet (
                m_tcb, TcpSocketState::CA_RECOVERY);
              m_tcb->m_ssThresh = m_congestionControl->GetSsThresh (
                m_tcb, BytesInFlight ());
              m_tcb->m_cWnd = m_tcb->m_ssThresh;
            }
          NS_ASSERT (m_tcb->m_congState == TcpSocketState::CA_RECOVERY);
        }
      else
        {
          DynamicCast<QuicCongestionOps> (m_congestionControl)->OnPacketsLost (
            m_tcb, lostPackets);
        }
      DoRetransmit (lostPackets);
    }
  /* else */ if (ackedBytes > 0)
    {
      if (!m_quicCongestionControlLegacy)
        {
          NS_LOG_INFO ("Update the variables in the congestion control (QUIC)");
          // Process the ACK
          DynamicCast<QuicCongestionOps> (m_congestionControl)->OnAckReceived (
            m_tcb, sub, ackedPackets, rs);
          m_lastRtt = m_tcb->m_lastRtt;
        }
      else
        {
          uint32_t ackedSegments = ackedBytes / GetSegSize ();

          NS_LOG_INFO ("Update the variables in the congestion control (legacy), ackedBytes "
                       << ackedBytes << " ackedSegments " << ackedSegments);
          // new acks are ordered from the highest packet number to the smalles
          Ptr<QuicSocketTxItem> lastAcked = ackedPackets.at (0);

          NS_LOG_LOGIC ("Updating RTT estimate");
          // If the largest acked is newly acked, update the RTT.
          if (lastAcked->m_packetNumber >= m_tcb->m_largestAckedPacket)
            {
              Time ackDelay = MicroSeconds (sub.GetAckDelay ());
              m_tcb->m_lastRtt = Now () - lastAcked->m_lastSent - ackDelay;
              m_lastRtt = m_tcb->m_lastRtt;
            }
          if (m_tcb->m_congState != TcpSocketState::CA_RECOVERY
              && m_tcb->m_congState != TcpSocketState::CA_LOSS)
            {
              // Increase the congestion window
              m_congestionControl->PktsAcked (m_tcb, ackedSegments,
                                              m_tcb->m_lastRtt);
              m_congestionControl->IncreaseWindow (m_tcb, ackedSegments);
            }
          else
            {
              if (m_tcb->m_endOfRecovery.GetValue () > largestAcknowledged)
                {
                  m_congestionControl->PktsAcked (m_tcb, ackedSegments,
                                                  m_tcb->m_lastRtt);
                  m_congestionControl->IncreaseWindow (m_tcb, ackedSegments);
                }
              else
                {
                  m_tcb->m_congState = TcpSocketState::CA_OPEN;
                  m_congestionControl->PktsAcked (m_tcb, ackedSegments, m_tcb->m_lastRtt);
                  m_congestionControl->CongestionStateSet (m_tcb, TcpSocketState::CA_OPEN);
                }
            }
        }
    }
  else
    {
      NS_LOG_INFO ("Received an ACK to ack an ACK");
    }

  // notify the application that more data can be sent
  if (GetTxAvailable () > 0)
    {
      NotifySend (GetTxAvailable ());
    }

  // try to send more data
  SendPendingData (m_connected);

  // Compute timers
  SetReTxTimeout ();
}

QuicTransportParameters
QuicSocketBase::OnSendingTransportParameters ()
{
  NS_LOG_FUNCTION (this);

  QuicTransportParameters transportParameters;
  transportParameters = transportParameters.CreateTransportParameters (
    m_initial_max_stream_data, m_max_data, m_initial_max_stream_id_bidi,
    (uint16_t) m_idleTimeout.Get ().GetSeconds (),
    (uint8_t) m_omit_connection_id, m_tcb->m_segmentSize,
    m_ack_delay_exponent, m_initial_max_stream_id_uni);

  return transportParameters;
}

void
QuicSocketBase::OnReceivedTransportParameters (
  QuicTransportParameters transportParameters)
{
  NS_LOG_FUNCTION (this);

  if (m_receivedTransportParameters)
    {
      AbortConnection (
        QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR,
        "Duplicate transport parameters reception");
      return;
    }
  m_receivedTransportParameters = true;

// TODO: A client MUST NOT include a stateless reset token. A server MUST treat receipt of a stateless_reset_token_transport
//   parameter as a connection error of type TRANSPORT_PARAMETER_ERROR

  uint32_t mask = transportParameters.GetInitialMaxStreamIdBidi ()
    & 0x00000003;
  if ((mask == 0) && m_socketState != CONNECTING_CLT)
    {
      // TODO AbortConnection(QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR, "Invalid Initial Max Stream Id Bidi value provided from Server");
      return;
    }
  else if ((mask == 1) && m_socketState != CONNECTING_SVR)
    {
      // TODO AbortConnection(QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR, "Invalid Initial Max Stream Id Bidi value provided from Client");
      return;
    }

  mask = transportParameters.GetInitialMaxStreamIdUni () & 0x00000003;
  if ((mask == 2) && m_socketState != CONNECTING_CLT)
    {
      // TODO AbortConnection(QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR, "Invalid Initial Max Stream Id Uni value provided from Server");
      return;
    }
  else if ((mask == 3) && m_socketState != CONNECTING_SVR)
    {
      // TODO AbortConnection(QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR, "Invalid Initial Max Stream Id Uni value provided from Client");
      return;
    }

  if (transportParameters.GetMaxPacketSize ()
      < QuicSocketBase::MIN_INITIAL_PACKET_SIZE
      or transportParameters.GetMaxPacketSize () > 65527)
    {
      AbortConnection (
        QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR,
        "Invalid Max Packet Size value provided");
      return;
    }

// version 15 has removed the upper bound on the idle timeout
// if (transportParameters.GetIdleTimeout () > 600)
//   {
//     AbortConnection (
//         QuicSubheader::TransportErrorCodes_t::TRANSPORT_PARAMETER_ERROR,
//         "Invalid Idle Timeout value provided");
//     return;
//   }

  NS_LOG_DEBUG (
    "Before applying received transport parameters " << " m_initial_max_stream_data " << m_initial_max_stream_data << " m_max_data " << m_max_data << " m_initial_max_stream_id_bidi " << m_initial_max_stream_id_bidi << " m_idleTimeout " << m_idleTimeout << " m_omit_connection_id " << m_omit_connection_id << " m_tcb->m_segmentSize " << m_tcb->m_segmentSize << " m_ack_delay_exponent " << m_ack_delay_exponent << " m_initial_max_stream_id_uni " << m_initial_max_stream_id_uni);

  m_initial_max_stream_data = std::min (
    transportParameters.GetInitialMaxStreamData (),
    m_initial_max_stream_data);
  m_quicl5->UpdateInitialMaxStreamData (m_initial_max_stream_data);

  m_max_data = std::min (transportParameters.GetInitialMaxData (),
                         m_max_data);

  m_initial_max_stream_id_bidi = std::min (
    transportParameters.GetInitialMaxStreamIdBidi (),
    m_initial_max_stream_id_bidi);

  m_idleTimeout = Time (
    std::min (transportParameters.GetIdleTimeout (),
              (uint16_t) m_idleTimeout.Get ().GetSeconds ()) * 1e9);

  m_omit_connection_id = std::min (transportParameters.GetOmitConnection (),
                                   (uint8_t) m_omit_connection_id);

  SetSegSize (
    std::min ((uint32_t) transportParameters.GetMaxPacketSize (),
              m_tcb->m_segmentSize));

//m_stateless_reset_token = std::min(transportParameters.getStatelessResetToken(), m_stateless_reset_token);
  m_ack_delay_exponent = std::min (transportParameters.GetAckDelayExponent (),
                                   m_ack_delay_exponent);

  m_initial_max_stream_id_uni = std::min (
    transportParameters.GetInitialMaxStreamIdUni (),
    m_initial_max_stream_id_uni);

  NS_LOG_DEBUG (
    "After applying received transport parameters " << " m_initial_max_stream_data " << m_initial_max_stream_data << " m_max_data " << m_max_data << " m_initial_max_stream_id_bidi " << m_initial_max_stream_id_bidi << " m_idleTimeout " << m_idleTimeout << " m_omit_connection_id " << m_omit_connection_id << " m_tcb->m_segmentSize " << m_tcb->m_segmentSize << " m_ack_delay_exponent " << m_ack_delay_exponent << " m_initial_max_stream_id_uni " << m_initial_max_stream_id_uni);
}

int
QuicSocketBase::DoConnect (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socketState != IDLE and m_socketState != QuicSocket::LISTENING)
    {
      //m_errno = ERROR_INVAL;
      return -1;
    }

  if (m_socketState == LISTENING)
    {
      SetState (CONNECTING_SVR);
    }
  else if (m_socketState == IDLE)
    {
      SetState (CONNECTING_CLT);
      QuicHeader q;
      SendInitialHandshake (QuicHeader::INITIAL, q, 0);
    }
  return 0;
}

int
QuicSocketBase::DoFastConnect (void)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (!IsVersionSupported (m_vers),
                   "0RTT Handshake requested with wrong Initial Version");

  if (m_socketState != IDLE)
    {
      //m_errno = ERROR_INVAL;
      return -1;
    }

  else if (m_socketState == IDLE)
    {
      SetState (OPEN);
      Simulator::ScheduleNow (&QuicSocketBase::ConnectionSucceeded, this);
      m_congestionControl->CongestionStateSet (m_tcb,
                                               TcpSocketState::CA_OPEN);
      QuicHeader q;
      SendInitialHandshake (QuicHeader::ZRTT_PROTECTED, q, 0);
    }
  return 0;
}

void
QuicSocketBase::ConnectionSucceeded ()
{ // Wrapper to protected function NotifyConnectionSucceeded() so that it can
  // be called as a scheduled event
  NotifyConnectionSucceeded ();
  // The if-block below was moved from ProcessSynSent() to here because we need
  // to invoke the NotifySend() only after NotifyConnectionSucceeded() to
  // reflect the behaviour in the real world.
  if (GetTxAvailable () > 0)
    {
      NotifySend (GetTxAvailable ());
    }
}

int
QuicSocketBase::DoClose (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (this << " DoClose at time " << Simulator::Now ().GetSeconds ());

  if (m_socketState != IDLE)
    {
      SetState (IDLE);
    }

  SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  return m_quicl4->RemoveSocket (this);
}

void
QuicSocketBase::ReceivedData (Ptr<Packet> p, const QuicHeader& quicHeader,
                              Address &address)
{
  NS_LOG_FUNCTION (this);

  m_rxTrace (p, quicHeader, this);

  NS_LOG_INFO ("Received packet of size " << p->GetSize ());

  // check if this packet is not received during the draining period
  if (!m_drainingPeriodEvent.IsRunning ())
    {
      m_idleTimeoutEvent.Cancel ();   // reset the IDLE timeout
      NS_LOG_LOGIC (
        this << " ReceivedData Schedule Close at time " << Simulator::Now ().GetSeconds () << " to expire at time " << (Simulator::Now () + m_idleTimeout.Get ()).GetSeconds ());
      m_idleTimeoutEvent = Simulator::Schedule (m_idleTimeout,
                                                &QuicSocketBase::Close, this);
    }
  else   // If the socket is in Draining Period, discard the packets
    {
      return;
    }

  int onlyAckFrames = 0;
  bool unsupportedVersion = false;

  if (quicHeader.IsORTT () and m_socketState == LISTENING)
    {

      if (m_serverBusy)
        {
          AbortConnection (QuicSubheader::TransportErrorCodes_t::SERVER_BUSY,
                           "Server too busy to accept new connections");
          return;
        }

      m_couldContainTransportParameters = true;

      onlyAckFrames = m_quicl5->DispatchRecv (p, address);
      m_receivedPacketNumbers.push_back (quicHeader.GetPacketNumber ());

      m_connected = true;
      m_keyPhase == QuicHeader::PHASE_ONE ? m_keyPhase =
        QuicHeader::PHASE_ZERO :
        m_keyPhase =
          QuicHeader::PHASE_ONE;
      SetState (OPEN);
      Simulator::ScheduleNow (&QuicSocketBase::ConnectionSucceeded, this);
      m_congestionControl->CongestionStateSet (m_tcb,
                                               TcpSocketState::CA_OPEN);
      m_couldContainTransportParameters = false;

    }
  else if (quicHeader.IsInitial () and m_socketState == CONNECTING_SVR)
    {
      NS_LOG_INFO ("Server receives INITIAL");
      if (m_serverBusy)
        {
          AbortConnection (QuicSubheader::TransportErrorCodes_t::SERVER_BUSY,
                           "Server too busy to accept new connections");
          return;
        }

      if (p->GetSize () < QuicSocketBase::MIN_INITIAL_PACKET_SIZE)
        {
          std::stringstream error;
          error << "Initial Packet smaller than "
                << QuicSocketBase::MIN_INITIAL_PACKET_SIZE << " octects";
          AbortConnection (
            QuicSubheader::TransportErrorCodes_t::PROTOCOL_VIOLATION,
            error.str ().c_str ());
          return;
        }

      onlyAckFrames = m_quicl5->DispatchRecv (p, address);
      m_receivedPacketNumbers.push_back (quicHeader.GetPacketNumber ());

      if (IsVersionSupported (quicHeader.GetVersion ()))
        {
          m_couldContainTransportParameters = false;
          SendInitialHandshake (QuicHeader::HANDSHAKE, quicHeader, p);
        }
      else
        {
          NS_LOG_INFO (this << " WRONG VERSION " << quicHeader.GetVersion ());
          unsupportedVersion = true;
          SendInitialHandshake (QuicHeader::VERSION_NEGOTIATION, quicHeader,
                                p);
        }
      return;
    }
  else if (quicHeader.IsHandshake () and m_socketState == CONNECTING_CLT)   // Undefined compiler behaviour if i try to receive transport parameters
    {
      NS_LOG_INFO ("Client receives HANDSHAKE");

      onlyAckFrames = m_quicl5->DispatchRecv (p, address);
      m_receivedPacketNumbers.push_back (quicHeader.GetPacketNumber ());

      SetState (OPEN);
      Simulator::ScheduleNow (&QuicSocketBase::ConnectionSucceeded, this);
      m_congestionControl->CongestionStateSet (m_tcb,
                                               TcpSocketState::CA_OPEN);
      m_couldContainTransportParameters = false;

      SendInitialHandshake (QuicHeader::HANDSHAKE, quicHeader, p);
      return;
    }
  else if (quicHeader.IsHandshake () and m_socketState == CONNECTING_SVR)
    {
      NS_LOG_INFO ("Server receives HANDSHAKE");

      onlyAckFrames = m_quicl5->DispatchRecv (p, address);
      m_receivedPacketNumbers.push_back (quicHeader.GetPacketNumber ());

      SetState (OPEN);
      Simulator::ScheduleNow (&QuicSocketBase::ConnectionSucceeded, this);
      m_congestionControl->CongestionStateSet (m_tcb,
                                               TcpSocketState::CA_OPEN);
      SendPendingData (true);
      return;
    }
  else if (quicHeader.IsVersionNegotiation ()
           and m_socketState == CONNECTING_CLT)
    {
      NS_LOG_INFO ("Client receives VERSION_NEGOTIATION");

      uint8_t *buffer = new uint8_t[p->GetSize ()];
      p->CopyData (buffer, p->GetSize ());

      std::vector<uint32_t> receivedVersions;
      for (uint8_t i = 0; i < p->GetSize (); i = i + 4)
        {
          receivedVersions.push_back (
            buffer[i] + (buffer[i + 1] << 8) + (buffer[i + 2] << 16)
            + (buffer[i + 3] << 24));
          //NS_LOG_INFO(" " << (uint64_t) buffer[i] << " " << (uint64_t)buffer[i+1] << " " << (uint64_t)buffer[i+2] << " " << (uint64_t)buffer[i+3] );
        }

      std::vector<uint32_t> supportedVersions;
      supportedVersions.push_back (QUIC_VERSION);
      supportedVersions.push_back (QUIC_VERSION_DRAFT_10);
      supportedVersions.push_back (QUIC_VERSION_NS3_IMPL);

      uint32_t foundVersion = 0;
      for (uint8_t i = 0; i < receivedVersions.size (); i++)
        {
          for (uint8_t j = 0; j < supportedVersions.size (); j++)
            {
//			NS_LOG_INFO("rec " << receivedVersions[i] << " myvers " << m_supportedVersions[j] );
              if (receivedVersions[i] == supportedVersions[j])
                {
                  foundVersion = receivedVersions[i];
                }
            }
        }

      if (foundVersion != 0)
        {
          NS_LOG_INFO ("A matching supported version is found " << foundVersion << " re-send initial");
          m_vers = foundVersion;
          SendInitialHandshake (QuicHeader::INITIAL, quicHeader, p);
        }
      else
        {
          AbortConnection (
            QuicSubheader::TransportErrorCodes_t::VERSION_NEGOTIATION_ERROR,
            "No supported Version found by the Client");
          return;
        }
      return;
    }
  else if (quicHeader.IsShort () and m_socketState == OPEN)
    {
      // TODOACK here?
      // we need to check if the packet contains only an ACK frame
      // in this case we cannot explicitely ACK it!
      // check if delayed ACK is used
      m_receivedPacketNumbers.push_back (quicHeader.GetPacketNumber ());
      onlyAckFrames = m_quicl5->DispatchRecv (p, address);

    }
  else if (m_socketState == CLOSING)
    {

      AbortConnection (m_transportErrorCode,
                       "Received packet in Closing state");

    }
  else
    {

      return;
    }

  // trigger the process for ACK handling if the received packet was not ACK only
  NS_LOG_DEBUG ("onlyAckFrames " << onlyAckFrames << " unsupportedVersion " << unsupportedVersion);
  if (onlyAckFrames == 1 && !unsupportedVersion)
    {
      m_lastReceived = Simulator::Now ();
      NS_LOG_DEBUG ("Call MaybeQueueAck");
      MaybeQueueAck ();
    }

}

uint32_t
QuicSocketBase::GetInitialMaxStreamData () const
{
  return m_initial_max_stream_data;
}

uint32_t
QuicSocketBase::GetConnectionMaxData () const
{
  return m_max_data;
}

void
QuicSocketBase::SetConnectionMaxData (uint32_t maxData)
{
  m_max_data = maxData;
}

QuicSocket::QuicStates_t
QuicSocketBase::GetSocketState () const
{
  return m_socketState;
}

void
QuicSocketBase::SetState (TracedValue<QuicStates_t> newstate)
{
  NS_LOG_FUNCTION (this);

  if (m_quicl4->IsServer ())
    {
      NS_LOG_INFO (
        "Server " << QuicStateName[m_socketState] << " -> " << QuicStateName[newstate] << "");
    }
  else
    {
      NS_LOG_INFO (
        "Client " << QuicStateName[m_socketState] << " -> " << QuicStateName[newstate] << "");
    }

  m_socketState = newstate;
}

bool
QuicSocketBase::IsVersionSupported (uint32_t version)
{
  if (version == QUIC_VERSION || version == QUIC_VERSION_DRAFT_10
      || version == QUIC_VERSION_NS3_IMPL)
    {
      return true;
    }
  else
    {
      return false;
    }
}

void
QuicSocketBase::AbortConnection (uint16_t transportErrorCode,
                                 const char* reasonPhrase,
                                 bool applicationClose)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO (
    "Abort connection " << transportErrorCode << " because " << reasonPhrase);

  m_transportErrorCode = transportErrorCode;

  QuicSubheader quicSubheader;
  Ptr<Packet> frame = Create<Packet> ();
  if (!applicationClose)
    {
      quicSubheader = QuicSubheader::CreateConnectionClose (
        m_transportErrorCode, reasonPhrase);
    }
  else
    {
      quicSubheader = QuicSubheader::CreateApplicationClose (
        m_transportErrorCode, reasonPhrase);
    }
  frame->AddHeader (quicSubheader);

  QuicHeader quicHeader;
  switch (m_socketState)
    {
      case CONNECTING_CLT:
        quicHeader = QuicHeader::CreateInitial (m_connectionId, m_vers,
                                                m_tcb->m_nextTxSequence++);
        break;
      case CONNECTING_SVR:
        quicHeader = QuicHeader::CreateHandshake (m_connectionId, m_vers,
                                                  m_tcb->m_nextTxSequence++);
        break;
      case OPEN:
        quicHeader =
          !m_connected ?
          QuicHeader::CreateHandshake (m_connectionId, m_vers,
                                       m_tcb->m_nextTxSequence++) :
          QuicHeader::CreateShort (m_connectionId,
                                   m_tcb->m_nextTxSequence++,
                                   !m_omit_connection_id, m_keyPhase);
        break;
      case CLOSING:
        quicHeader = QuicHeader::CreateShort (m_connectionId,
                                              m_tcb->m_nextTxSequence++,
                                              !m_omit_connection_id,
                                              m_keyPhase);
        break;
      default:
        NS_ABORT_MSG (
          "AbortConnection in unfeasible Socket State for the request");
        return;
    }
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddAtEnd (frame);
  uint32_t sz = packet->GetSize ();

  m_quicl4->SendPacket (this, packet, quicHeader);
  m_txTrace (packet, quicHeader, this);
  NotifyDataSent (sz);

  Close ();
}

bool
QuicSocketBase::GetReceivedTransportParametersFlag () const
{
  return m_receivedTransportParameters;
}

bool
QuicSocketBase::CheckIfPacketOverflowMaxDataLimit (
  std::vector<std::pair<Ptr<Packet>, QuicSubheader> > disgregated)
{
  NS_LOG_FUNCTION (this);
  uint32_t validPacketSize = 0;

  for (auto frame_recv_it = disgregated.begin ();
       frame_recv_it != disgregated.end () and !disgregated.empty ();
       ++frame_recv_it)
    {
      QuicSubheader sub = (*frame_recv_it).second;
      // (*frame_recv_it)->PeekHeader (sub);

      if (sub.IsStream () and sub.GetStreamId () != 0)
        {
          validPacketSize += (*frame_recv_it).first->GetSize ();
        }
    }

  if ((m_max_data < m_rxBuffer->Size () + validPacketSize))
    {
      return true;
    }
  return false;
}

uint32_t
QuicSocketBase::GetMaxStreamId () const
{
  return std::max (m_initial_max_stream_id_bidi, m_initial_max_stream_id_uni);
}

uint32_t
QuicSocketBase::GetMaxStreamIdBidirectional () const
{
  return m_initial_max_stream_id_bidi;
}

uint32_t
QuicSocketBase::GetMaxStreamIdUnidirectional () const
{
  return m_initial_max_stream_id_uni;
}

bool
QuicSocketBase::CouldContainTransportParameters () const
{
  return m_couldContainTransportParameters;
}

void
QuicSocketBase::SetCongestionControlAlgorithm (Ptr<TcpCongestionOps> algo)
{
  NS_LOG_FUNCTION (this << algo);
  if (DynamicCast<QuicCongestionOps> (algo) != 0)
    {
      NS_LOG_INFO ("Non-legacy congestion control");
      m_quicCongestionControlLegacy = false;
    }
  else
    {
      NS_LOG_INFO (
        "Legacy congestion control, using only TCP standard functions");
      m_quicCongestionControlLegacy = true;
    }
  m_congestionControl = algo;
}

void
QuicSocketBase::SetSocketSndBufSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_socketTxBufferSize = size;
  m_txBuffer->SetMaxBufferSize (size);
}

uint32_t
QuicSocketBase::GetSocketSndBufSize (void) const
{
  return m_txBuffer->GetMaxBufferSize ();
}

void
QuicSocketBase::SetSocketRcvBufSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_socketRxBufferSize = size;
  m_rxBuffer->SetMaxBufferSize (size);
}

uint32_t
QuicSocketBase::GetSocketRcvBufSize (void) const
{
  return m_rxBuffer->GetMaxBufferSize ();
}

void
QuicSocketBase::UpdateCwnd (uint32_t oldValue, uint32_t newValue)
{
  m_cWndTrace (oldValue, newValue);
}

void
QuicSocketBase::UpdateSsThresh (uint32_t oldValue, uint32_t newValue)
{
  m_ssThTrace (oldValue, newValue);
}

void
QuicSocketBase::UpdateCongState (TcpSocketState::TcpCongState_t oldValue,
                                 TcpSocketState::TcpCongState_t newValue)
{
  m_congStateTrace (oldValue, newValue);
}

void
QuicSocketBase::UpdateNextTxSequence (SequenceNumber32 oldValue,
                                      SequenceNumber32 newValue)

{
  m_nextTxSequenceTrace (oldValue.GetValue (), newValue.GetValue ());
}

void
QuicSocketBase::UpdateHighTxMark (SequenceNumber32 oldValue, SequenceNumber32 newValue)
{
  m_highTxMarkTrace (oldValue.GetValue (), newValue.GetValue ());
}

void
QuicSocketBase::SetInitialSSThresh (uint32_t threshold)
{
  NS_ABORT_MSG_UNLESS ( (m_socketState == IDLE) || threshold == m_tcb->m_initialSsThresh,
                        "QuicSocketBase::SetSSThresh() cannot change initial ssThresh after connection started.");

  m_tcb->m_initialSsThresh = threshold;
}

uint32_t
QuicSocketBase::GetInitialSSThresh (void) const
{
  return m_tcb->m_initialSsThresh;
}

void
QuicSocketBase::SetInitialPacketSize (uint32_t size)
{
  NS_ABORT_MSG_IF (size < 1200, "The size of the initial packet should be at least 1200 bytes");
  m_initialPacketSize = size;
}

uint32_t
QuicSocketBase::GetInitialPacketSize () const
{
  return m_initialPacketSize;
}

void QuicSocketBase::SetLatency (uint32_t streamId, Time latency)
{
  m_txBuffer->SetLatency (streamId, latency);
}

Time QuicSocketBase::GetLatency (uint32_t streamId)
{
  return m_txBuffer->GetLatency (streamId);
}

void QuicSocketBase::SetDefaultLatency (Time latency)
{
  m_txBuffer->SetDefaultLatency (latency);
}

Time QuicSocketBase::GetDefaultLatency ()
{
  return m_txBuffer->GetDefaultLatency ();
}

void
QuicSocketBase::NotifyPacingPerformed (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Pacing timer expired, try sending a packet");
  SendPendingData (m_connected);
}

} // namespace ns3
