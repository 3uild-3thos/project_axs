#ifndef SYSVAR_H
#define SYSVAR_H

#include <vector>
#include <algorithm>
#include <array>
#include <iostream>
#include "solana/public_key.h"
#include "clock.h"
#include "epoch_schedule.h"
#include "instructions.h"
#include "recent_blockhashes.h"
#include "rent.h"
#include "rewards.h"
#include "slot_hashes.h"
#include "slot_history.h"
#include "stake_history.h"

namespace sysvar
{
  using ::Clock;
  using ::EpochSchedule;
  using ::Instructions;
  using ::RecentBlockhashes;
  using ::Rent;
  using ::Rewards;
  using ::SlotHashes;
  using ::SlotHistory;
  using ::StakeHistory;

  const std::vector<PublicKey> ALL_IDS = {
      sysvar::Clock::id(),
      sysvar::EpochSchedule::id(),
      sysvar::Instructions::id(),
      sysvar::Rent::id(),
      sysvar::RecentBlockhashes::id(),
      sysvar::Rent::id(),
      sysvar::Rewards::id(),
      sysvar::SlotHashes::id(),
      sysvar::SlotHistory::id(),
      sysvar::StakeHistory::id(),
  };

  static bool isSysvarId(const PublicKey &id)
  {
    return std::find(sysvar::ALL_IDS.begin(), sysvar::ALL_IDS.end(), id) != sysvar::ALL_IDS.end();
  }
}

#endif // SYSVAR_H