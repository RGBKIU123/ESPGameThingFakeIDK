#ifndef TEAMS_H
#define TEAMS_H

struct TeamData {
  const char* teamId;
  const char* password;
  const char* flag;
  const char* nextWifi;
  const char* nextWifiPassword;
  const char* clue;
};

// Define team data - generated from flag_list.txt
const TeamData teams[] = {
  { "TEAM04", "pass04", "FLAG{FILE_DETECTIVE_XMK}", "GOAL", "projectcell", "room 301" },
  { "TEAM05", "pass05", "FLAG{FILE_DETECTIVE_PWL}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM06", "pass06", "FLAG{FILE_DETECTIVE_QHZ}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM07", "pass07", "FLAG{FILE_DETECTIVE_VBY}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM08", "pass08", "FLAG{FILE_DETECTIVE_ZFR}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM09", "pass09", "FLAG{FILE_DETECTIVE_NJG}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM10", "pass10", "FLAG{FILE_DETECTIVE_JKL}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." },
  { "TEAM15", "pass15", "FLAG{FILE_DETECTIVE_ZKH}", "shadow", "projectcell", "Step to the side where echoes fall, The farthest mark will tell it all. When dribbles fade and sneakers pause, The corner holds your hidden cause." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
