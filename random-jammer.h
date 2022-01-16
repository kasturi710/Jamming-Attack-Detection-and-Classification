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

#ifndef RANDOM_JAMMER_H
#define RANDOM_JAMMER_H

#include "jammer.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

namespace ns3 {

/**
 * Random Jammer.
 */
class RandomJammer : public Jammer
{
public:
  static TypeId GetTypeId (void);
  RandomJammer ();
  virtual ~RandomJammer ();

  /**
   * \brief Sets pointer to WirelessModuleUtility installed on node..
   *
   * \param utility Pointer to WirelessModuleUtility.
   */
  virtual void SetUtility (Ptr<WirelessModuleUtility> utility);

  /**
   * \brief Sets pointer to energy source.
   *
   * \param energySrcPtr Pointer to EnergySource installed on node.
   *
   * This function is called by JammerHelper.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source);

  // setter & getters of attributes
  void SetTxPower (double power);
  double GetTxPower (void) const;
  void SetJammingDuration (Time duration);
  Time GetJammingDuration (void) const;
  void SetRandomVariable (RandomVariable random);
  void SetRxTimeout (Time rxTimeout);
  Time GetRxTimeout (void) const;
  void SetReactToMitigation (const bool flag);
  bool GetReactToMitigation (void) const;

private:
  void DoDispose (void);

  /**
   * Stops jammer.
   */
  virtual void DoStopJammer (void);

  /**
   * Sends out jamming burst at random interval.
   */
  virtual void DoJamming (void);

  /**
   * \brief Handles start RX event.
   *
   * \param packet Pointer to incoming packet.
   * \param startRss Start RSS of packet.
   * \return False. Constant jammer will *always* ignore incoming packets.
   */
  virtual bool DoStartRxHandler (Ptr<Packet> packet, double startRss);

  /**
   * \brief Handles incoming packet.
   *
   * \param packet Pointer to incoming packet.
   * \param averageRss Average RSS of packet
   * \return False. Constant jammer will *always* ignore incoming packets.
   */
  virtual bool DoEndRxHandler (Ptr<Packet> packet, double averageRss);

  /**
   * \brief Notifies jammer of end of sending jamming signal.
   *
   * \param packet Pointer to dummy packet that was sent.
   * \param txPower Transmit power of packet.
   *
   * For random jammer, it schedules the next jamming burst in this function.
   */
  virtual void DoEndTxHandler (Ptr<Packet> packet, double txPower);

  /**
   * Handles RX timeout event by hopping onto next channel.
   */
  void RxTimeoutHandler (void);

private:
  Ptr<WirelessModuleUtility> m_utility;
  Ptr<EnergySource> m_source;
  RandomVariable m_randomJammingInterval; // random variable to use
  double m_txPower;           // TX power
  Time m_jammingDuration;     // jamming duration
  EventId m_jammingEvent;     // jamming event
  Time m_rxTimeout;           // RX timeout interval
  EventId m_rxTimeoutEvent;   // RX timeout event
  bool m_reactToMitigation;   // true if jammer is reacting to mitigation
  bool m_reacting;    // flag indicating jammer is reacting to mitigation

};  // class RandomJammer

} // namespace ns3

#endif  /* RANDOM_JAMMER_H */
