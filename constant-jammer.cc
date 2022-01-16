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
 
#include "constant-jammer.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("ConstantJammer");

/*
 * Constant Jammer
 */
 namespace ns3 {
 	
NS_OBJECT_ENSURE_REGISTERED (ConstantJammer);
 	
TypeId
ConstantJammer::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::ConstantJammer")
    .SetParent<Jammer> ()
    .AddConstructor<ConstantJammer> ()
    .AddAttribute ("ConstantJammerTxPower",
                   "Power to send jamming signal for constant jammer, in Watts.",
                   DoubleValue (0.001), // 0dBm
                   MakeDoubleAccessor (&ConstantJammer::SetTxPower,
                                       &ConstantJammer::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ConstantJammerJammingDuration",
                   "Jamming duration for constant jammer.",
                   TimeValue (MilliSeconds (5.0)),
                   MakeTimeAccessor (&ConstantJammer::SetJammingDuration,
                                     &ConstantJammer::GetJammingDuration),
                   MakeTimeChecker ())
    .AddAttribute ("ConstantJammerConstantInterval",
                   "Constant jammer jamming interval.",
                   TimeValue (MilliSeconds (0.0)),  // Set to 0 for continuous jamming
                   MakeTimeAccessor (&ConstantJammer::SetConstantJammingInterval,
                                     &ConstantJammer::GetConstantJammingInterval),
                   MakeTimeChecker ())
    .AddAttribute ("ConstantJammerRxTimeout",
                   "Constant jammer RX timeout.",
                   TimeValue (Seconds (2.0)),
                   MakeTimeAccessor (&ConstantJammer::SetRxTimeout,
                                     &ConstantJammer::GetRxTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("ConstantJammerReactToMitigationFlag",
                   "Constant jammer react to mitigation flag, set to enable chasing.",
                   UintegerValue (false), // default to no chasing
                   MakeUintegerAccessor (&ConstantJammer::SetReactToMitigation,
                                         &ConstantJammer::GetReactToMitigation),
                   MakeUintegerChecker<bool> ())
  ;
  return tid;
}

ConstantJammer::ConstantJammer ()
  :  m_reactToMitigation (false),
     m_reacting (false)
{
}

ConstantJammer::~ConstantJammer ()
{
}

void
ConstantJammer::SetUtility (Ptr<WirelessModuleUtility> utility)
{
  NS_LOG_FUNCTION (this << utility);
  NS_ASSERT (utility != NULL);
  m_utility = utility;
}

void
ConstantJammer::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
ConstantJammer::SetTxPower (double power)
{
  NS_LOG_FUNCTION (this << power);
  m_txPower = power;
}

double
ConstantJammer::GetTxPower (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
ConstantJammer::SetJammingDuration (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_jammingDuration = duration;
}

Time
ConstantJammer::GetJammingDuration (void) const
{
  NS_LOG_FUNCTION (this);
  return m_jammingDuration;
}

void
ConstantJammer::SetConstantJammingInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  NS_ASSERT (interval.GetSeconds () >= 0);
  m_constantJammingInterval = interval;
}

Time
ConstantJammer::GetConstantJammingInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_constantJammingInterval;
}

void
ConstantJammer::SetRxTimeout (Time rxTimeout)
{
  NS_LOG_FUNCTION (this << rxTimeout);
  m_rxTimeout = rxTimeout;
}

Time
ConstantJammer::GetRxTimeout (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxTimeout;
}

void
ConstantJammer::SetReactToMitigation (const bool flag)
{
  NS_LOG_FUNCTION (this << flag);
  m_reactToMitigation = flag;
}

bool
ConstantJammer::GetReactToMitigation (void) const
{
  NS_LOG_FUNCTION (this);
  return m_reactToMitigation;
}

/*
 * Private functions start here.
 */

void
ConstantJammer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
ConstantJammer::DoStopJammer (void)
{
  NS_LOG_FUNCTION (this);
  m_jammingEvent.Cancel ();
}

void
ConstantJammer::DoJamming (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_utility != NULL);

  if (!IsJammerOn ()) // check if jammer is on
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () << ", Jammer is OFF!");
      return;
    }

  NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                ", Sending jamming signal with TX power = " << m_txPower <<
                " W" << ", At " << Simulator::Now ().GetSeconds () << "s");

  // send jamming signal
  double actualPower = m_utility->SendJammingSignal (m_txPower, m_jammingDuration);
  if (actualPower != 0.0)
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", Jamming signal sent with power = " << actualPower << " W");
    }
  else
    {
      NS_LOG_ERROR ("ConstantJammer:At Node #" << GetId () <<
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
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", After jammer starts, scheduling RX timeout!");
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &ConstantJammer::RxTimeoutHandler,
                                              this);
    }

  m_reacting = false; // always reset reacting flag
}

bool
ConstantJammer::DoStartRxHandler (Ptr<Packet> packet, double startRss)
{
  NS_LOG_FUNCTION (this << packet << startRss);

  if (m_reactToMitigation)  // check if react to mitigation is enabled
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", React to mitigation enabled! Rescheduling RX Timeout");
      // cancel RX timeout event
      m_rxTimeoutEvent.Cancel ();
      // reschedule next RX timeout
      m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                              &ConstantJammer::RxTimeoutHandler,
                                              this);
    }
  else
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", React to mitigation disabled!");
    }
  return false;
}

bool
ConstantJammer::DoEndRxHandler (Ptr<Packet> packet, double averageRss)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                ", Ignoring incoming packet!");
  return false;
}

void
ConstantJammer::DoEndTxHandler (Ptr<Packet> packet, double txPower)
{
  NS_LOG_FUNCTION (this << packet << txPower);
  NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                ". Sent jamming burst with power = " << txPower);

  m_jammingEvent.Cancel (); // cancel previously scheduled event

  // check to see if we are waiting the jammer to finish reacting to mitigation
  if (m_reacting)
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", Not sending jamming signal, jammer reacting to mitigation!");
      // get channel switch delay
      Time delay = m_utility->GetPhyLayerInfo().channelSwitchDelay;
      // schedule next jamming burst after channel switch delay
      m_jammingEvent = Simulator::Schedule (delay,
                                            &ConstantJammer::DoJamming,
                                            this);
      return;
    }

  // schedule next jamming burst after jamming interval.
  m_jammingEvent = Simulator::Schedule (m_constantJammingInterval,
                                        &ConstantJammer::DoJamming, this);
}

void
ConstantJammer::RxTimeoutHandler (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () << ", RX timeout at " <<
                Simulator::Now ().GetSeconds () << "s");
  NS_ASSERT (m_utility != NULL);

  if (!m_reactToMitigation) // check react to mitigation flag
    {
      NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                    ", React to mitigation is turned OFF!");
      return;
    }

  // Calculate channel number.
  uint16_t currentChannel = m_utility->GetPhyLayerInfo ().currentChannel;
  uint16_t nextChannel = currentChannel + 1;
  if (nextChannel >= m_utility->GetPhyLayerInfo ().numOfChannels)
    {
      nextChannel = 1;  // wrap around and start form 1
    }

  NS_LOG_DEBUG ("ConstantJammer:At Node #" << GetId () <<
                ", Switching from channel " << currentChannel << " >-> " <<
                nextChannel << ", At " << Simulator::Now ().GetSeconds () << "s");

  // hop to next channel
  m_utility->SwitchChannel (nextChannel);

  /*
   * Set reacting flag to indicate jammer is reacting. When the flag is set, no
   * scheduling of new jamming event is allowed.
   */
  m_reacting = true;

  // cancel previously scheduled RX timeout
  m_rxTimeoutEvent.Cancel ();

  // schedule next RX timeout
  m_rxTimeoutEvent = Simulator::Schedule (m_rxTimeout,
                                          &ConstantJammer::RxTimeoutHandler,
                                          this);
}

} // namespace ns3
  
