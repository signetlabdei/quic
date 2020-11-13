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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/object-vector.h"

#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
// #include "ns3/ipv4-route.h"
// #include "ns3/ipv6-route.h"

#include "quic-l5-protocol.h"
// #include "ns3/ipv4-end-point-demux.h"
// #include "ns3/ipv6-end-point-demux.h"
// #include "ns3/ipv4-end-point.h"
// #include "ns3/ipv6-end-point.h"
// #include "ns3/ipv4-l3-protocol.h"
// #include "ns3/ipv6-l3-protocol.h"
// #include "ns3/ipv6-routing-protocol.h"
#include "quic-socket-factory.h"
#include "quic-socket-base.h"
#include "quic-stream-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicL5Protocol");

NS_OBJECT_ENSURE_REGISTERED (QuicL5Protocol);

// #undef NS_LOG_APPEND_CONTEXT
// #define NS_LOG_APPEND_CONTEXT
// if (m_node and m_connectionId) { std::clog << " [node " << m_node->GetId () << " socket " << m_connectionId << "] "; }

TypeId
QuicL5Protocol::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::QuicL5Protocol")
      .SetParent<QuicSocketBase> ()
      .SetGroupName ("Internet")
      .AddConstructor<QuicL5Protocol> ()
      .AddAttribute ("StreamList", "The list of streams associated to this protocol.",
                     ObjectVectorValue (),
                     MakeObjectVectorAccessor (&QuicL5Protocol::m_streams),
                     MakeObjectVectorChecker<QuicStreamBase> ())
  ;
  return tid;
}

QuicL5Protocol::QuicL5Protocol ()
  : m_socket (0),
  m_node (0),
  m_connectionId ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("Made a QuicL5Protocol " << this);
  m_socket = 0;
  m_node = 0;
  m_connectionId = 0;
}

QuicL5Protocol::~QuicL5Protocol ()
{
  NS_LOG_FUNCTION (this);
}

void
QuicL5Protocol::CreateStream (
  const QuicStreamBase::QuicStreamDirectionTypes_t streamDirectionType)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Create the stream with ID " << m_streams.size ());
  Ptr<QuicStreamBase> stream = CreateObject<QuicStreamBase> ();

  stream->SetQuicL5 (this);

  stream->SetNode (m_node);

  stream->SetConnectionId (m_connectionId);

  stream->SetStreamId ((uint64_t) m_streams.size ());

  uint64_t mask = 0x00000003;
  if ((m_streams.size () & mask) == QuicStream::CLIENT_INITIATED_BIDIRECTIONAL
      or (m_streams.size () & mask)
      == QuicStream::SERVER_INITIATED_BIDIRECTIONAL)
    {
      stream->SetStreamDirectionType (QuicStream::BIDIRECTIONAL);

    }
  else
    {
      stream->SetStreamDirectionType (streamDirectionType);
    }

  if (stream->GetStreamId () > 0)
    {
      stream->SetMaxStreamData (m_socket->GetInitialMaxStreamData ());
    }
  else
    {
      stream->SetMaxStreamData (UINT32_MAX);
    }

  m_streams.push_back (stream);

}

void
QuicL5Protocol::CreateStream (
  const QuicStream::QuicStreamDirectionTypes_t streamDirectionType,
  uint64_t streamNum)
{

  NS_LOG_FUNCTION (this << m_streams.size () << streamNum);


  if (streamNum > m_socket->GetMaxStreamId ())   // TODO separate unidirectional and bidirectional
    {
      NS_LOG_INFO ("MaxStreamId " << m_socket->GetMaxStreamId ());
      SignalAbortConnection (
        QuicSubheader::TransportErrorCodes_t::STREAM_ID_ERROR,
        "Initiating Stream with higher StreamID with respect to what already negotiated");
      return;
    }

  // create streamNum streams
  while (m_streams.size () <= streamNum)
    {
      NS_LOG_INFO ("Create stream " << m_streams.size ());
      CreateStream (streamDirectionType);
    }

}

void
QuicL5Protocol::SetSocket (Ptr<QuicSocketBase> sock)
{
  NS_LOG_FUNCTION (this);
  m_socket = sock;
}

int
QuicL5Protocol::DispatchSend (Ptr<Packet> data)
{
  NS_LOG_FUNCTION (this);

  int sentData = 0;

  // if the streams are not created yet, open the streams
  if (m_streams.size () != m_socket->GetMaxStreamId ())
    {
      NS_LOG_INFO ("Create the missing streams");
      CreateStream (QuicStream::SENDER, m_socket->GetMaxStreamId ());   // TODO open up to max_stream_uni and max_stream_bidi
    }

  std::vector<Ptr<Packet> > disgregated = DisgregateSend (data);

  std::vector<Ptr<QuicStreamBase> >::iterator jt = m_streams.begin () + 1;   // Avoid Send on stream <0>, which is used only for handshake

  for (std::vector<Ptr<Packet> >::iterator it = disgregated.begin ();
       it != disgregated.end (); ++jt)
    {
      if (jt == m_streams.end ())             // Sending Remaining Load
        {
          jt = m_streams.begin () + 1;
        }
      NS_LOG_LOGIC (
        this << " " << (uint64_t)(*jt)->GetStreamDirectionType () << (uint64_t) QuicStream::SENDER << (uint64_t) QuicStream::BIDIRECTIONAL);

      if ((*jt)->GetStreamDirectionType () == QuicStream::SENDER
          or (*jt)->GetStreamDirectionType () == QuicStream::BIDIRECTIONAL)
        {
          NS_LOG_INFO (
            "Sending data on stream " << (*jt)->GetStreamId ());
          sentData = (*jt)->Send ((*it));
          ++it;
        }
    }

  return sentData;
}

int
QuicL5Protocol::DispatchSend (Ptr<Packet> data, uint64_t streamId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Send packet on (specified) stream " << streamId);

  Ptr<QuicStreamBase> stream = SearchStream (streamId);

  if (stream == nullptr)
    {
      CreateStream (QuicStream::SENDER, streamId);
    }

  stream = SearchStream (streamId);
  int sentData = 0;

  if (stream->GetStreamDirectionType () == QuicStream::SENDER
      or stream->GetStreamDirectionType () == QuicStream::BIDIRECTIONAL)
    {
      sentData = stream->Send (data);
    }

  return sentData;
}

int
QuicL5Protocol::DispatchRecv (Ptr<Packet> data, Address &address)
{
  NS_LOG_FUNCTION (this);
  auto disgregated = DisgregateRecv (data);

  if (m_socket->CheckIfPacketOverflowMaxDataLimit (disgregated))
    {
      NS_LOG_WARN ("Maximum data limit overflow");
      // put here this check instead of in QuicSocketBase due to framework mismatch in packet->Copy()
      SignalAbortConnection (
        QuicSubheader::TransportErrorCodes_t::FLOW_CONTROL_ERROR,
        "Received more data w.r.t. Max Data limit");
      return -1;
    }

  bool onlyAckFrames = true;
  uint64_t currStreamNum = m_streams.size () - 1;
  for (auto &elem : disgregated)
    {
      QuicSubheader sub = elem.second;

      // check if this is an ack frame
      if (!sub.IsAck ())
        {
          onlyAckFrames = false;
        }

      if (sub.GetStreamId () > currStreamNum)
        {
          currStreamNum = sub.GetStreamId ();
        }
    }

  CreateStream (QuicStream::RECEIVER, currStreamNum);

  for (auto it = disgregated.begin (); it != disgregated.end (); ++it)
    {
      QuicSubheader sub = (*it).second;

      if (sub.IsRstStream () or sub.IsMaxStreamData ()
          or sub.IsStreamBlocked () or sub.IsStopSending ()
          or sub.IsStream ())
        {
          Ptr<QuicStreamBase> stream = SearchStream (sub.GetStreamId ());

          if (stream != nullptr
              and (stream->GetStreamDirectionType () == QuicStream::RECEIVER
                   or stream->GetStreamDirectionType ()
                   == QuicStream::BIDIRECTIONAL))
            {
              NS_LOG_INFO (
                "Receiving frame on stream " << stream->GetStreamId () <<
                  " trigger stream");
              stream->Recv ((*it).first, sub, address);
            }
        }
      else
        {
          NS_LOG_INFO (
            "Receiving frame on stream " << sub.GetStreamId () <<
              " trigger socket");
          m_socket->OnReceivedFrame (sub);
        }
    }

  // trigger ACK TX if the received packet was not ACK-only
  return !onlyAckFrames;
}

int
QuicL5Protocol::Send (Ptr<Packet> frame)
{
  NS_LOG_FUNCTION (this);

  return m_socket->AppendingTx (frame);
}

int
QuicL5Protocol::Recv (Ptr<Packet> frame, Address &address)
{
  NS_LOG_FUNCTION (this);

  m_socket->AppendingRx (frame, address);

  return frame->GetSize ();
}

std::vector<Ptr<Packet> >
QuicL5Protocol::DisgregateSend (Ptr<Packet> data)
{
  NS_LOG_FUNCTION (this);

  uint32_t dataSizeByte = data->GetSize ();
  std::vector< Ptr<Packet> > disgregated;
  //data->Print(std::cout);

  // Equally distribute load on all streams except on stream 0
  uint32_t loadPerStream = dataSizeByte / (m_streams.size () - 1);
  uint32_t remainingLoad = dataSizeByte - loadPerStream * (m_streams.size () - 1);
  if (loadPerStream < 1)
    {
      loadPerStream = 1;
    }

  for (uint32_t start = 0; start < dataSizeByte; start += loadPerStream)
    {
      if (remainingLoad > 0 && start + remainingLoad == dataSizeByte)
        {
          Ptr<Packet> remainingfragment = data->CreateFragment (
            start, remainingLoad);
          disgregated.push_back (remainingfragment);
        }
      else
        {
          Ptr<Packet> fragment = data->CreateFragment (start, loadPerStream);
          disgregated.push_back (fragment);
        }

    }

  return disgregated;
}

std::vector< std::pair<Ptr<Packet>, QuicSubheader> >
QuicL5Protocol::DisgregateRecv (Ptr<Packet> data)
{
  NS_LOG_FUNCTION (this);

  uint32_t dataSizeByte = data->GetSize ();
  std::vector< std::pair<Ptr<Packet>, QuicSubheader> > disgregated;
  NS_LOG_INFO ("DisgregateRecv for a packet with size " << dataSizeByte);
  //data->Print(std::cout);

  // the packet could contain multiple frames
  // each of them starts with a subheader
  // cycle through the data packet and extract the frames
  for (uint32_t start = 0; start < dataSizeByte; )
    {
      QuicSubheader sub;
      data->RemoveHeader (sub);
      NS_LOG_INFO ("subheader " << sub << " dataSizeByte " << dataSizeByte
                                << " remaining " << data->GetSize () << " frame size " << sub.GetLength ());
      Ptr<Packet> remainingfragment = data->CreateFragment (0, sub.GetLength ());
      NS_LOG_INFO ("fragment size " << remainingfragment->GetSize ());

      // remove the first portion of the packet
      data->RemoveAtStart (sub.GetLength ());
      start += sub.GetSerializedSize () + sub.GetLength ();
      disgregated.push_back (std::make_pair (remainingfragment, sub));
    }


  return disgregated;
}

Ptr<QuicStreamBase>
QuicL5Protocol::SearchStream (uint64_t streamId)
{
  NS_LOG_FUNCTION (this);
  std::vector<Ptr<QuicStreamBase> >::iterator it = m_streams.begin ();
  Ptr<QuicStreamBase> stream;
  while (it != m_streams.end ())
    {
      if ((*it)->GetStreamId () == streamId)
        {
          stream = *it;
          break;
        }
      ++it;
    }
  return stream;
}

void
QuicL5Protocol::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

void
QuicL5Protocol::SetConnectionId (uint64_t connId)
{
  NS_LOG_FUNCTION (this << connId);
  m_connectionId = connId;
}

uint16_t
QuicL5Protocol::GetMaxPacketSize () const
{
  return m_socket->GetSegSize ();
}

bool
QuicL5Protocol::ContainsTransportParameters ()
{
  return m_socket->CouldContainTransportParameters ();
}

void
QuicL5Protocol::OnReceivedTransportParameters (
  QuicTransportParameters transportParameters)
{
  m_socket->OnReceivedTransportParameters (transportParameters);
}

void
QuicL5Protocol::SignalAbortConnection (uint16_t transportErrorCode,
                                       const char* reasonPhrase)
{
  NS_LOG_FUNCTION (this);
  m_socket->AbortConnection (transportErrorCode, reasonPhrase);
}

void
QuicL5Protocol::UpdateInitialMaxStreamData (uint32_t newMaxStreamData)
{
  NS_LOG_FUNCTION (this << newMaxStreamData);

  // TODO handle in a different way bidirectional and unidirectional streams
  for (auto stream : m_streams)
    {
      if (stream->GetStreamId () > 0) // stream 0 is set to UINT32_MAX and not modified
        {
          stream->SetMaxStreamData (newMaxStreamData);
        }
    }
}

uint64_t
QuicL5Protocol::GetMaxData ()
{
  NS_LOG_FUNCTION (this);

  uint64_t maxData = 0;
  for (auto stream : m_streams)
    {
      maxData += stream->SendMaxStreamData ();
    }
  return maxData;
}

} // namespace ns3

