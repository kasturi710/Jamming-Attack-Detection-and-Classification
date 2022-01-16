/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */
 
#include "reactive-jammer.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("ReactiveJammer");

/*
 * Reactive Jammer
 */
 namespace ns3 {
 	
NS_OBJECT_ENSURE_REGISTERED (ReactiveJammer);
 	
TypeId
ReactiveJammer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ReactiveJammer")
    .SetParent<Jammer> ()
    .AddConstructor<ReactiveJammer> ()
    .AddAttribute ("ReactiveJammerTxPower",
                   "Power to send jamming signal for reactive jammer, in Watts.",
                   DoubleValue (0.001), // 1mW = 0dBm
                   MakeDoubleAccessor (&ReactiveJammer::SetTxPower,
                                       &ReactiveJammer::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReactiveJammerJammingDuration",
                   "Jamming duration for constant jammer.",
                   TimeValue (MilliSeconds (5.0)),
                   MakeTimeAccessor (&ReactiveJammer::SetJammingDuration,
                                     &ReactiveJammer::GetJammingDuration),
                   MakeTimeChecker ())
    .AddAttribute ("ReactiveJammerRxTxSwitchingDelay",
                   "Reactive jammer rx to tx switching delay.",
                   TimeValue (MicroSeconds (500.0)),
                   MakeTimeAccessor (&ReactiveJammer::SetRxTxSwitchingDelay,
                                     &ReactiveJammer::GetRxTxSwitchingDelay),
                   MakeTimeChecker ())
    .AddAttribute ("ReactiveJammerReactionStrategy",
                   "Reaction strategy of the reactive jammer",
                   UintegerValue (0), // default to energy-aware
                   MakeUintegerAccessor (&ReactiveJammer::SetReactionStrategy,
                                         &ReactiveJammer::GetReactionStrategy),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ReactiveJammerFixedProbability",
                   "Fixed probability of reacting to packets, for reactive jammer.",
                   DoubleValue (1.0), // default to *always* react to packets
                   MakeDoubleAccessor (&ReactiveJammer::SetFixedProbability,
                                       &ReactiveJammer::GetFixedProbability),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReactiveJammerRxTimeout",
                   "Reactive jammer RX timeout.",
                   TimeValue (Seconds (2.0)),
                   MakeTimeAccessor (&ReactiveJammer::SetRxTimeout,
                                     &ReactiveJammer::GetRxTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("ReactiveJammerReactToMitigation",
                   "Reactive jammer react to mitigation flag, set to enable chasing.",
                   UintegerValue (false), // default to not react to mitigation
                   MakeUintegerAccessor (&ReactiveJammer::SetReactToMitigation,
                                         &ReactiveJammer::GetReactToMitigation),
                   MakeUintegerChecker<bool> ())
  ;
  return tid;
}

ReactiveJammer::ReactiveJammer ()
  : m_random (0.0, 1.0),
    m_reactToMitigation (false)
{
}

ReactiveJammer::~ReactiveJammer ()
{
}

void
ReactiveJammer::SetUtility (Ptr<WirelessModuleUtility> utility)
{
  NS_LOG_FUNCTION (this << utility);
  NS_ASSERT (utility != NULL);
  m_utility = utility;
}

void
ReactiveJammer::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
ReactiveJammer::SetTxPower (double power)
{
  NS_LOG_FUNCTION (this << power);
  m_txPower = power;
}

double
ReactiveJammer::GetTxPower (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
ReactiveJammer::SetJammingDuration (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_jammingDuration = duration;
}

Time
ReactiveJammer::GetJammingDuration (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingDuration;
}

void
ReactiveJammer::SetRxTxSwitchingDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  NS_ASSERT (delay.GetSeconds () >= 0);
  m_rxTxSwitchingDelay = delay;
}

Time
ReactiveJammer::GetRxTxSwitchingDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxTxSwitchingDelay;
}

void
ReactiveJammer::SetReactionStrategy(ReactionStrategy strategy)
{
  NS_LOG_FUNCTION (this << strategy);
  m_reactionStrategy = strategy;
}

uint32_t
ReactiveJammer::GetReactionStrategy(void) const
{
  NS_LOG_FUNCTION (this);
  return m_reactionStrategy;
}

void
ReactiveJammer::SetFixedProbability(double probability)
{
  NS_LOG_FUNCTION (this << probability);
  NS_ASSERT (probability >=0 && probability <= 1);
  m_fixedProbability = probability;
}

double
ReactiveJammer::GetFixedProbability (void) const
{
  NS_LOG_FUNCTION (this);
  return m_fixedProbability;
}

void
ReactiveJammer::SetRxTimeout (Time rxTimeout)
{
  NS_LOG_FUNCTION (this << rxTimeout);
  m_rxTimeout = rxTimeout;
}

Time
ReactiveJammer::GetRxTimeout (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxTimeout;
}

void
ReactiveJammer::SetReactToMitigation (const bool flag)
{
  NS_LOG_FUNCTION (this << flag);
  m_reactToMitigation = flag;
}

bool
ReactiveJammer::GetReactToMitigation (void) const
{
  NS_LOG_FUNCTION (this);
  return m_reactToMitigation;
}

/*
 * Private functions start here.
 */

void
ReactiveJammer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
ReactiveJammer::DoStopJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
ReactiveJammer::DoJamming (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Started!");

  if (m_reactToMitigation)
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", After jammer starts, scheduling RX timeout!");
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &ReactiveJammer::RxTimeoutHandler,
                                              this);
    }
}

bool
ReactiveJammer::DoStartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Started receiving a packet!");

  if (IsPacketToBeJammed (packet))
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", Decided to jam this packet!");
      m_jammingEvent.Cancel (); // cancel previously scheduled event
      // react to packet
      m_jammingEvent = Simulator::Schedule (m_rxTxSwitchingDelay,
                                            &ReactiveJammer::ReactToPacket,
                                            this);
    }
  else
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", Decided NOT to jam this packet!");
    }

  if (m_reactToMitigation)
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", React to mitigation enabled! Rescheduling RX Timeout");
      m_rxTimeoutEvent.Cancel (); // cancel RX timeout event
      // reschedule next RX timeout
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &ReactiveJammer::RxTimeoutHandler,
                                              this);
    }
  else
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", React to mitigation disabled!");
    }

  return false; // Reactive Jammer always ignores incoming packets
}

bool
ReactiveJammer::DoEndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Ignoring incoming packet!");
  return false;
}

void
ReactiveJammer::DoEndTxHandler(Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Done sending jamming signal with power = " << txPower);
}

bool
ReactiveJammer::IsPacketToBeJammed (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Deciding whether to react to packet!");

  double energyFraction;
  switch (m_reactionStrategy)
    {
    case ENERGY_AWARE:
      energyFraction = m_source->GetEnergyFraction ();
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", Energy fraction = " << energyFraction);
      // make probabilistic decision based on energy fraction
      return (m_random.GetValue() < energyFraction);
    case FIXED_PROBABILITY:
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", Fixed probability " << m_fixedProbability);
      // make probabilistic decision based on fixed probability
      return (m_random.GetValue () < m_fixedProbability);
    default:
      NS_FATAL_ERROR ("ReactiveJammer:At Node #" << GetId () <<
                      ", Error! Unknown strategy of reaction.");
      break;
    }
  return false;
}

void
ReactiveJammer::ReactToPacket (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Sending jamming signal with power = " << m_txPower << " W");

  // send jamming signal
  double actualPower = m_utility->SendJammingSignal (m_txPower, m_jammingDuration);
  if (actualPower != 0.0)
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", Jamming signal sent with power = " << actualPower << " W");
    }
  else
    {
      NS_LOG_ERROR ("ReactiveJammer:At Node #" << GetId () <<
                    ", Failed to send jamming signal!");
    }
}

void
ReactiveJammer::RxTimeoutHandler (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () << ", RX timeout at " <<
                Simulator::Now ().GetSeconds () << "s");
  NS_ASSERT (m_utility != NULL);

  m_rxTimeoutEvent.Cancel (); // cancel previously scheduled RX timeout

  if (!m_reactToMitigation)
    {
      NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                    ", React to mitigation is turned OFF!");
      return;
    }

  // calculate channel number
  uint16_t currentChannel = m_utility->GetPhyLayerInfo ().currentChannel;
  uint16_t nextChannel = currentChannel + 1;
  if (nextChannel >= m_utility->GetPhyLayerInfo ().numOfChannels)
    {
      nextChannel = 1;  // wrap around and start form 1
    }

  NS_LOG_DEBUG ("ReactiveJammer:At Node #" << GetId () <<
                ", Switching from channel " << currentChannel << " >-> " <<
                nextChannel);

  m_utility->SwitchChannel (nextChannel); // hop to next channel

  // schedule next RX timeout
  m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                          &ReactiveJammer::RxTimeoutHandler,
                                          this);
}

} // namespace ns3
  
