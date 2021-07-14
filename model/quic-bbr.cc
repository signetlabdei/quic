/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal, 
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
 * Authors: Vivek Jain <jain.vivek.anand@gmail.com>
 *          Viyom Mittal <viyommittal@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *          Umberto Paro <umberto.paro@me.com>
 */

#include "quic-bbr.h"
#include "ns3/log.h"
#include "ns3/quic-socket-base.h"
#include "ns3/quic-socket-tx-buffer.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QuicBbr");
NS_OBJECT_ENSURE_REGISTERED (QuicBbr);

const double QuicBbr::PACING_GAIN_CYCLE [] = {5.0 / 4, 3.0 / 4, 1, 1, 1, 1, 1, 1};

TypeId
QuicBbr::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuicBbr")
    .SetParent<QuicCongestionOps> ()
    .AddConstructor<QuicBbr> ()
    .SetGroupName ("Internet")
    .AddAttribute ("HighGain",
                   "Value of high gain",
                   DoubleValue (2.89),
                   MakeDoubleAccessor (&QuicBbr::m_highGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BwWindowLength",
                   "Length of bandwidth windowed filter",
                   UintegerValue (10),
                   MakeUintegerAccessor (&QuicBbr::m_bandwidthWindowLength),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RttWindowLength",
                   "Length of bandwidth windowed filter",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&QuicBbr::m_rtPropFilterLen),
                   MakeTimeChecker ())
    .AddAttribute ("ProbeRttDuration",
                   "Length of bandwidth windowed filter",
                   TimeValue (MilliSeconds (200)),
                   MakeTimeAccessor (&QuicBbr::m_probeRttDuration),
                   MakeTimeChecker ())
    .AddTraceSource ("BbrState", "Current state of the BBR state machine",
                     MakeTraceSourceAccessor (&QuicBbr::m_state),
                     "ns3::QuicBbr::BbrStatesTracedValueCallback")
  ;
  return tid;
}

QuicBbr::QuicBbr ()
  : QuicCongestionOps ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

QuicBbr::QuicBbr (const QuicBbr &sock)
  : QuicCongestionOps (sock),
    m_bandwidthWindowLength (sock.m_bandwidthWindowLength),
    m_pacingGain (sock.m_pacingGain),
    m_cWndGain (sock.m_cWndGain),
    m_highGain (sock.m_highGain),
    m_isPipeFilled (sock.m_isPipeFilled),
    m_minPipeCwnd (sock.m_minPipeCwnd),
    m_roundCount (sock.m_roundCount),
    m_roundStart (sock.m_roundStart),
    m_nextRoundDelivered (sock.m_nextRoundDelivered),
    m_probeRttDuration (sock.m_probeRttDuration),
    m_probeRtPropStamp (sock.m_probeRtPropStamp),
    m_probeRttDoneStamp (sock.m_probeRttDoneStamp),
    m_probeRttRoundDone (sock.m_probeRttRoundDone),
    m_packetConservation (sock.m_packetConservation),
    m_priorCwnd (sock.m_priorCwnd),
    m_idleRestart (sock.m_idleRestart),
    m_targetCWnd (sock.m_targetCWnd),
    m_fullBandwidth (sock.m_fullBandwidth),
    m_fullBandwidthCount (sock.m_fullBandwidthCount),
    m_rtProp (Time::Max ()),
    m_sendQuantum (sock.m_sendQuantum),
    m_cycleStamp (sock.m_cycleStamp),
    m_cycleIndex (sock.m_cycleIndex),
    m_rtPropExpired (sock.m_rtPropExpired),
    m_rtPropFilterLen (sock.m_rtPropFilterLen),
    m_rtPropStamp (sock.m_rtPropStamp),
    m_isInitialized (sock.m_isInitialized)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

//随机数生成
int64_t
QuicBbr::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}


//初始化与轮数计数相关的变量
void
QuicBbr::InitRoundCounting ()
{
  NS_LOG_FUNCTION (this);
  m_nextRoundDelivered = 0;
  m_roundStart = false;
  m_roundCount = 0;
}

//初始化完整的管道估计器。
void
QuicBbr::InitFullPipe ()
{
  NS_LOG_FUNCTION (this);
  m_isPipeFilled = false;
  m_fullBandwidth = 0;
  m_fullBandwidthCount = 0;
}

//初始化pacingrate
void
QuicBbr::InitPacingRate (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);

  if (!tcb->m_pacing)
    {
      NS_LOG_WARN ("BBR must use pacing");
      tcb->m_pacing = true;
    }
  Time rtt = tcb->m_lastRtt != Time::Max () ? tcb->m_lastRtt.Get () : MilliSeconds (1);
  if (rtt == Seconds (0))
    {
      NS_LOG_INFO ("No rtt estimate is available, using kDefaultInitialRtt=" << tcb->m_kDefaultInitialRtt);
      rtt = tcb->m_kDefaultInitialRtt;
    }
  DataRate nominalBandwidth (tcb->m_initialCWnd * 8 / rtt.GetSeconds ());
  tcb->m_pacingRate = DataRate (m_pacingGain * nominalBandwidth.GetBitRate ());

}

//更新特定于 BBR_STARTUP 状态的变量
void
QuicBbr::EnterStartup ()
{
  NS_LOG_FUNCTION (this);
  SetBbrState (BbrMode_t::BBR_STARTUP);
  m_pacingGain = m_highGain;
  m_cWndGain = m_highGain;
}

//如果套接字从空闲状态重新启动，则更新pacingrate
void
QuicBbr::HandleRestartFromIdle (Ptr<QuicSocketState> tcb, const RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  if (tcb->m_bytesInFlight.Get () == 0U && rs->m_isAppLimited)
    {
      m_idleRestart = true;
      if (m_state.Get () == BbrMode_t::BBR_PROBE_BW)
        {
          SetPacingRate (tcb, 1);
        }
    }
}

//根据网络模型更新pacingrate 更新机制：
void
QuicBbr::SetPacingRate (Ptr<QuicSocketState> tcb, double gain)
{
  NS_LOG_FUNCTION (this << tcb << gain);
  DataRate rate (gain * m_maxBwFilter.GetBest ().GetBitRate ());
  rate = std::min (rate, tcb->m_maxPacingRate);
  if (m_isPipeFilled || rate > tcb->m_pacingRate)
    {
      tcb->m_pacingRate = rate;
    }
}

//估计拥塞窗口的目标值
uint32_t
QuicBbr::InFlight (Ptr<QuicSocketState> tcb, double gain)
{
  NS_LOG_FUNCTION (this << tcb << gain);
  if (m_rtProp == Time::Max ())
    {
      return tcb->m_initialCWnd;
    }
  double quanta = 3 * m_sendQuantum;
  double estimatedBdp = m_maxBwFilter.GetBest () * m_rtProp / 8.0;
  return gain * estimatedBdp + quanta;
}


//在 BBR_PROBE_BW 状态下使用循环增益算法提高pacing gain
void
QuicBbr::AdvanceCyclePhase ()
{
  NS_LOG_FUNCTION (this);
  m_cycleStamp = Simulator::Now ();
  m_cycleIndex = (m_cycleIndex + 1) % GAIN_CYCLE_LENGTH;
  m_pacingGain = PACING_GAIN_CYCLE [m_cycleIndex];
}


//检查是否在 BBR_PROBE_BW 中移动到下一个pacing gain值
bool
QuicBbr::IsNextCyclePhase (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  bool isFullLength = (Simulator::Now () - m_cycleStamp) > m_rtProp;
  if (m_pacingGain == 1)
    {
      return isFullLength;
    }
  else if (m_pacingGain > 1)
    {
      return isFullLength && (rs->m_packetLoss > 0 || rs->m_priorInFlight >= InFlight (tcb, m_pacingGain));
    }
  else
    {
      return isFullLength || rs->m_priorInFlight <= InFlight (tcb, 1);
    }
}

//在BBR_PROBE_BW 状态下是否提高pacing增益，如果是调用AdvanceCyclePhase()
void
QuicBbr::CheckCyclePhase (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  if (m_state.Get () == BbrMode_t::BBR_PROBE_BW && IsNextCyclePhase (tcb, rs))
    {
      AdvanceCyclePhase ();
    }
}

//检查管道是否已经充满
void
QuicBbr::CheckFullPipe (const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << rs);
  if (m_isPipeFilled || !m_roundStart || rs->m_isAppLimited)
    {
      return;
    }

  /* Check if Bottleneck bandwidth is still growing*/
  if (m_maxBwFilter.GetBest ().GetBitRate () >= m_fullBandwidth.GetBitRate () * 1.25)
    {
      m_fullBandwidth = m_maxBwFilter.GetBest ();
      m_fullBandwidthCount = 0;
      return;
    }

  m_fullBandwidthCount++;
  if (m_fullBandwidthCount >= 3)
    {
      NS_LOG_DEBUG ("Pipe filled");
      m_isPipeFilled = true;
    }
}

//更新 BBR_DRAIN 状态下的特定变量
void
QuicBbr::EnterDrain ()
{
  NS_LOG_FUNCTION (this);
  SetBbrState (BbrMode_t::BBR_DRAIN);
  m_pacingGain = 1.0 / m_highGain;
  m_cWndGain = m_highGain;
}

//更新 BBR_PROBE_BW 状态下的特定变量 
void
QuicBbr::EnterProbeBW ()
{
  NS_LOG_FUNCTION (this);
  SetBbrState (BbrMode_t::BBR_PROBE_BW);
  m_pacingGain = 1;
  m_cWndGain = 2;
  m_cycleIndex = GAIN_CYCLE_LENGTH - 1 - (int) m_uv->GetValue (0, 8);
  AdvanceCyclePhase ();
}

//检查是否应该进入BBR_DRAIN或BBR_PROBE_BW 状态
void
QuicBbr::CheckDrain (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  if (m_state.Get () == BbrMode_t::BBR_STARTUP && m_isPipeFilled)
    {
      EnterDrain ();
    }

  if (m_state.Get () == BbrMode_t::BBR_DRAIN && tcb->m_bytesInFlight <= InFlight (tcb, 1))
    {
      EnterProbeBW ();
    }
}

//更新最小RTT
void
QuicBbr::UpdateRTprop (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  m_rtPropExpired = Simulator::Now () > (m_rtPropStamp + m_rtPropFilterLen);
  if (tcb->m_lastRtt >= Seconds (0) && (tcb->m_lastRtt <= m_rtProp || m_rtPropExpired))
    {
      m_rtProp = tcb->m_lastRtt;
      m_rtPropStamp = Simulator::Now ();
    }
}

//更新BBR_PROBE_RTT 状态下的特定变量
void
QuicBbr::EnterProbeRTT ()
{
  NS_LOG_FUNCTION (this);
  SetBbrState (BbrMode_t::BBR_PROBE_RTT);
  m_pacingGain = 1;
  m_cWndGain = 1;
}

//帮助记住最近已知的良好拥塞窗口或最近未被丢失恢复或ProbeRTT调制的拥塞窗口
void
QuicBbr::SaveCwnd (Ptr<const QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  if (tcb->m_congState != TcpSocketState::CA_RECOVERY && m_state.Get () != BbrMode_t::BBR_PROBE_RTT)
    {
      m_priorCwnd = tcb->m_cWnd;
    }
  else
    {
      m_priorCwnd = std::max (m_priorCwnd, tcb->m_cWnd.Get ());
    }
}

//帮助恢复最后已知的良好的拥塞窗口
void
QuicBbr::RestoreCwnd (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  tcb->m_cWnd = std::max (m_priorCwnd, tcb->m_cWnd.Get ());
}
//根据管道是否充满判断执行 EnterProbeBW () 或者是EnterStartup () 
void
QuicBbr::ExitProbeRTT ()
{
  NS_LOG_FUNCTION (this);
  if (m_isPipeFilled)
    {
      EnterProbeBW ();
    }
  else
    {
      EnterStartup ();
    }
}

//BBR_PROBE_RTT状态的处理步骤
void
QuicBbr::HandleProbeRTT (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);

  tcb->m_appLimitedUntil = (tcb->m_delivered + tcb->m_bytesInFlight.Get ()) ?: 1;

  if (m_probeRttDoneStamp == Seconds (0) && tcb->m_bytesInFlight <= m_minPipeCwnd)
    {
      m_probeRttDoneStamp = Simulator::Now () + m_probeRttDuration;
      m_probeRttRoundDone = false;
      m_nextRoundDelivered = tcb->m_delivered;
    }
  else if (m_probeRttDoneStamp != Seconds (0))
    {
      if (m_roundStart)
        {
          m_probeRttRoundDone = true;
        }
      if (m_probeRttRoundDone && Simulator::Now () > m_probeRttDoneStamp)
        {
          m_rtPropStamp = Simulator::Now ();
          RestoreCwnd (tcb);
          ExitProbeRTT ();
        }
    }
}


//处理与ProbeRTT状态相关的步骤
void
QuicBbr::CheckProbeRTT (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  NS_LOG_DEBUG (Simulator::Now () << "WhichState " << WhichState (m_state.Get ())
                                  << " m_rtPropExpired " << m_rtPropExpired << " !m_idleRestart "
                                  << !m_idleRestart);
  if (m_state.Get () != BbrMode_t::BBR_PROBE_RTT && m_rtPropExpired && !m_idleRestart)
    {
      EnterProbeRTT ();
      SaveCwnd (tcb);
      m_probeRttDoneStamp = Seconds (0);
    }

  if (m_state.Get () == BbrMode_t::BBR_PROBE_RTT)
    {
      HandleProbeRTT (tcb);
    }

  m_idleRestart = false;
}

//更新基于网络模型的发送额
void
QuicBbr::SetSendQuantum (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  m_sendQuantum = 1 * tcb->m_segmentSize;
/*TODO
  Since TSO can't be implemented in ns-3
  if (tcb->m_pacingRate < DataRate ("1.2Mbps"))
    {
      m_sendQuantum = 1 * tcb->m_segmentSize;
    }
  else if (tcb->m_pacingRate < DataRate ("24Mbps"))
    {
      m_sendQuantum  = 2 * tcb->m_segmentSize;
    }
  else
    {
      m_sendQuantum = std::min (tcb->m_pacingRate.GetBitRate () * MilliSeconds (1).GetMilliSeconds () / 8, (uint64_t) 64000);
    }*/
}

//更新目标cwnd值
void
QuicBbr::UpdateTargetCwnd (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  m_targetCWnd = InFlight (tcb, m_cWndGain);
}

//调节CA_RECOVERY状态中的拥塞窗口
void
QuicBbr::ModulateCwndForRecovery (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  if ( rs->m_packetLoss > 0)
    {
      tcb->m_cWnd = std::max ((int) tcb->m_cWnd.Get () - (int) rs->m_packetLoss, (int) tcb->m_segmentSize);
    }

  if (m_packetConservation)
    {
      tcb->m_cWnd = std::max (tcb->m_cWnd.Get (), tcb->m_bytesInFlight.Get () + tcb->m_lastAckedSackedBytes);
    }
}

//调节BBR_PROBE_RTT中的拥塞窗口
void
QuicBbr::ModulateCwndForProbeRTT (Ptr<QuicSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  if (m_state.Get () == BbrMode_t::BBR_PROBE_RTT)
    {
      tcb->m_cWnd = std::min (tcb->m_cWnd.Get (), m_minPipeCwnd);
    }
}

//根据网络模型更新cwnd
void
QuicBbr::SetCwnd (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  UpdateTargetCwnd (tcb);

  if (tcb->m_congState == TcpSocketState::CA_RECOVERY)
    {
      ModulateCwndForRecovery (tcb, rs);
    }

  if (!m_packetConservation)
    {
      if (m_isPipeFilled)
        {
          tcb->m_cWnd = std::min (tcb->m_cWnd.Get () + (uint32_t) tcb->m_lastAckedSackedBytes, m_targetCWnd);
        }
      else if (tcb->m_cWnd < m_targetCWnd || tcb->m_delivered < tcb->m_initialCWnd)
        {
          tcb->m_cWnd = tcb->m_cWnd.Get () + tcb->m_lastAckedSackedBytes;
        }
      tcb->m_cWnd = std::max (tcb->m_cWnd.Get (), m_minPipeCwnd);
    }
  ModulateCwndForProbeRTT (tcb);
  if (tcb->m_congState == TcpSocketState::CA_RECOVERY)
    {
      m_packetConservation = false;
    }
}

//更新轮计数相关变量
void
QuicBbr::UpdateRound (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  if (tcb->m_txItemDelivered >= m_nextRoundDelivered)
    {
      m_nextRoundDelivered = tcb->m_delivered;
      m_roundCount++;
      m_roundStart = true;
    }
  else
    {
      m_roundStart = false;
    }
}


//更新最大瓶颈带宽
void
QuicBbr::UpdateBtlBw (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  if (rs->m_deliveryRate == 0)
    {
      return;
    }

  UpdateRound (tcb, rs);

  if (rs->m_deliveryRate >= m_maxBwFilter.GetBest () || !rs->m_isAppLimited)
    {
      m_maxBwFilter.Update (rs->m_deliveryRate, m_roundCount);
    }
}


//更新BBR网络模型 即更新最大带宽以及最小RTT
void
QuicBbr::UpdateModelAndState (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  UpdateBtlBw (tcb, rs);
  CheckCyclePhase (tcb, rs);
  CheckFullPipe (rs);
  CheckDrain (tcb);
  UpdateRTprop (tcb);
  CheckProbeRTT (tcb);
}

//更新控制参数，包括cwnd、pacingrate、发送原子
void
QuicBbr::UpdateControlParameters (Ptr<QuicSocketState> tcb, const struct RateSample * rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  SetPacingRate (tcb, m_pacingGain);
  SetSendQuantum (tcb);
  SetCwnd (tcb, rs);
}

//状态映射
std::string
QuicBbr::WhichState (BbrMode_t mode) const
{
  switch (mode)
    {
      case 0:
        return "BBR_STARTUP";
      case 1:
        return "BBR_DRAIN";
      case 2:
        return "BBR_PROBE_BW";
      case 3:
        return "BBR_PROBE_RTT";
      default:
        NS_ABORT_MSG ("Invalid BBR state");
        return "";
    }
}

//根据状态映射，设置bbr状态
void
QuicBbr::SetBbrState (BbrMode_t mode)
{
  NS_LOG_FUNCTION (this << mode);
  NS_LOG_DEBUG (Simulator::Now () << " Changing from " << WhichState (m_state) << " to " << WhichState (mode));
  m_state = mode;
}

//获取bbr状态
uint32_t
QuicBbr::GetBbrState ()
{
  NS_LOG_FUNCTION (this);
  return m_state.Get ();
}

//获取当前cwnd增益
double
QuicBbr::GetCwndGain ()
{
  NS_LOG_FUNCTION (this);
  return m_cWndGain;
}

//获取当前pacing增益
double
QuicBbr::GetPacingGain ()
{
  NS_LOG_FUNCTION (this);
  return m_pacingGain;
}


std::string
QuicBbr::GetName () const
{
  return "QuicBbr";
}


//数据包发送时被调用以更新cwnd和pacing rate
void
QuicBbr::CongControl (Ptr<QuicSocketState> tcb, const struct RateSample *rs)
{
  NS_LOG_FUNCTION (this << tcb << rs);
  UpdateModelAndState (tcb, rs);
  UpdateControlParameters (tcb, rs);
}

//标记rc rs为未使用
void
QuicBbr::CongControl (Ptr<TcpSocketState> tcb,
                      const TcpRateOps::TcpRateConnection &rc,
                      const TcpRateOps::TcpRateSample &rs)
{
    NS_LOG_FUNCTION (this << tcb);
    //将局部变量标记为未使用
    NS_UNUSED (rc);
    NS_UNUSED (rs);
}

//触发应用于特定拥塞状态的事件/计算
void
QuicBbr::CongestionStateSet (Ptr<TcpSocketState> tcb,
                             const TcpSocketState::TcpCongState_t newState)
{
  NS_LOG_FUNCTION (this << tcb << newState);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState *> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  if (newState == TcpSocketState::CA_OPEN && !m_isInitialized)
    {
      NS_LOG_DEBUG ("CongestionStateSet triggered to CA_OPEN :: " << newState);
      m_rtProp = tcbd->m_lastRtt.Get () != Seconds (0) ? tcbd->m_lastRtt.Get () : Time::Max ();
      m_rtPropStamp = Simulator::Now ();
      m_priorCwnd = tcbd->m_initialCWnd;
      m_targetCWnd = tcbd->m_initialCWnd;
      m_minPipeCwnd = 4 * tcbd->m_segmentSize;
      m_sendQuantum = 1 * tcbd->m_segmentSize;
      m_maxBwFilter = MaxBandwidthFilter_t (m_bandwidthWindowLength,
                                            DataRate (tcbd->m_initialCWnd * 8 / m_rtProp.GetSeconds ())
                                            , 0);
      InitRoundCounting ();
      InitFullPipe ();
      EnterStartup ();
      InitPacingRate (tcbd);
      m_isInitialized = true;
    }
  else if (newState == TcpSocketState::CA_LOSS)
    {
      NS_LOG_DEBUG ("CongestionStateSet triggered to CA_LOSS :: " << newState);
      SaveCwnd (tcbd);
      tcbd->m_cWnd = tcbd->m_segmentSize;
      m_roundStart = true;
    }
  else if (newState == TcpSocketState::CA_RECOVERY)
    {
      NS_LOG_DEBUG ("CongestionStateSet triggered to CA_RECOVERY :: " << newState);
      SaveCwnd (tcbd);
      tcbd->m_cWnd = tcbd->m_bytesInFlight.Get () + std::max (tcbd->m_lastAckedSackedBytes, tcbd->m_segmentSize);
      m_packetConservation = true;
    }
}

//发生拥塞窗口事件时触发事件/计算
void
QuicBbr::CwndEvent (Ptr<TcpSocketState> tcb,
                    const TcpSocketState::TcpCAEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState *> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  if (event == TcpSocketState::CA_EVENT_COMPLETE_CWR)
    {
      NS_LOG_DEBUG ("CwndEvent triggered to CA_EVENT_COMPLETE_CWR :: " << event);
      m_packetConservation = false;
      RestoreCwnd (tcbd);
    }
  else if (event == TcpSocketState::CA_EVENT_TX_START)
    {
      NS_LOG_DEBUG ("CwndEvent triggered to CA_EVENT_TX_START :: " << event);
      if (tcbd->m_bytesInFlight.Get () == 0 && tcbd->m_appLimitedUntil > tcbd->m_delivered)
        {
          m_idleRestart = true;
          if (m_state.Get () == BbrMode_t::BBR_PROBE_BW && tcbd->m_appLimitedUntil > tcbd->m_delivered)
            {
              SetPacingRate (tcbd, 1);
            }
        }
    }
}

//在丢包事件发生后获取慢启动门限值
uint32_t
QuicBbr::GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);
  Ptr<const QuicSocketState> tcbd = dynamic_cast<const QuicSocketState *> (&(*tcb));
  if (tcbd)
    {
      SaveCwnd (tcbd);
    }
  return tcb->m_initialSsThresh;
}

//提升拥塞窗口 无实质操作 标记tcb和segmentsAcked为未使用
void
QuicBbr::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_UNUSED (tcb);
  NS_UNUSED (segmentsAcked);
}

//当数据包发送时触发
void
QuicBbr::OnPacketSent (Ptr<TcpSocketState> tcb, SequenceNumber32 packetNumber, bool isAckOnly)
{
  NS_LOG_FUNCTION (this << packetNumber << isAckOnly);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState *> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  tcbd->m_timeOfLastSentPacket = Now ();
  tcbd->m_highTxMark = packetNumber;
}

//收到ack处理函数
void
QuicBbr::OnAckReceived (Ptr<TcpSocketState> tcb, QuicSubheader &ack,
                        std::vector<Ptr<QuicSocketTxItem> > newAcks,
                        const struct RateSample *rs)
{
  NS_LOG_FUNCTION (this);

  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState *> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  tcbd->m_largestAckedPacket = SequenceNumber32 (ack.GetLargestAcknowledged ());

  // newAcks are ordered from the highest packet number to the smalles
  Ptr<QuicSocketTxItem> lastAcked = newAcks.at (0);

  NS_LOG_LOGIC ("Updating RTT estimate");
  // If the largest acked is newly acked, update the RTT.
  if (lastAcked->m_packetNumber == tcbd->m_largestAckedPacket)
    {
      tcbd->m_lastRtt = Now () - lastAcked->m_lastSent;
      UpdateRtt (tcbd, tcbd->m_lastRtt, MicroSeconds (ack.GetAckDelay ()));
    }

  // Precess end of recovery
  if ((tcbd->m_congState == TcpSocketState::CA_RECOVERY or
       tcbd->m_congState == TcpSocketState::CA_LOSS) and
      tcbd->m_endOfRecovery <= tcbd->m_largestAckedPacket)
    {
      tcbd->m_congState = TcpSocketState::CA_OPEN;
      CongestionStateSet (tcb, TcpSocketState::CA_OPEN);
      CwndEvent (tcb, TcpSocketState::CA_EVENT_COMPLETE_CWR);
    }

  NS_LOG_LOGIC ("Processing acknowledged packets");
  // Process each acked packet
  for (auto it = newAcks.rbegin (); it != newAcks.rend (); ++it)
    {
      if ((*it)->m_acked)
        {
          OnPacketAcked (tcb, (*it));
        }
    }
  CongControl (tcbd, rs);
}

//丢包处理
void
QuicBbr::OnPacketsLost (Ptr<TcpSocketState> tcb, std::vector<Ptr<QuicSocketTxItem> > lostPackets)
{
  NS_LOG_LOGIC (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState *> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  auto largestLostPacket = *(lostPackets.end () - 1);

  NS_LOG_INFO ("Go in recovery mode");

  // TCP early retransmit logic [RFC 5827]: enter recovery (RFC 6675, Sec. 5)
  if (!InRecovery (tcb, largestLostPacket->m_packetNumber))
    {
      tcbd->m_endOfRecovery = tcbd->m_highTxMark;
      tcbd->m_congState = TcpSocketState::CA_RECOVERY;
      CongestionStateSet (tcbd, TcpSocketState::CA_RECOVERY);
    }
}
//收到ack后 根据数据包编号进行后续处理 被调用
void
QuicBbr::OnPacketAcked (Ptr<TcpSocketState> tcb, Ptr<QuicSocketTxItem> ackedPacket)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");

  NS_LOG_LOGIC ("Handle possible RTO");
  // If a packet sent prior to RTO was acked, then the RTO  was spurious. Otherwise, inform congestion control.
  if (tcbd->m_rtoCount > 0
      and ackedPacket->m_packetNumber > tcbd->m_largestSentBeforeRto)
    {
      OnRetransmissionTimeoutVerified (tcb);
    }
  tcbd->m_handshakeCount = 0;
  tcbd->m_tlpCount = 0;
  tcbd->m_rtoCount = 0;
}

//重传超时验证，一旦重传超时触发后，在处理RTO后的第一个确认时，QUIC将cwnd减小到最小值，即类似于拥塞算法状态初始化
void
QuicBbr::OnRetransmissionTimeoutVerified (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);
  Ptr<QuicSocketState> tcbd = dynamic_cast<QuicSocketState*> (&(*tcb));
  NS_ASSERT_MSG (tcbd != 0, "tcb is not a QuicSocketState");
  NS_LOG_INFO ("Loss state");
  tcbd->m_congState = TcpSocketState::CA_LOSS;
  CongestionStateSet (tcbd, TcpSocketState::CA_LOSS);
}

Ptr<TcpCongestionOps>
QuicBbr::Fork (void)
{
  return CopyObject<QuicBbr> (this);
}

} // namespace ns3
