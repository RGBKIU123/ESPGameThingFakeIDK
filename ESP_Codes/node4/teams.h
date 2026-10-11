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
  { "TEAM01", "pass01", "FLAG{NETWORK_MAPPED_WXK}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM02", "pass02", "FLAG{NETWORK_MAPPED_QMP}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM03", "pass03", "FLAG{NETWORK_MAPPED_RLZ}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM04", "pass04", "FLAG{NETWORK_MAPPED_VHY}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM12", "pass12", "FLAG{NETWORK_MAPPED_BFN}", "GOAL", "projectcell", "room 301" },
  { "TEAM13", "pass13", "FLAG{NETWORK_MAPPED_GJM}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM14", "pass14", "FLAG{NETWORK_MAPPED_HSY}", "metal", "projectcell", "Beside where hunger meets its cure, A roomy space stands calm and pure. Step right, not left, past laughter's wall — There lies your clue, where voices fall." },
  { "TEAM15", "pass15", "FLAG{NETWORK_MAPPED_YPB}", "wood", "projectcell", "Where silence and knowledge fills the void." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
