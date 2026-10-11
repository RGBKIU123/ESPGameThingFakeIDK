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
  { "TEAM02", "pass02", "FLAG{CODE_BREAKER_MKX}", "GOAL", "projectcell", "room 301" },
  { "TEAM03", "pass03", "FLAG{CODE_BREAKER_PLW}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" },
  { "TEAM04", "pass04", "FLAG{CODE_BREAKER_QHZ}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" },
  { "TEAM05", "pass05", "FLAG{CODE_BREAKER_VBY}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" },
  { "TEAM06", "pass06", "FLAG{CODE_BREAKER_ZFR}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" },
  { "TEAM07", "pass07", "FLAG{CODE_BREAKER_NJG}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" },
  { "TEAM08", "pass08", "FLAG{CODE_BREAKER_VBN}", "ice", "projectcell", "You're in halls where grown minds learn, Yet your next path takes a backward turn. Where uniforms once walked in line, That's where your next sign will shine" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
