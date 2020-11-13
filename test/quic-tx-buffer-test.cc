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

#include "ns3/test.h"
#include "ns3/quic-socket-tx-buffer.h"
#include "ns3/quic-stream-tx-buffer.h"
#include "ns3/quic-socket-tx-scheduler.h"

#include "ns3/quic-socket-base.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("QuicTxBufferTestSuite");

/**
 * \ingroup internet-tests
 * \ingroup tests
 *
 * \brief The QuicTxBuffer Test
 */
class QuicTxBufferTestCase : public TestCase
{
public:
  /** \brief Constructor */
  QuicTxBufferTestCase ();

private:
  virtual void
  DoRun (void);
  virtual void
  DoTeardown (void);

  /** \brief Test the generation of an unsent block */
  void
  TestNewBlock ();
  /** \brief Test the block-based acknowledgment mechanism */
  void
  TestPartialAck ();
  /** \brief Test the acknowledgment mechanism with losses */
  void
  TestAckLoss ();
  /** \brief Test the direct loss setting mechanism */
  void
  TestSetLoss ();
  /** \brief Test the edge cases for adding packets */
  void
  TestAddBlocks ();
  /** \brief Test the handling of Stream 0 packets */
  void
  TestStream0 ();
  /** \brief Test the insertion of packets in the Stream TX buffer */
  void
  TestStreamAdd ();
  /** \brief Test the extraction of packets from the Stream TX buffer */
  void
  TestStreamExtract ();
  /** \brief Test the Socket TX buffer rejection due to available space limitations */
  void
  TestRejection ();
  /** \brief Test the Socket TX buffer retransmission of lost packets */
  void
  TestRetransmission ();
};

QuicTxBufferTestCase::QuicTxBufferTestCase () :
    TestCase ("QuicTxBuffer Test")
{
}

void
QuicTxBufferTestCase::DoRun ()
{
  /*
   * Cases for new block:
   * -> add a small block (equal to minimum MSS)
   * -> add a L5 block larger than the packet
   * -> send a packet with two L5 blocks
   * -> acknowledge everything
   */
   Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestNewBlock,
                        this);

  /*
   * Partial ACK:
   * -> add 6 small blocks
   * -> acknowledge each one except the fifth
   * -> check correctness of congestion window and lost packets
   */
   Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestPartialAck,
                        this);

  /*
   * Partial ACK with loss:
   * -> add 6 small blocks
   * -> acknowledge each one except the second
   * -> check correctness of loss indication
   * -> check correctness of congestion window and lost packets
   */
  Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestAckLoss, this);

  /*
   * Mark a packet as lost:
   * -> add 6 small blocks
   * -> mark packet 4 as lost
   * -> reset sent list to mark packets 1 and 2 as lost
   * -> check correctness
   * 
   */
   Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestSetLoss, this);

  /*
   * Edge cases of the block adding function:
   * -> add 6 small blocks
   * -> mark packet 4 as lost
   * -> reset sent list to mark packets 1 and 2 as lost
   * -> check correctness
   * 
   */
   Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestAddBlocks,
                        this);
  /*
   * Edge cases of the block adding function:
   * -> add 1 block
   * -> add a Stream 0 packet
   * -> acknowledge everything
   * -> check correctness of bytes in flight count
   * 
   */
   Simulator::Schedule (Seconds (0.0), &QuicTxBufferTestCase::TestStream0, this);
   Simulator::Run ();
   Simulator::Destroy ();

  /*
   * Test the insertion of packets in the Stream TX buffer:
   * -> add packets till stream tx buffer overflow
   * -> check correctness of buffer application size and available size
   */
  TestStreamAdd ();

  /*
   * Test the extraction of packets from the Stream TX buffer:
   * -> add 3 packets 
   * -> extract first 2 packets and readd them again
   * -> extract all packets and test that the buffer is empty
   * -> check correctness of buffer application size and available size
   */
  TestStreamExtract ();

  /*
   * Test the Socket TX buffer rejection due to available space limitations:
   * -> add 5 packets to the stream tx buffer
   * -> extract first 2 packets and send them as frames to socket tx buffer
   * -> extract 2 packets more and send them as frames to socket tx buffer
   * -> reject the packets due to socket tx buffer available space limitations
   * -> enqueue back the rejected packets to stream tx buffer
   * -> check correctness of the two buffers application size and available size
   */
  TestRejection ();

  /*
   * Test the Socket TX buffer retransmission of lost packets:
   * -> send 1 packet from socket tx buffer and ack the packet sent
   * -> send other 2 packets from socket tx buffer but mark them as lost on ack
   * -> retransmit the first of the two packets
   * -> ack the previous packet but not the retransmitted one
   * -> ack also the retransmitted packet
   * -> check correctness of buffer application size and available size
   * -> check correctness of bytes in flight count
   * -> check correctness of acked and lost packets list
   */
  TestRetransmission ();
}

void
QuicTxBufferTestCase::TestRetransmission ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 0, "TxBuf miscalculates initial size of in flight segments");

  // send a packet from socket tx buffer
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), 
                                          false, true, false);
  p1->AddHeader (sub);
  txBuf.Add (p1);

  Ptr<Packet> ptx = txBuf.NextSequence (1200, SequenceNumber32 (1));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200, "TxBuf miscalculates size of in flight segments");

  // ack the packet sent
  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;
  uint32_t largestAcknowledged = 1;
  additionalAckBlocks.push_back (0);
  gaps.push_back (0);

  std::vector<Ptr<QuicSocketTxItem>> acked = txBuf.OnAckUpdate (tcbd,
                                                            largestAcknowledged,
                                                            additionalAckBlocks,
                                                            gaps);
  NS_TEST_ASSERT_MSG_EQ(acked.size (), 1, "Wrong acked packet vector size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packet->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packetNumber, SequenceNumber32 (1),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 0, "TxBuf miscalculates size of in flight segments");

  // send other two packets from socket tx buffer but mark them as lost on ack
  Ptr<Packet> p2 = Create<Packet> (1196);
  sub = QuicSubheader::CreateStreamSubHeader (1, 1200, p2->GetSize (), 
                                          false, true, false);
  p2->AddHeader (sub);
  txBuf.Add (p2);

  ptx = txBuf.NextSequence (1200, SequenceNumber32 (2));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200, "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> p3 = Create<Packet> (1196);
  sub = QuicSubheader::CreateStreamSubHeader (1, 2400, p3->GetSize (), 
                                          false, true, false);
  p3->AddHeader (sub);
  txBuf.Add (p3);

  ptx = txBuf.NextSequence (1200, SequenceNumber32 (3));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 2400, "TxBuf miscalculates size of in flight segments");

  acked = txBuf.OnAckUpdate (tcbd,
                             largestAcknowledged,
                             additionalAckBlocks,
                             gaps);
  NS_TEST_ASSERT_MSG_EQ(acked.size (), 0, "Wrong acked packet vector size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 2400, "TxBuf miscalculates size of in flight segments");

  // retransmit the first of the two packets
  uint32_t newPackets = 1;
  txBuf.ResetSentList (newPackets);
  std::vector<Ptr<QuicSocketTxItem>> lostPackets = txBuf.DetectLostPackets ();
  NS_TEST_ASSERT_MSG_EQ(lostPackets.size (), 1, "Wrong lost packet vector size");
  NS_TEST_ASSERT_MSG_EQ(lostPackets.at (0)->m_packet->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(lostPackets.at (0)->m_packetNumber, SequenceNumber32 (2),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 2400, "TxBuf miscalculates size of in flight segments");

  uint32_t toRetx = txBuf.Retransmission (SequenceNumber32(2));
  NS_TEST_ASSERT_MSG_EQ(toRetx, 1200, "wrong number of lost bytes");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200, "TxBuf miscalculates size of in flight segments");

  ptx = txBuf.NextSequence (toRetx, SequenceNumber32 (4));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 2400, "TxBuf miscalculates size of in flight segments");

  // ack the previous packet but not the retransmitted one
  largestAcknowledged = 3;
  acked = txBuf.OnAckUpdate (tcbd,
                             largestAcknowledged,
                             additionalAckBlocks,
                             gaps);
  NS_TEST_ASSERT_MSG_EQ(acked.size (), 1, "Wrong acked packet vector size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packet->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packetNumber, SequenceNumber32 (3),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200, "TxBuf miscalculates size of in flight segments");

  // ack also the retransmitted packet
  largestAcknowledged = 4;
  acked = txBuf.OnAckUpdate (tcbd,
                             largestAcknowledged,
                             additionalAckBlocks,
                             gaps);
  NS_TEST_ASSERT_MSG_EQ(acked.size (), 1, "Wrong acked packet vector size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packet->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packetNumber, SequenceNumber32 (4),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 0, "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::TestRejection ()
{
  // create the two buffers
  QuicStreamTxBuffer streamTxBuf;
  streamTxBuf.SetMaxBufferSize (18000);

  QuicSocketTxBuffer socketTxBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  socketTxBuf.SetScheduler(sched);
  socketTxBuf.SetMaxBufferSize (4800);

  // Create packet
  Ptr<Packet> p = Create<Packet> (1200);
  
  // Add 5 packets
  bool pos = streamTxBuf.Add(p);
  pos = streamTxBuf.Add(p);
  pos = streamTxBuf.Add(p);
  pos = streamTxBuf.Add(p);
  pos = streamTxBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.Available (), 12000, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.AppSize (), 6000, "Wrong buffer size");

  // Extract first two packets
  Ptr<Packet> outPkt = streamTxBuf.NextSequence(2400, SequenceNumber32(0));

  NS_TEST_ASSERT_MSG_NE(outPkt, 0, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 2400,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.Available (), 14400, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.AppSize (), 3600, "Wrong buffer size");

  // Send the extracted packets as frames to socket tx buffer
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, outPkt->GetSize (), 
                         false, true, false);
  outPkt->AddHeader (sub);
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 2400 + sub.GetSerializedSize (),  "Wrong packet size");

  pos = socketTxBuf.Add (outPkt);
  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(socketTxBuf.Available (), 4800 - outPkt->GetSize (), "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(socketTxBuf.AppSize (), outPkt->GetSize (), "Wrong buffer size");

  // Extract two packets more
  Ptr<Packet> outPktMore = streamTxBuf.NextSequence(2400, SequenceNumber32(0));

  NS_TEST_ASSERT_MSG_NE(outPktMore, 0, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(outPktMore->GetSize(), 2400,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.AppSize (), 1200, "Wrong buffer size");

  // Add the extracted packets as frames to socket tx buffer but this time is full
  sub = QuicSubheader::CreateStreamSubHeader (1, 2400, outPktMore->GetSize (), 
                         true, true, false);
  outPktMore->AddHeader (sub);
  NS_TEST_ASSERT_MSG_EQ(outPktMore->GetSize(), 2400 + sub.GetSerializedSize (),  "Wrong packet size");

  pos = socketTxBuf.Add (outPktMore);
  NS_TEST_ASSERT_MSG_EQ(pos, false, "Buffer overflow");
  NS_TEST_ASSERT_MSG_EQ(socketTxBuf.Available (), 4800 - outPkt->GetSize (), "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(socketTxBuf.AppSize (), outPkt->GetSize (), "Wrong buffer size");

  // Enqueue back the rejected packet to stream tx buffer
  pos = streamTxBuf.Rejected(outPktMore);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.Available (), 14400 - sub.GetSerializedSize (), "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(streamTxBuf.AppSize (), 3600 + sub.GetSerializedSize (), "Wrong buffer size");
}

void
QuicTxBufferTestCase::TestStreamAdd ()
{
  // create the buffer
  QuicStreamTxBuffer txBuf;
  txBuf.SetMaxBufferSize (18000);

  // Create packet
  Ptr<Packet> p = Create<Packet> (1200);
  
  // Add a packet
  bool pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 1200, "Wrong buffer size");

  // Add a second packet
  pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 15600, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 2400, "Wrong buffer size");

  // Add a third packet
  pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 14400, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 3600, "Wrong buffer size");

  // Test full buffer
  p = Create<Packet> (14400);
  pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 0, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 18000, "Wrong buffer size");

  // Test buffer overflow
  pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, false, "Buffer overflow");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 0, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 18000, "Wrong buffer size");
}

void
QuicTxBufferTestCase::TestStreamExtract ()
{
  // create the buffer
  QuicStreamTxBuffer txBuf;
  txBuf.SetMaxBufferSize (18000);

  // Create packet
  Ptr<Packet> p = Create<Packet> (1200);
  
  // Add a packet
  bool pos = txBuf.Add(p);

  // Add a second packet
  pos = txBuf.Add(p);

  // Add a third packet
  pos = txBuf.Add(p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 14400, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 3600, "Wrong buffer size");

  //Extract first two packets
  Ptr<Packet> outPkt = txBuf.NextSequence(2400, SequenceNumber32(0));

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 2400,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 1200, "Wrong buffer size");

  //Insert missing packet
  pos = txBuf.Add(outPkt);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 14400, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 3600, "Wrong buffer size");

  //Extract all packets
  outPkt = txBuf.NextSequence(3600, SequenceNumber32(1));

  NS_TEST_ASSERT_MSG_NE(outPkt, 0, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 3600,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 18000, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 0, "Wrong buffer size");

  // Test empty buffer
  outPkt = txBuf.NextSequence(1200, SequenceNumber32(2));
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 0,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.Available (), 18000, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.AppSize (), 0, "Wrong buffer size");
}

void
QuicTxBufferTestCase::TestNewBlock ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  NS_TEST_ASSERT_MSG_EQ(
      txBuf.BytesInFlight (), 0,
      "TxBuf miscalculates initial size of in flight segments");

  // get a packet which is exactly the same stored
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  p1->AddHeader (sub);
  txBuf.Add (p1);
  NS_TEST_ASSERT_MSG_EQ(p1->GetSize (), 1200, "Wrong header size");

  
  Ptr<Packet> ptx = txBuf.NextSequence (1200, SequenceNumber32 (1));
  
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200, "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");
  
  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;
  uint32_t largestAcknowledged = 1;
  additionalAckBlocks.push_back (1);
  
  std::vector<Ptr<QuicSocketTxItem>> acked = txBuf.OnAckUpdate (tcbd,
                                                                largestAcknowledged,
                                                                additionalAckBlocks,
                                                                gaps);
  NS_TEST_ASSERT_MSG_EQ(acked.size (), 1, "Wrong acked packet vector size");
  NS_TEST_ASSERT_MSG_EQ(acked.at (0)->m_packet->GetSize (), 1200,
                        "TxBuf miscalculates size");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 0,
                        "TxBuf miscalculates size of in flight segments");
  
  // starts over the boundary, but ends earlier
  
  Ptr<Packet> p2 = Create<Packet> (2996);
  sub = QuicSubheader::CreateStreamSubHeader (1, 0, p2->GetSize (), false,
                                              true, false);
  p2->AddHeader (sub);
  txBuf.Add (p2);
  
  ptx = txBuf.NextSequence (1200, SequenceNumber32 (2));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1200,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");
  
  ptx = txBuf.NextSequence (3000, SequenceNumber32 (3));
  // Expecting 3000 (added, including QuicSubheader 4) - 1200 (extracted, including QuicSubheader 4)
  // + 6 (QuicSubheader of the new packet, with both the length and the offset)
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 1806, 
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 3006,
                        "TxBuf miscalculates size of in flight segments");
  
  // starts over the boundary, but ends after
  Ptr<Packet> p3 = Create<Packet> (1196);
  sub = QuicSubheader::CreateStreamSubHeader (1, 0, p3->GetSize (), false,
                                              true, false);
  p3->AddHeader (sub);
  txBuf.Add (p3);
  Ptr<Packet> p4 = Create<Packet> (1196);
  sub = QuicSubheader::CreateStreamSubHeader (1, 0, p4->GetSize (), false,
                                              true, false);
  p4->AddHeader (sub);
  txBuf.Add (p4);
  ptx = txBuf.NextSequence (2400, SequenceNumber32 (4));
  NS_TEST_ASSERT_MSG_EQ(ptx->GetSize (), 2400,
                        "Returned packet has different size than requested");
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 5406,
                        "TxBuf miscalculates size of in flight segments");
  
  additionalAckBlocks.pop_back ();
  largestAcknowledged = 4;
  // Clear everything
  acked = txBuf.OnAckUpdate (tcbd, largestAcknowledged, additionalAckBlocks,
                             gaps);
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 0,
                        "TxBuf miscalculates size of in flight segments");
  
}

void
QuicTxBufferTestCase::TestPartialAck ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  // get a packet which is exactly the same stored
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  p1->AddHeader (sub);

  // create 5 identical packets
  Ptr<Packet> p2 = Copy (p1);
  Ptr<Packet> p3 = Copy (p1);
  Ptr<Packet> p4 = Copy (p1);
  Ptr<Packet> p5 = Copy (p1);
  Ptr<Packet> p6 = Copy (p1);

  // add the 6 packets to the send buffer
  txBuf.Add (p1);
  txBuf.Add (p2);
  txBuf.Add (p3);
  txBuf.Add (p4);
  txBuf.Add (p5);
  txBuf.Add (p6);

  // send the packets with successive sequence numbers
  Ptr<Packet> ptx1 = txBuf.NextSequence (1200, SequenceNumber32 (1));
  Ptr<Packet> ptx2 = txBuf.NextSequence (1200, SequenceNumber32 (2));
  Ptr<Packet> ptx3 = txBuf.NextSequence (1200, SequenceNumber32 (3));
  Ptr<Packet> ptx4 = txBuf.NextSequence (1200, SequenceNumber32 (4));
  Ptr<Packet> ptx5 = txBuf.NextSequence (1200, SequenceNumber32 (5));
  Ptr<Packet> ptx6 = txBuf.NextSequence (1200, SequenceNumber32 (6));

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 7200,
                        "TxBuf miscalculates size of in flight segments");

  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;
  uint32_t largestAcknowledged = 6;
  additionalAckBlocks.push_back (4);
  gaps.push_back (5);

  // acknowledge all packets except 5
  std::vector<Ptr<QuicSocketTxItem>> acked = txBuf.OnAckUpdate (tcbd,
                                                            largestAcknowledged,
                                                            additionalAckBlocks,
                                                            gaps);

  std::vector<Ptr<QuicSocketTxItem>> lost = txBuf.DetectLostPackets ();
  NS_TEST_ASSERT_MSG_EQ(lost.empty (), true,
                        "TxBuf detects a non-existent loss");
  //NS_TEST_ASSERT_MSG_EQ(
   //   acked.size (), 5,
     // "TxBuf does not correctly detect the number of ACKed packets");

  // test ACK correctness
  std::vector<uint32_t> pkts;
  pkts.push_back (6);
  pkts.push_back (4);
  pkts.push_back (3);
  pkts.push_back (2);
  pkts.push_back (1);
  uint32_t i = 0;
  for (auto acked_it = acked.begin (); acked_it != acked.end ();
      ++acked_it, i++)
    {
      NS_LOG_LOGIC("Hello");
      NS_TEST_ASSERT_MSG_EQ(
          (*acked_it)->m_packetNumber.GetValue (), pkts.at (i),
          "TxBuf does not correctly detect the IDs of ACKed packets");
    }

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::TestAckLoss ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  // get a packet which is exactly the same stored
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  p1->AddHeader (sub);

  // create 5 identical packets
  Ptr<Packet> p2 = Copy (p1);
  Ptr<Packet> p3 = Copy (p1);
  Ptr<Packet> p4 = Copy (p1);
  Ptr<Packet> p5 = Copy (p1);
  Ptr<Packet> p6 = Copy (p1);

  // add the 6 packets to the send buffer
  txBuf.Add (p1);
  txBuf.Add (p2);
  txBuf.Add (p3);
  txBuf.Add (p4);
  txBuf.Add (p5);
  txBuf.Add (p6);

  // send the packets with successive sequence numbers
  Ptr<Packet> ptx1 = txBuf.NextSequence (1200, SequenceNumber32 (1));
  Ptr<Packet> ptx2 = txBuf.NextSequence (1200, SequenceNumber32 (2));
  Ptr<Packet> ptx3 = txBuf.NextSequence (1200, SequenceNumber32 (3));
  Ptr<Packet> ptx4 = txBuf.NextSequence (1200, SequenceNumber32 (4));
  Ptr<Packet> ptx5 = txBuf.NextSequence (1200, SequenceNumber32 (5));
  Ptr<Packet> ptx6 = txBuf.NextSequence (1200, SequenceNumber32 (6));

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 7200,
                        "TxBuf miscalculates size of in flight segments");

  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;
  uint32_t largestAcknowledged = 6;
  gaps.push_back (2);
  additionalAckBlocks.push_back (1);

  // acknowledge all packets except 5
  std::vector<Ptr<QuicSocketTxItem>> acked = txBuf.OnAckUpdate (tcbd,
                                                            largestAcknowledged,
                                                            additionalAckBlocks,
                                                            gaps);

  std::vector<Ptr<QuicSocketTxItem>> lost = txBuf.DetectLostPackets ();
  NS_TEST_ASSERT_MSG_EQ(
      acked.size(), 5,
      "TxBuf does not correctly detect the number of ACKed packets");

  // test ACK correctness
  std::vector<uint32_t> pkts;
  pkts.push_back (6);
  pkts.push_back (5);
  pkts.push_back (4);
  pkts.push_back (3);
  pkts.push_back (1);
  uint32_t i = 0;
  for (auto acked_it = acked.begin (); acked_it != acked.end ();
      ++acked_it, i++)
    {

      NS_TEST_ASSERT_MSG_EQ(
          (*acked_it)->m_packetNumber.GetValue (), pkts.at (i),
          "TxBuf does not correctly detect the IDs of ACKed packets");
    }

  NS_TEST_ASSERT_MSG_EQ(lost.size (), 1, "TxBuf misses a loss");

  NS_TEST_ASSERT_MSG_EQ(
      lost.at (0)->m_packetNumber.GetValue (), 2,
      "TxBuf does not correctly detect the IDs of lost packets");

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::TestSetLoss ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  // get a packet which is exactly the same stored
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  p1->AddHeader (sub);

  // create 5 identical packets
  Ptr<Packet> p2 = Copy (p1);
  Ptr<Packet> p3 = Copy (p1);
  Ptr<Packet> p4 = Copy (p1);
  Ptr<Packet> p5 = Copy (p1);
  Ptr<Packet> p6 = Copy (p1);

  // add the 6 packets to the send buffer
  txBuf.Add (p1);
  txBuf.Add (p2);
  txBuf.Add (p3);
  txBuf.Add (p4);
  txBuf.Add (p5);
  txBuf.Add (p6);

  // send the packets with successive sequence numbers
  Ptr<Packet> ptx1 = txBuf.NextSequence (1200, SequenceNumber32 (1));
  Ptr<Packet> ptx2 = txBuf.NextSequence (1200, SequenceNumber32 (2));
  Ptr<Packet> ptx3 = txBuf.NextSequence (1200, SequenceNumber32 (3));
  Ptr<Packet> ptx4 = txBuf.NextSequence (1200, SequenceNumber32 (4));
  Ptr<Packet> ptx5 = txBuf.NextSequence (1200, SequenceNumber32 (5));
  Ptr<Packet> ptx6 = txBuf.NextSequence (1200, SequenceNumber32 (6));

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 7200,
                        "TxBuf miscalculates size of in flight segments");
  bool found = txBuf.MarkAsLost (SequenceNumber32 (4));

  NS_TEST_ASSERT_MSG_EQ(found, true, "TxBuf misses lost packet");

  // mark packet 4 as lost
  std::vector<Ptr<QuicSocketTxItem>> lost = txBuf.DetectLostPackets ();

  NS_TEST_ASSERT_MSG_EQ(lost.size (), 1,
                        "TxBuf cannot set the correct number of lost packets");
  NS_TEST_ASSERT_MSG_EQ(lost.at (0)->m_packetNumber, SequenceNumber32 (4),
                        "TxBuf gets the wrong lost packet ID");

  // mark packets 1 and 2 as lost (all except the last 4)
  txBuf.ResetSentList (4);

  lost = txBuf.DetectLostPackets ();

  NS_TEST_ASSERT_MSG_EQ(lost.size (), 3,
                        "TxBuf cannot set the correct number of lost packets");
  NS_TEST_ASSERT_MSG_EQ(lost.at (0)->m_packetNumber, SequenceNumber32 (1),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(lost.at (1)->m_packetNumber, SequenceNumber32 (2),
                        "TxBuf gets the wrong lost packet ID");
  NS_TEST_ASSERT_MSG_EQ(lost.at (2)->m_packetNumber, SequenceNumber32 (4),
                        "TxBuf gets the wrong lost packet ID");

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 7200,
                        "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::TestAddBlocks ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  // set buffer size
  txBuf.SetMaxBufferSize (6000);

  tcbd = CreateObject<QuicSocketState> ();

  // get a packet which is exactly the same stored
  Ptr<Packet> p1 = Create<Packet> (1196);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  p1->AddHeader (sub);

  // create 5 identical packets
  Ptr<Packet> p2 = Copy (p1);
  Ptr<Packet> p3 = Copy (p1);
  Ptr<Packet> p4 = Copy (p1);
  Ptr<Packet> p5 = Copy (p1);
  Ptr<Packet> p6 = Copy (p1);

  // add the 6 packets to the send buffer
  txBuf.Add (p1);
  txBuf.Add (p2);
  txBuf.Add (p3);
  txBuf.Add (p4);
  bool full = txBuf.Add (p5);
  bool extra = txBuf.Add (p6);

  NS_TEST_ASSERT_MSG_EQ(full, true, "TxBuf does not add a correct packet");
  NS_TEST_ASSERT_MSG_EQ(extra, false, "TxBuf adds a packet in overflow");

  // send the packets with successive sequence numbers
  Ptr<Packet> ptx1 = txBuf.NextSequence (1200, SequenceNumber32 (1));
  Ptr<Packet> ptx2 = txBuf.NextSequence (1200, SequenceNumber32 (2));
  Ptr<Packet> ptx3 = txBuf.NextSequence (1200, SequenceNumber32 (3));
  Ptr<Packet> ptx4 = txBuf.NextSequence (1200, SequenceNumber32 (4));
  Ptr<Packet> ptx5 = txBuf.NextSequence (1200, SequenceNumber32 (5));
  Ptr<Packet> ptx6 = txBuf.NextSequence (1200, SequenceNumber32 (6));

  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 6000,
                        "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::TestStream0 ()
{
  // create the buffer
  QuicSocketTxBuffer txBuf;
  Ptr<QuicSocketTxScheduler> sched = CreateObject<QuicSocketTxScheduler>();
  txBuf.SetScheduler(sched);
  Ptr<QuicSocketState> tcbd;

  tcbd = CreateObject<QuicSocketState> ();

  // get a packet
  Ptr<Packet> p1 = Create<Packet> (1196);
  Ptr<Packet> p2 = Copy (p1);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                   true, false);
  QuicSubheader sub0 = QuicSubheader::CreateStreamSubHeader (1, 0, p1->GetSize (), false,
                                                    true, false);
  p1->AddHeader (sub);
  p2->AddHeader (sub0);

  // create another identical packet
  Ptr<Packet> p3 = Copy (p1);

  // add the packets to the send buffer
  txBuf.Add (p1);
  txBuf.Add (p2);
  txBuf.Add (p3);

  // send the packets with successive sequence numbers
  Ptr<Packet> ptx1 = txBuf.NextSequence (1200, SequenceNumber32 (1));
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> ptx2 = txBuf.NextStream0Sequence (SequenceNumber32 (2));
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");

  Ptr<Packet> ptx3 = txBuf.NextSequence (1200, SequenceNumber32 (3));
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 2400,
                        "TxBuf miscalculates size of in flight segments");

  std::vector<uint32_t> additionalAckBlocks;
  std::vector<uint32_t> gaps;
  uint32_t largestAcknowledged = 1;
  additionalAckBlocks.push_back (1);

  // acknowledge all packets except 5
  std::vector<Ptr<QuicSocketTxItem>> acked = txBuf.OnAckUpdate (tcbd,
                                                            largestAcknowledged,
                                                            additionalAckBlocks,
                                                            gaps);
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");

  largestAcknowledged = 2;
  additionalAckBlocks.push_back (2);

  // acknowledge all packets except 5
  acked = txBuf.OnAckUpdate (tcbd,
                                                            largestAcknowledged,
                                                            additionalAckBlocks,
                                                            gaps);
  NS_TEST_ASSERT_MSG_EQ(txBuf.BytesInFlight (), 1200,
                        "TxBuf miscalculates size of in flight segments");
}

void
QuicTxBufferTestCase::DoTeardown ()
{
}

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief the TestSuite for the TcpTxBuffer test case
 */
class QuicTxBufferTestSuite : public TestSuite
{
public:
  QuicTxBufferTestSuite () :
      TestSuite ("quic-tx-buffer", UNIT)
  {
    LogComponentEnable ("QuicTxBufferTestSuite", LOG_LEVEL_ALL);
    LogComponentEnable ("QuicSocketTxBuffer", LOG_LEVEL_LOGIC);

    AddTestCase (new QuicTxBufferTestCase, TestCase::QUICK);
  }
};

static QuicTxBufferTestSuite g_quicTxBufferTestSuite; //!< Static variable for test initialization
