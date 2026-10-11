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
  { "TEAM07", "pass07", "FLAG{VIRUS_HUNTER_KWX}", "GOAL", "projectcell", "room 301" },
  { "TEAM08", "pass08", "FLAG{VIRUS_HUNTER_PMQ}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM09", "pass09", "FLAG{VIRUS_HUNTER_RLZ}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM10", "pass10", "FLAG{VIRUS_HUNTER_VHY}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM11", "pass11", "FLAG{VIRUS_HUNTER_BFN}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM12", "pass12", "FLAG{VIRUS_HUNTER_GJM}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM13", "pass13", "FLAG{VIRUS_HUNTER_PLM}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." },
  { "TEAM15", "pass15", "FLAG{VIRUS_HUNTER_TRF}", "aether", "projectcell", "Where every sir and ma'am appear, Yet silence fills the air. Faces missing, names remain — Find the board that knows them all." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
