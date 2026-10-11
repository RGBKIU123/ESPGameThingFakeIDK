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

// Final node for all teams
const TeamData teams[] = {
  { "TEAM01", "pass01", "FLAG{ULTIMATE_CHAMPION_AQZ}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM02", "pass02", "FLAG{ULTIMATE_CHAMPION_BXR}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM03", "pass03", "FLAG{ULTIMATE_CHAMPION_CWT}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM04", "pass04", "FLAG{ULTIMATE_CHAMPION_DVS}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM05", "pass05", "FLAG{ULTIMATE_CHAMPION_EUY}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM06", "pass06", "FLAG{ULTIMATE_CHAMPION_FPN}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM07", "pass07", "FLAG{ULTIMATE_CHAMPION_GKM}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM08", "pass08", "FLAG{ULTIMATE_CHAMPION_HJL}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM09", "pass09", "FLAG{ULTIMATE_CHAMPION_JRV}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM10", "pass10", "FLAG{ULTIMATE_CHAMPION_KQT}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM11", "pass11", "FLAG{ULTIMATE_CHAMPION_LYS}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM12", "pass12", "FLAG{ULTIMATE_CHAMPION_MFB}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM13", "pass13", "FLAG{ULTIMATE_CHAMPION_NDG}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM14", "pass14", "FLAG{ULTIMATE_CHAMPION_PWC}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" },
  { "TEAM15", "pass15", "FLAG{ULTIMATE_CHAMPION_XRY}", "FINISH", "projectcell", "Congratulations! You have completed your treasure hunt journey!" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
