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
  { "TEAM06", "pass06", "FLAG{MEMORY_CHAMPION_XMK}", "GOAL", "projectcell", "room 301" },
  { "TEAM07", "pass07", "FLAG{MEMORY_CHAMPION_PWL}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM08", "pass08", "FLAG{MEMORY_CHAMPION_QHZ}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM09", "pass09", "FLAG{MEMORY_CHAMPION_VBY}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM10", "pass10", "FLAG{MEMORY_CHAMPION_ZFR}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM11", "pass11", "FLAG{MEMORY_CHAMPION_NJG}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM12", "pass12", "FLAG{MEMORY_CHAMPION_YHB}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." },
  { "TEAM15", "pass15", "FLAG{MEMORY_CHAMPION_QWN}", "ether", "projectcell", "No crown, no throne, yet all obey, His steed rests here at break of day. Metal sleeps where duty starts — Seek the shelter of the leader's cart." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
