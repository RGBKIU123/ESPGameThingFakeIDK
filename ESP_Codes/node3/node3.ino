#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"
#include "login.h"

// Access Point credentials
const char* ssid = "earth";
const char* password = "projectcell";

WebServer server(80);
/*new*/ 
const char* getLoggingTeamID();
void logEvent(const char* eventName);
void sendHttpError(int statusCode, const char* message);
void redirectWithLog(const char* reason, const char* destination);
void handleBrowserEventRoute();
void handleUnknownRoute();
void printHeapStats(const char* label);

void handleInvalidAccess(
  const char* reason,
  int statusCode,
  const char* message
);

void resetInvalidAccessCount(String ip);
/*end of new*/

// Session management - stores team index for each IP address
String authenticatedIPs[10]; // Support up to 10 simultaneous sessions
int teamIndexForIP[10];
int sessionCount = 0;

// Game page HTML - now uses placeholders for team-specific data
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Memory Matrix - Challenge 14</title>
    <style>
        body {
            background: linear-gradient(135deg, #1a1a3a, #3a1a3a);
            color: #00ff88;
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .container {
            max-width: 95vw;
            width: 100%;
            text-align: center;
            padding: 0 10px;
        }

        h1 {
            text-shadow: 0 0 15px #00ff88;
            margin-bottom: 20px;
            color: #00ff88;
            font-size: clamp(1.5rem, 4vw, 2.5rem);
        }

        .memory-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 8px;
            justify-content: center;
            margin: 20px auto;
            background: #001133;
            padding: 20px;
            border: 2px solid #00ff88;
            border-radius: 15px;
            box-shadow: 0 0 30px rgba(0, 255, 136, 0.3);
            max-width: 350px;
        }

        .memory-cell {
            width: 100%;
            height: 70px;
            min-height: 60px;
            border: 2px solid #004422;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            position: relative;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 24px;
            font-weight: bold;
            touch-action: manipulation;
            background: #002211;
        }

        .memory-cell.hidden {
            background: #002211;
            border-color: #004422;
        }

        .memory-cell.revealed {
            background: #004433;
            border-color: #00ff88;
            box-shadow: 0 0 10px #00ff88;
        }

        .memory-cell.flashing {
            background: #00ff88;
            color: #000;
            border-color: #88ffaa;
            box-shadow: 0 0 20px #00ff88;
            animation: flash-pulse 0.5s;
        }

        .memory-cell.correct {
            background: #006600;
            border-color: #00ff00;
            box-shadow: 0 0 15px #00ff00;
            animation: success-glow 0.5s;
        }

        .memory-cell.wrong {
            background: #660000;
            border-color: #ff0000;
            box-shadow: 0 0 15px #ff0000;
            animation: error-shake 0.5s;
        }

        @keyframes flash-pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }

        @keyframes success-glow {
            0%, 100% { box-shadow: 0 0 15px #00ff00; }
            50% { box-shadow: 0 0 25px #00ff00; }
        }

        @keyframes error-shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-5px); }
            75% { transform: translateX(5px); }
        }

        .game-info {
            background: #001133;
            border: 2px solid #00ff88;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 15px;
        }

        .info-box {
            text-align: center;
        }

        .info-value {
            font-size: 24px;
            font-weight: bold;
            color: #88ffaa;
            margin-bottom: 5px;
        }

        .info-label {
            font-size: 12px;
            color: #00ff88;
        }

        .pattern-display {
            background: #002211;
            border: 1px solid #00ff88;
            padding: 15px;
            border-radius: 8px;
            margin: 15px 0;
            font-size: 16px;
            min-height: 50px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        button {
            background: linear-gradient(45deg, #00ff88, #00aa66);
            border: none;
            color: black;
            padding: 15px 20px;
            font-size: clamp(14px, 3vw, 16px);
            cursor: pointer;
            border-radius: 8px;
            font-weight: bold;
            margin: 8px;
            transition: all 0.3s;
            touch-action: manipulation;
            min-height: 44px;
        }

        button:hover, button:active {
            transform: scale(1.05);
            box-shadow: 0 0 15px #00ff88;
        }

        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }

        .controls {
            margin: 20px 0;
        }

        .status {
            font-size: 18px;
            margin: 20px 0;
            min-height: 48px;
            line-height: 1.4;
            background: #001133;
            border: 1px solid #00ff88;
            border-radius: 10px;
            padding: 15px;
        }

        .flag {
            background: #001133;
            border: 2px solid #00ff88;
            padding: 20px;
            border-radius: 15px;
            margin-top: 30px;
            font-size: 20px;
            font-weight: bold;
            display: none;
            box-shadow: 0 0 30px rgba(0, 255, 136, 0.5);
        }

        .instructions {
            background: #002211;
            border: 1px solid #00ff88;
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
            font-size: 14px;
            line-height: 1.6;
        }

        .level-indicator {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin: 15px 0;
        }

        .level-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #333366;
            border: 1px solid #00ff88;
            transition: all 0.3s;
        }

        .level-dot.active {
            background: #00ff88;
            box-shadow: 0 0 10px #00ff88;
        }

        .level-dot.completed {
            background: #66ff66;
            border-color: #66ff66;
        }

        .logout{position:absolute;top:20px;right:20px;background:#880000;color:#ff0000;padding:8px 15px;border-radius:5px;text-decoration:none;font-size:14px}

        @media (max-width: 768px) {
            body {
                padding: 10px;
            }

            .instructions {
                font-size: 12px;
            }

            .game-info {
                grid-template-columns: repeat(2, 1fr);
                gap: 10px;
                font-size: 12px;
            }

            .controls {
                display: flex;
                flex-direction: column;
                gap: 10px;
            }

            button {
                width: 100%;
                margin: 5px 0;
            }

            .info-value {
                font-size: 20px;
            }
        }

        .game-phase {
            font-size: 18px;
            font-weight: bold;
            color: #88ffaa;
            margin: 15px 0;
            padding: 10px;
            background: #002211;
            border-radius: 8px;
            border: 1px solid #00ff88;
        }
    </style>
</head>
<body>
    <a href="/logout" class="logout">Logout</a>
    <div class="container">
        <h1>MEMORY MATRIX</h1>
        <p>Test your memory by following the flashing patterns!</p>

        <div class="level-indicator">
            <div class="level-dot active" id="level1"></div>
            <div class="level-dot" id="level2"></div>
            <div class="level-dot" id="level3"></div>
            <div class="level-dot" id="level4"></div>
            <div class="level-dot" id="level5"></div>
        </div>

        <div class="instructions">
            <strong>MEMORY PROTOCOL:</strong><br>
            • Watch the sequence of flashing cells carefully<br>
            • Click the cells in the exact same order<br>
            • Each level adds one more step to remember<br>
            • Complete all 5 levels to master your memory!
        </div>

        <div class="game-phase" id="gamePhase">Ready to start...</div>

        <div class="memory-grid" id="memoryGrid"></div>

        <div class="game-info">
            <div class="info-box">
                <div class="info-value" id="currentLevel">1</div>
                <div class="info-label">Level</div>
            </div>
            <div class="info-box">
                <div class="info-value" id="sequenceLength">3</div>
                <div class="info-label">Sequence</div>
            </div>
            <div class="info-box">
                <div class="info-value" id="currentStep">0</div>
                <div class="info-label">Step</div>
            </div>
            <div class="info-box">
                <div class="info-value" id="score">0</div>
                <div class="info-label">Score</div>
            </div>
        </div>

        <div class="pattern-display" id="patternDisplay">
            Click "Start Sequence" to begin the memory challenge
        </div>

        <div class="controls">
            <button onclick="startSequence()" id="startBtn">Start Sequence</button>
            <button onclick="resetGame()">Reset Game</button>
            <button onclick="showHint()">Memory Hint</button>
        </div>

        <div class="status" id="status">Watch the flashing pattern, then repeat it by clicking the cells...</div>

        <div class="flag" id="flag">
            Memory master achieved! Flag: <span style="color: #ffff00;">{{FLAG}}</span>
            <br><br>
            Your neural pathways have been optimized. Perfect recall achieved across all memory matrices.
            <br><br>
            Next WiFi: <span style="color: #88ffaa;">{{NEXT_WIFI}}</span> | Password: <span style="color: #88ffaa;">{{WIFI_PASS}}</span>
            <br><br>
            Clue: <span style="color: #ffaa00;">{{CLUE}}</span>
        </div>
    </div>

    <script>
    /*new*/ 
function reportEvent(eventName) {
    fetch('/event', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: 'event=' + encodeURIComponent(eventName)
    }).catch(error => {
        console.error('Could not report event:', error);
    });
}
/*end of new*/
        let currentLevel = 1;
        let maxLevel = 5;
        let sequence = [];
        let playerSequence = [];
        let isShowingSequence = false;
        let isPlayerTurn = false;
        let currentStep = 0;
        let score = 0;
        let gamePhase = 'ready';

        const symbols = ['🔥', '⚡', '💎', '🌟', '🔮', '⭐', '💠', '🎯', '🔷', '💫', '✨', '🌈', '🎪', '🎭', '🎨', '🎼'];

        function initializeGrid() {
            const gridElement = document.getElementById('memoryGrid');
            gridElement.innerHTML = '';

            for (let i = 0; i < 16; i++) {
                const cell = document.createElement('div');
                cell.className = 'memory-cell hidden';
                cell.id = `cell-${i}`;
                cell.onclick = () => handleCellClick(i);
                cell.textContent = symbols[i];
                gridElement.appendChild(cell);
            }

            updateDisplay();
        }

        function startSequence() {
            if (isShowingSequence || isPlayerTurn) return;

            gamePhase = 'showing';
            document.getElementById('gamePhase').textContent = 'Watch the sequence...';
            document.getElementById('startBtn').disabled = true;

            sequence = [];
            const sequenceLength = 2 + currentLevel;

            for (let i = 0; i < sequenceLength; i++) {
                sequence.push(Math.floor(Math.random() * 16));
            }

            playerSequence = [];
            currentStep = 0;
            isShowingSequence = true;

            showSequenceStep(0);
            updateDisplay();
        }

        function showSequenceStep(index) {
            if (index >= sequence.length) {
                isShowingSequence = false;
                isPlayerTurn = true;
                gamePhase = 'player';
                document.getElementById('gamePhase').textContent = 'Your turn - repeat the sequence!';
                document.getElementById('patternDisplay').textContent = 'Click the cells in the same order you saw them flash';
                return;
            }

            const cellId = sequence[index];
            const cell = document.getElementById(`cell-${cellId}`);

            cell.classList.add('flashing');

            setTimeout(() => {
                cell.classList.remove('flashing');

                setTimeout(() => {
                    showSequenceStep(index + 1);
                }, 300);
            }, 600);
        }

        function handleCellClick(cellId) {
            if (!isPlayerTurn || isShowingSequence) return;

            const cell = document.getElementById(`cell-${cellId}`);
            playerSequence.push(cellId);

            if (cellId === sequence[currentStep]) {
                cell.classList.add('correct');
                setTimeout(() => cell.classList.remove('correct'), 500);

                currentStep++;
                score += 10;

                if (currentStep === sequence.length) {
                    setTimeout(() => {
                        completeLevel();
                    }, 500);
                }
            } else {
                cell.classList.add('wrong');
                setTimeout(() => cell.classList.remove('wrong'), 500);

                gamePhase = 'failed';
                document.getElementById('gamePhase').textContent = 'Wrong sequence! Try again.';
                document.getElementById('patternDisplay').textContent = 'Click "Start Sequence" to try this level again';

                isPlayerTurn = false;
                document.getElementById('startBtn').disabled = false;

                setTimeout(() => {
                    highlightCorrectSequence();
                }, 800);
            }

            updateDisplay();
        }

        function highlightCorrectSequence() {
            sequence.forEach((cellId, index) => {
                setTimeout(() => {
                    const cell = document.getElementById(`cell-${cellId}`);
                    cell.classList.add('revealed');

                    setTimeout(() => {
                        cell.classList.remove('revealed');
                    }, 400);
                }, index * 200);
            });
        }

        function completeLevel() {
            score += currentLevel * 50;

            if (currentLevel === maxLevel) {
               /*new*/ reportEvent('Game completed');
                gamePhase = 'complete';
                document.getElementById('gamePhase').textContent = 'All levels completed!';
                document.getElementById('status').innerHTML =
                    '<strong style="color: #66ff66;">MEMORY MATRIX MASTERED!</strong><br>Perfect memory recall achieved!';
                document.getElementById('flag').style.display = 'block';
                document.getElementById('startBtn').disabled = true;
            } else {
                gamePhase = 'levelComplete';
                document.getElementById('gamePhase').textContent = `Level ${currentLevel} Complete!`;
                document.getElementById('patternDisplay').textContent = 'Great job! Moving to next level...';

                document.getElementById(`level${currentLevel}`).className = 'level-dot completed';
                currentLevel++;
                if (currentLevel <= maxLevel) {
                    document.getElementById(`level${currentLevel}`).className = 'level-dot active';
                }

                setTimeout(() => {
                    gamePhase = 'ready';
                    document.getElementById('gamePhase').textContent = 'Ready for next level...';
                    document.getElementById('patternDisplay').textContent = 'Click "Start Sequence" to begin the next challenge';
                    isPlayerTurn = false;
                    document.getElementById('startBtn').disabled = false;
                    updateDisplay();
                }, 2000);
            }

            updateDisplay();
        }

        function updateDisplay() {
            document.getElementById('currentLevel').textContent = currentLevel;
            document.getElementById('sequenceLength').textContent = sequence.length || (2 + currentLevel);
            document.getElementById('currentStep').textContent = currentStep;
            document.getElementById('score').textContent = score;

            const statusMessages = {
                'ready': 'Click "Start Sequence" to begin the memory challenge',
                'showing': 'Watch carefully! Remember the flashing pattern...',
                'player': `Click cell ${currentStep + 1} of ${sequence.length} in the sequence`,
                'failed': 'Incorrect! Study the correct sequence and try again.',
                'levelComplete': `Level ${currentLevel - 1} mastered! Preparing next challenge...`,
                'complete': 'All memory matrices conquered! Neural optimization complete.'
            };

            document.getElementById('status').textContent = statusMessages[gamePhase] || 'Memory training in progress...';
        }

        function resetGame() {
            currentLevel = 1;
            sequence = [];
            playerSequence = [];
            isShowingSequence = false;
            isPlayerTurn = false;
            currentStep = 0;
            score = 0;
            gamePhase = 'ready';

            for (let i = 1; i <= maxLevel; i++) {
                document.getElementById(`level${i}`).className = 'level-dot';
            }
            document.getElementById('level1').className = 'level-dot active';

            document.getElementById('startBtn').disabled = false;
            document.getElementById('flag').style.display = 'none';
            document.getElementById('gamePhase').textContent = 'Ready to start...';
            document.getElementById('patternDisplay').textContent = 'Click "Start Sequence" to begin the memory challenge';

            const cells = document.querySelectorAll('.memory-cell');
            cells.forEach(cell => {
                cell.className = 'memory-cell hidden';
            });

            updateDisplay();
        }

        function showHint() {
            const hints = [
                "Focus on the symbols - each cell has a unique icon to help you remember!",
                "Try to create a mental story connecting the symbols in order.",
                "Look for patterns - sometimes adjacent cells flash in sequence.",
                "Take your time during your turn - there's no time limit for clicking!",
                "If you fail, watch the correct sequence replay to learn the pattern."
            ];

            const randomHint = hints[Math.floor(Math.random() * hints.length)];
            document.getElementById('status').innerHTML = `<strong>Memory Hint:</strong><br>${randomHint}`;
        }

        initializeGrid();
    </script>
</body>
</html>
)rawliteral";

// Helper function to find team index by IP
int getTeamIndex(String ip) {
  for (int i = 0; i < sessionCount; i++) {
    if (authenticatedIPs[i] == ip) {
      return teamIndexForIP[i];
    }
  }
  return -1;
}

// Helper function to authenticate team
int authenticateTeam(String teamId, String pass) {
  for (int i = 0; i < NUM_TEAMS; i++) {
    if (String(teams[i].teamId) == teamId && String(teams[i].password) == pass) {
      return i;
    }
  }
  return -1;
}

// Helper function to add authenticated session
void addSession(String ip, int teamIdx) {
  // Check if IP already exists, update it
  for (int i = 0; i < sessionCount; i++) {
    if (authenticatedIPs[i] == ip) {
      teamIndexForIP[i] = teamIdx;
      return;
    }
  }

  // Add new session if space available
  if (sessionCount < 10) {
    authenticatedIPs[sessionCount] = ip;
    teamIndexForIP[sessionCount] = teamIdx;
    sessionCount++;
  }
}

// Helper function to remove session
void removeSession(String ip) {
  for (int i = 0; i < sessionCount; i++) {
    if (authenticatedIPs[i] == ip) {
      // Shift remaining sessions
      for (int j = i; j < sessionCount - 1; j++) {
        authenticatedIPs[j] = authenticatedIPs[j + 1];
        teamIndexForIP[j] = teamIndexForIP[j + 1];
      }
      sessionCount--;
      return;
    }
  }
}

// Route: Login page
void handleRoot() {
    /*new*/ logEvent("Login page requested");
  server.send_P(200, "text/html", login_html);
}

// Route: Handle login POST
void handleLogin() {
    /*new*/ logEvent("Login page requested");
    if (server.method() != HTTP_POST) {
    handleInvalidAccess(
        "Invalid method on /login",
        405,
        "Method Not Allowed"
    );
    return;
    }

  String teamId = server.arg("teamId");
  String pass = server.arg("password");

  int teamIdx = authenticateTeam(teamId, pass);

if (teamIdx >= 0) {
  String clientIP = server.client().remoteIP().toString();

  addSession(clientIP, teamIdx);
  resetInvalidAccessCount(clientIP);

  logEvent("Login successful");
  server.send(200, "text/plain", "OK");
} else {
  logEvent("Login failed");
  server.send(200, "text/plain", "FAIL");
}
}
//c:\Users\Admin\Downloads\ErrorHandling.ino
// Route: Game page (requires authentication)
void handleGame() {
    /*new*/ logEvent("Game page requested");
  String clientIP = server.client().remoteIP().toString();
  int teamIdx = getTeamIndex(clientIP);

if (teamIdx < 0) {
  handleInvalidAccess(
    "Unauthenticated access to /game",
    403,
    "Please log in first."
  );
  return;
}
  // Get team data
  const TeamData& team = teams[teamIdx];

  // Replace placeholders with team-specific data
  String html = String(game_html);
  html.replace("{{FLAG}}", team.flag);
  html.replace("{{NEXT_WIFI}}", team.nextWifi);
  html.replace("{{WIFI_PASS}}", team.nextWifiPassword);
  html.replace("{{CLUE}}", team.clue);

  server.send(200, "text/html", html);
}

// Route: Logout
void handleLogout() {
  String clientIP = server.client().remoteIP().toString();

  if (getTeamIndex(clientIP) < 0) {
    // Not logged in: just send them to the login page
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
    return;
  }

  logEvent("Logout requested");
  removeSession(clientIP);

  // Send the "logging out" page and make sure it is fully delivered
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", login_html);
  server.client().flush();

  Serial.println("[System] Restarting ESP32 after logout...");
  delay(1000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);

  // Configure Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Route handlers
  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/game", handleGame);
  server.on("/logout", handleLogout);
 /*new*/ server.on("/event", HTTP_POST, handleBrowserEventRoute); server.onNotFound(handleUnknownRoute);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
