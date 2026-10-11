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
  { "TEAM01", "pass01", "FLAG{H1DD3N_HUNT3R_KWX}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM02", "pass02", "FLAG{H1DD3N_HUNT3R_PMQ}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM03", "pass03", "FLAG{H1DD3N_HUNT3R_RLZ}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM04", "pass04", "FLAG{H1DD3N_HUNT3R_VHY}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM05", "pass05", "FLAG{H1DD3N_HUNT3R_BFN}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM06", "pass06", "FLAG{H1DD3N_HUNT3R_GJM}", "void", "projectcell", "The vertical box that pauses between floors — check where it opens on the second." },
  { "TEAM14", "pass14", "FLAG{H1DD3N_HUNT3R_MDR}", "GOAL", "projectcell", "room 301" },
  { "TEAM15", "pass15", "FLAG{H1DD3N_HUNT3R_ZQW}", "GOAL", "projectcell", "room 301" }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
