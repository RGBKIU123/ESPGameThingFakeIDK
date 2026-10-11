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
  { "TEAM01", "pass01", "FLAG{B1N4RY_D3C0D3R_MKX}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" },
  { "TEAM02", "pass02", "FLAG{B1N4RY_D3C0D3R_RWP}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" },
  { "TEAM10", "pass10", "FLAG{B1N4RY_D3C0D3R_LNZ}", "GOAL", "projectcell", "room 301" },
  { "TEAM11", "pass11", "FLAG{B1N4RY_D3C0D3R_QHY}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" },
  { "TEAM12", "pass12", "FLAG{B1N4RY_D3C0D3R_VFJ}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" },
  { "TEAM13", "pass13", "FLAG{B1N4RY_D3C0D3R_BGM}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" },
  { "TEAM14", "pass14", "FLAG{B1N4RY_D3C0D3R_WQZ}", "earth", "projectcell", "Of the three watchful homes, the one that keeps the guard close" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
