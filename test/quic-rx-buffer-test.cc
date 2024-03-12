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
#include "ns3/packet.h"
#include "ns3/log.h"

#include "ns3/quic-socket-rx-buffer.h"
#include "ns3/quic-stream-rx-buffer.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("QuicRxBufferTestSuite");

/**
 * \ingroup internet-tests
 * \ingroup tests
 *
 * \brief The QuicRxBuffer Test
 */
class QuicRxBufferTestCase : public TestCase
{
public:
  QuicRxBufferTestCase ();

private:
  virtual void
  DoRun (void);
  virtual void
  DoTeardown (void);

  /*
   * \brief Test the insertion of packets in the Socket RX buffer
   */
  void
  TestSocketAdd ();
  /*
   * \brief Test the extraction of packets from the Socket RX buffer
   */
  void
  TestSocketExtract ();
  /**
   * \brief Test the insertion of packets in the Stream RX buffer
   */
  void
  TestStreamAdd ();
  /**
   * \brief Test the extraction of packets from the Stream RX buffer
   */
  void
  TestStreamExtract ();
};

QuicRxBufferTestCase::QuicRxBufferTestCase () :
    TestCase ("QuicRxBuffer Test")
{
}

void
QuicRxBufferTestCase::DoRun ()
{
  /*
   * Test the insertion of packets in the Socket RX buffer:
   * -> add packets till socket tx buffer overflow
   * -> check correctness of buffer application size and available size
   * -> checking availability count
   */
  TestSocketAdd ();

  /*
   * Test the extraction of packets from the Socket RX buffer
   * -> add 3 packets
   * -> extract all packets and test that the buffer is empty
   * -> check correctness of buffer application size and available size
   * -> checking availability count
   */
  TestSocketExtract ();

  /*
   * Test the insertion of packets in the Stream RX buffer:
   * -> add packets till stream tx buffer overflow
   * -> check correctness of buffer application size and available size
   * -> checking availability count
   * -> check in-order buffer insertion and duplicate packets
   * -> check FIN functionality
   */
  TestStreamAdd ();

  /*
   * Test the extraction of packets from the Stream RX buffer:
   * -> add 3 packets
   * -> extract first 2 packets and readd them again
   * -> extract all packets and test that the buffer is empty
   * -> check correctness of buffer application size and available size
   */
  TestStreamExtract ();
}

void
QuicRxBufferTestCase::TestSocketAdd ()
{
  // create the buffer
  QuicSocketRxBuffer rxBuf;
  rxBuf.SetMaxBufferSize (3000);

  // add 1 packet to the socket rx buffer
  Ptr<Packet> p = Create<Packet> (1200);
  bool pos = rxBuf.Add (p);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 1800,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 1200,
                        "Buffer size differs from expected");

  // test full buffer
  Ptr<Packet> p1 = Copy (p);
  Ptr<Packet> p2 = Copy (p);

  pos = rxBuf.Add (p1);
  bool neg = rxBuf.Add (p2);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(neg, false, "Added wrong packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 600,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 2400,
                        "Buffer size differs from expected");
}

void
QuicRxBufferTestCase::TestSocketExtract ()
{
  // create the buffer
  QuicSocketRxBuffer rxBuf;
  rxBuf.SetMaxBufferSize (3600);

  // add 3 packets to the socket rx buffer
  Ptr<Packet> p = Create<Packet> (1200);
  Ptr<Packet> p1 = Copy (p);
  Ptr<Packet> p2 = Copy (p);

  rxBuf.Add (p);
  rxBuf.Add (p1);
  rxBuf.Add (p2);

  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 0,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 3600,
                        "Buffer size differs from expected");

  // extract first packet
  Ptr<Packet> out = rxBuf.Extract (1200);
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 1200,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 2400,
                        "Buffer size differs from expected");
  NS_TEST_ASSERT_MSG_EQ(out->GetSize (), 1200,
                        "Packet size differs from expected");

  // extract another packet
  out = rxBuf.Extract (1800);
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 2400,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 1200,
                        "Buffer size differs from expected");
  NS_TEST_ASSERT_MSG_EQ(out->GetSize (), 1200,
                        "Packet size differs from expected");

  // extract another packet
  out = rxBuf.Extract (2400);
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 3600,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 0, "Buffer size differs from expected");
  NS_TEST_ASSERT_MSG_EQ(out->GetSize (), 1200,
                        "Packet size differs from expected");

  // test empty buffer
  out = rxBuf.Extract (3600);
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 3600,
                        "Availability differs from expected");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 0, "Buffer size differs from expected");
  NS_TEST_ASSERT_MSG_NE(!out, true, "Packet size differs from expected");
}

void
QuicRxBufferTestCase::TestStreamAdd ()
{
  // create the buffer
  QuicStreamRxBuffer rxBuf;
  rxBuf.SetMaxBufferSize (18000);

  // create packet
  Ptr<Packet> p = Create<Packet> (1200);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p->GetSize (), false,
                                                            true, false);
  sub.SetOffset(0);

  // add a packet
  bool pos = rxBuf.Add (p, sub);
  std::pair<uint64_t, uint64_t> deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 1200, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 0, "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 1200,
                        "Wrong deliverable packet size");

  // duplicate packet
  bool neg = rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(neg, false, "Added duplicate packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 1200, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 0, "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 1200,
                        "Wrong deliverable packet size");

  // add a second packet
  sub.SetOffset (1200);
  pos = rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 15600, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 2400, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 1200,
                        "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 2400,
                        "Wrong deliverable packet size");

  // insert out of order packet
  sub.SetOffset (3600);
  pos = rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 14400, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 3600, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 1200,
                        "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 2400,
                        "Wrong deliverable packet size");

  // insert missing packet
  sub.SetOffset (2400);
  pos = rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 13200, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 4800, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 3600,
                        "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 4800,
                        "Wrong deliverable packet size");

  // test full buffer
  sub.SetOffset (4800);
  Ptr<Packet> p1 = Create<Packet> (13200);
  pos = rxBuf.Add (p1, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(pos, true, "Failed to add packet");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 0, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 18000, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 4800,
                        "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 18000,
                        "Wrong deliverable packet size");

  // test buffer overflow
  sub.SetOffset (18000);
  neg = rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  NS_TEST_ASSERT_MSG_EQ(neg, false, "Buffer overflow");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 0, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 18000, "Wrong buffer size");
  NS_TEST_ASSERT_MSG_EQ(deliverable.first, 4800,
                        "Wrong deliverable offset value");
  NS_TEST_ASSERT_MSG_EQ(deliverable.second, 18000,
                        "Wrong deliverable packet size");
}

void
QuicRxBufferTestCase::TestStreamExtract ()
{
  // create the buffer
  QuicStreamRxBuffer rxBuf;
  rxBuf.SetMaxBufferSize (18000);

  // create packet
  Ptr<Packet> p = Create<Packet> (1200);
  QuicSubheader sub = QuicSubheader::CreateStreamSubHeader (1, 0, p->GetSize (), false,
                                                   true, false);
  sub.SetOffset (0);

  // add a packet
  rxBuf.Add (p, sub);
  std::pair<uint64_t, uint64_t> deliverable = rxBuf.GetDeliverable (0);

  // add a second packet
  sub.SetOffset (1200);
  rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  // insert a third packet
  sub.SetOffset (2400);
  rxBuf.Add (p, sub);
  deliverable = rxBuf.GetDeliverable (0);

  // extract first two packets
  Ptr<Packet> outPkt = rxBuf.Extract(deliverable.second - 1200);

  NS_TEST_ASSERT_MSG_NE(!outPkt, true, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(),2400,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 16800, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 1200, "Wrong buffer size");

  // insert missing packet
  sub.SetOffset (0);
  rxBuf.Add (outPkt, sub);
  deliverable = rxBuf.GetDeliverable (0);

  // extract all packets
  outPkt = rxBuf.Extract(deliverable.second);

  NS_TEST_ASSERT_MSG_NE(!outPkt, true, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(outPkt->GetSize(), 3600,  "Wrong packet size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 18000, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 0, "Wrong buffer size");

  // test empty buffer
  outPkt = rxBuf.Extract(1200);
  NS_TEST_ASSERT_MSG_NE(!outPkt, true, "Failed to extract packets");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Available (), 18000, "Wrong available data size");
  NS_TEST_ASSERT_MSG_EQ(rxBuf.Size (), 0, "Wrong buffer size");
}

void
QuicRxBufferTestCase::DoTeardown ()
{
}

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief the TestSuite for the QuicSocketRxBuffer and QuicStreamRxBuffer test cases
 */
class QuicRxBufferTestSuite : public TestSuite
{
public:
  QuicRxBufferTestSuite () :
      TestSuite ("quic-rx-buffer", UNIT)
  {
    AddTestCase (new QuicRxBufferTestCase, TestCase::QUICK);
  }
};
static QuicRxBufferTestSuite g_quicRxBufferTestSuite;
