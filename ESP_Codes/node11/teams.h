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
  { "TEAM05", "pass05", "FLAG{PATTERN_SPOTTER_KWX}", "GOAL", "projectcell", "room 301" },
  { "TEAM06", "pass06", "FLAG{PATTERN_SPOTTER_PMQ}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM07", "pass07", "FLAG{PATTERN_SPOTTER_RLZ}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM08", "pass08", "FLAG{PATTERN_SPOTTER_VHY}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM09", "pass09", "FLAG{PATTERN_SPOTTER_BFN}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM10", "pass10", "FLAG{PATTERN_SPOTTER_GJM}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM11", "pass11", "FLAG{PATTERN_SPOTTER_VFY}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." },
  { "TEAM15", "pass15", "FLAG{PATTERN_SPOTTER_XLM}", "spirit", "projectcell", "Beyond the lot where mentors park, Awaits a space for names that mark. When guests arrive, both new and known, Their resting place stands just beyond." }
};

const int NUM_TEAMS = sizeof(teams) / sizeof(teams[0]);

#endif
