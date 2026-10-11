
/*
  ErrorHandling.ino
  Logs team events, tracks invalid requests, and monitors memory.
*/

#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

extern WebServer server;
extern int getTeamIndex(String ip);

#define MAX_TRACKED_CLIENTS 10
#define MAX_INVALID_REQUESTS 3

String trackedIPs[MAX_TRACKED_CLIENTS];
int invalidCounts[MAX_TRACKED_CLIENTS];
int nextSlot = 0;

// Find a client or allocate a tracking slot.
int getClientSlot(String ip) {
  for (int i = 0; i < MAX_TRACKED_CLIENTS; i++) {
    if (trackedIPs[i] == ip && ip.length() > 0) {
      return i;
    }
  }

  for (int i = 0; i < MAX_TRACKED_CLIENTS; i++) {
    if (trackedIPs[i].length() == 0) {
      trackedIPs[i] = ip;
      invalidCounts[i] = 0;
      return i;
    }
  }

  // Replace an old slot if all slots are occupied.
  int slot = nextSlot;
  nextSlot = (nextSlot + 1) % MAX_TRACKED_CLIENTS;

  trackedIPs[slot] = ip;
  invalidCounts[slot] = 0;

  return slot;
}

// Identify the team using this request.
const char* getLoggingTeamID() {
  static String teamID;

  String ip = server.client().remoteIP().toString();
  int teamIndex = getTeamIndex(ip);

  if (teamIndex >= 0) {
    teamID = teams[teamIndex].teamId;
    return teamID.c_str();
  }

  return "UNKNOWN_TEAM";
}

// Log event and memory statistics.
void logEvent(const char* eventName) {
  String ip = server.client().remoteIP().toString();

  Serial.printf(
    "[Team: %s] %s | IP: %s | Free heap: %u | Minimum free heap: %u\n",
    getLoggingTeamID(),
    eventName,
    ip.c_str(),
    (unsigned int)ESP.getFreeHeap(),
    (unsigned int)ESP.getMinFreeHeap()
  );
}

// Clear invalid request count after successful login.
void resetInvalidAccessCount(String ip) {
  for (int i = 0; i < MAX_TRACKED_CLIENTS; i++) {
    if (trackedIPs[i] == ip) {
      invalidCounts[i] = 0;
      return;
    }
  }
}

// Handle an invalid request.
// Restart after more than 3 invalid requests from one IP.
void handleInvalidAccess(
  const char* reason,
  int statusCode,
  const char* message
) {
  String ip = server.client().remoteIP().toString();
  int slot = getClientSlot(ip);

  invalidCounts[slot]++;

  logEvent(reason);

  Serial.printf(
    "[Security] Invalid request %d/%d from %s\n",
    invalidCounts[slot],
    MAX_INVALID_REQUESTS,
    ip.c_str()
  );

  // Limit reached: send the restart page instead of the normal error, then reboot
  if (invalidCounts[slot] >= MAX_INVALID_REQUESTS) {
  Serial.println("[Security] Too many invalid requests. Restarting ESP32...");
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", login_html);
  server.client().flush();
  delay(1000);
  ESP.restart();
  return;
}

  server.send(statusCode, "text/plain", message);
}

// Browser events such as "Game completed".
void handleBrowserEventRoute() {
  String eventName = server.arg("event");

  if (eventName.length() == 0) {
    eventName = "Unspecified browser event";
  }

  logEvent(eventName.c_str());
  server.send(204, "text/plain", "");
}

// Unknown URL.
void handleUnknownRoute() {
  handleInvalidAccess(
    "Unknown URL requested",
    404,
    "Page not found"
  );
}

// Memory statistics.
void printHeapStats(const char* label) {
  Serial.printf(
    "[Memory] %s | Free heap: %u | Minimum free heap: %u\n",
    label,
    (unsigned int)ESP.getFreeHeap(),
    (unsigned int)ESP.getMinFreeHeap()
  );
}
