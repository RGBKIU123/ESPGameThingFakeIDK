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
  { "TEAM01", "pass01", "FLAG{C1PH3R_D3C0D3R_XMK}", "wood", "projectcell", "Where silence and knowledge fills the void." },
  { "TEAM02", "pass02", "FLAG{C1PH3R_D3C0D3R_PWL}", "wood", "projectcell", "Where silence and knowledge fills the void." },
  { "TEAM03", "pass03", "FLAG{C1PH3R_D3C0D3R_QHZ}", "wood", "projectcell", "Where silence and knowledge fills the void." },
  { "TEAM04", "pass04", "FLAG{C1PH3R_D3C0D3R_VBY}", "wood", "projectcell", "Where silence and knowledge fills the void." },
  { "TEAM05", "pass05", "FLAG{C1PH3R_D3C0D3R_ZFR}", "wood", "projectcell", "Where silence and knowledge fills the void." },
  { "TEAM13", "pass13", "FLAG{C1PH3R_D3C0D3R_NJG}", "GOAL", "projectcell", "Room 301" },
  { "TEAM14", "pass14", "FLAG{C1PH3R_D3C0D3R_XPK}", "wood", "projectcell", "Where silence and knowledge fills the void." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
