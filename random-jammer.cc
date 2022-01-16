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

#include "random-jammer.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("RandomJammer");

/*
 * Random Jammer.
 */
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomJammer);

TypeId
RandomJammer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomJammer")
    .SetParent<Jammer> ()
    .AddConstructor<RandomJammer> ()
    .AddAttribute ("RandomJammerTxPower",
                   "Power to send jamming signal for random jammer, in Watts.",
                   DoubleValue (0.001), // 0dBm
                   MakeDoubleAccessor (&RandomJammer::SetTxPower,
                                       &RandomJammer::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RandomJammerJammingDuration",
                   "Jamming duration for random jammer.",
                   TimeValue (MilliSeconds (200.0)),
                   MakeTimeAccessor (&RandomJammer::SetJammingDuration,
                                     &RandomJammer::GetJammingDuration),
                   MakeTimeChecker ())
    .AddAttribute ("RandomJammerRandomInterval",
                   "Random jammer interval.",
                   RandomVariableValue (UniformVariable (0.0, 0.001)),
                   MakeRandomVariableAccessor (&RandomJammer::SetRandomVariable),
                   MakeRandomVariableChecker ())
    .AddAttribute ("RandomJammerRxTimeout",
                   "Random jammer RX timeout.",
                   TimeValue (Seconds (2.0)),
                   MakeTimeAccessor (&RandomJammer::SetRxTimeout,
                                     &RandomJammer::GetRxTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("RandomJammerReactToMitigationFlag",
                   "Random jammer react to mitigation flag, set to enable chasing.",
                   UintegerValue (true), // default to not chasing
                   MakeUintegerAccessor (&RandomJammer::SetReactToMitigation,
                                         &RandomJammer::GetReactToMitigation),
                   MakeUintegerChecker<bool> ())
  ;
  return tid;
}

RandomJammer::RandomJammer ()
  :  m_reactToMitigation (false),
     m_reacting (false)
{
}

RandomJammer::~RandomJammer ()
{
}

void
RandomJammer::SetUtility (Ptr<WirelessModuleUtility> utility)
{
  NS_LOG_FUNCTION (this << utility);
  NS_ASSERT (utility != NULL);
  m_utility = utility;
}

void
RandomJammer::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
RandomJammer::SetTxPower (double power)
{
  NS_LOG_FUNCTION (this << power);
  m_txPower = power;
}

double
RandomJammer::GetTxPower (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
RandomJammer::SetJammingDuration (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_jammingDuration = duration;
}

Time
RandomJammer::GetJammingDuration (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingDuration;
}

void
RandomJammer::SetRandomVariable (RandomVariable random)
{
  NS_LOG_FUNCTION (this << random);
  m_randomJammingInterval = random;
}

void
RandomJammer::SetRxTimeout (Time rxTimeout)
{
  NS_LOG_FUNCTION (this << rxTimeout);
  m_rxTimeout = rxTimeout;
}

Time
RandomJammer::GetRxTimeout (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxTimeout;
}

void
RandomJammer::SetReactToMitigation (const bool flag)
{
  NS_LOG_FUNCTION (this << flag);
  m_reactToMitigation = flag;
}

bool
RandomJammer::GetReactToMitigation (void) const
{
  NS_LOG_FUNCTION (this);
  return m_reactToMitigation;
}

/*
 * Private functions start here.
 */

void
RandomJammer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
RandomJammer::DoStopJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
RandomJammer::DoJamming (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_utility);

  if (!IsJammerOn ())
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () << ", Jammer is OFF!");
      return;
    }

  NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                ", Sending jamming signal with power = " << m_txPower << " W" <<
                ", At " << Simulator::Now ().GetSeconds () << "s");

  // send jamming signal
  double actualPower = m_utility->SendJammingSignal (m_txPower,
                                                     m_jammingDuration);
  if (actualPower != 0.0)
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", Jamming signal sent with power = " << actualPower <<
                    " W");
    }
  else
    {
      NS_LOG_ERROR ("RandomJammer:At Node #" << GetId () <<
                    ", Failed to send jamming signal!");
    }

  /*
   * Schedule *first* RX timeout if react to mitigation is enabled. We know if
   * react to jamming mitigation is enabled, there should always be a RX timeout
   * event scheduled. The RX timeout event can only "expire" if it's never been
   * scheduled.
   */
  if (m_reactToMitigation && m_rxTimeoutEvent.IsExpired())
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", After jammer starts, scheduling RX timeout!");
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &RandomJammer::RxTimeoutHandler,
                                              this);
    }

  m_reacting = false; // always reset reacting flag
}

bool
RandomJammer::DoStartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);

  if (m_reactToMitigation)
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", React to mitigation enabled! Rescheduling RX Timeout");
      m_rxTimeoutEvent.Cancel (); // cancel RX timeout event
      // reschedule next RX timeout
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &RandomJammer::RxTimeoutHandler,
                                              this);
    }
  else
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", React to mitigation disabled!");
    }

  return false; // Random Jammer always rejects the incoming packet
}

bool
RandomJammer::DoEndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet << averageRss);
  NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                ", Ignoring incoming packet!");
  return false;
}

void
RandomJammer::DoEndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  NS_LOG_DEBUG("RandomJammer:At Node #" << GetId () <<
               ", Jamming packet is sent with power = " << txPower);
  
  m_jammingEvent.Cancel (); // cancel previously scheduled event

  if (m_reacting)
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", Not sending jamming signal, jammer reacting to mitigation!");
      // channel switch delay
      Time delay = m_utility->GetPhyLayerInfo().channelSwitchDelay;
      // schedule jamming after channel switch delay
      m_jammingEvent = Simulator::Schedule (delay, &RandomJammer::DoJamming,
                                            this);
      return; // do nothing if already reacting to mitigation
    }

  // calculate interval to sending next jamming burst
  Time intervalToNextJamming = Seconds (m_randomJammingInterval.GetValue () +
                                        m_jammingDuration.GetSeconds());

  // schedules send jamming signal in utility.
  m_jammingEvent = Simulator::Schedule (intervalToNextJamming,
                                        &RandomJammer::DoJamming, this);
}

void
RandomJammer::RxTimeoutHandler (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () << ", RX timeout at " <<
                Simulator::Now ().GetSeconds () << "s");
  NS_ASSERT (m_utility != NULL);

  m_rxTimeoutEvent.Cancel (); // cancel previously scheduled RX timeout

  if (!m_reactToMitigation)
    {
      NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                    ", React to mitigation is turned OFF!");
      return; // do nothing if react to mitigation flag is not set
    }

  // calculate channel number
  uint16_t currentChannel = m_utility->GetPhyLayerInfo ().currentChannel;
  uint16_t nextChannel = currentChannel + 1;
  if (nextChannel >= m_utility->GetPhyLayerInfo ().numOfChannels)
    {
      nextChannel = 1;  // wrap around and start form 1
    }

  NS_LOG_DEBUG ("RandomJammer:At Node #" << GetId () <<
                ", Switching from channel " << currentChannel << " >-> " <<
                nextChannel << ", At " << Simulator::Now ().GetSeconds () << "s");

  // hop to next channel
  m_utility->SwitchChannel (nextChannel);

  /*
   * Set reacting flag to indicate jammer is reacting. When the flag is set, no
   * scheduling of new jamming event is allowed.
   */
  m_reacting = true;

  // schedule next RX timeout
  m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                          &RandomJammer::RxTimeoutHandler,
                                          this);
}

} // namespace ns3
