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
  { "TEAM01", "pass01", "FLAG{MEM0RY_M4ST3R_XKZ}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" },
  { "TEAM09", "pass09", "FLAG{MEM0RY_M4ST3R_TKP}", "GOAL", "projectcell", "room 301" },
  { "TEAM10", "pass10", "FLAG{MEM0RY_M4ST3R_QWM}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" },
  { "TEAM11", "pass11", "FLAG{MEM0RY_M4ST3R_PLR}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" },
  { "TEAM12", "pass12", "FLAG{MEM0RY_M4ST3R_VBN}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" },
  { "TEAM13", "pass13", "FLAG{MEM0RY_M4ST3R_ZHY}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" },
  { "TEAM14", "pass14", "FLAG{MEM0RY_M4ST3R_FGJ}", "water", "projectcell", "Where walls listen to prayers instead of words. I quench the thirst of those who pass by" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
