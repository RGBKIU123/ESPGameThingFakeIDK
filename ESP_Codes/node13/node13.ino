#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "ether";
const char* password = "projectcell";

WebServer server(80);

// Session management - stores team index for each IP address
String authenticatedIPs[10]; // Support up to 10 simultaneous sessions
int teamIndexForIP[10];
int sessionCount = 0;


// Login page HTML
const char PROGMEM login_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Team Login - Challenge 13</title>
<style>
body{background:linear-gradient(135deg,#0f0f23,#1a1a2e);color:#00ff00;font-family:'Courier New',monospace;margin:0;padding:20px;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center}
.login-container{max-width:400px;background:#001100;border:2px solid #00ff00;border-radius:10px;padding:30px;box-shadow:0 0 20px rgba(0,255,0,0.3)}
h1{text-shadow:0 0 10px #00ff00;margin-bottom:30px;text-align:center}
.form-group{margin-bottom:20px}
label{display:block;margin-bottom:8px;font-weight:bold}
input{width:100%;padding:10px;background:#0a0a0a;border:1px solid #00ff00;color:#00ff00;border-radius:5px;font-family:'Courier New',monospace;font-size:14px;box-sizing:border-box}
input:focus{outline:none;box-shadow:0 0 10px rgba(0,255,0,0.5)}
button{width:100%;background:linear-gradient(45deg,#00ff00,#008800);border:none;color:black;padding:12px;font-size:16px;cursor:pointer;border-radius:5px;font-weight:bold;margin-top:10px}
button:hover{background:linear-gradient(45deg,#00dd00,#006600)}
.error{color:#ff0000;text-align:center;margin-top:15px;display:none}

        .logout{position:absolute;top:20px;right:20px;background:#880000;color:#ff0000;padding:8px 15px;border-radius:5px;text-decoration:none;font-size:14px}
    </style>
</head>
<body>
    <a href="/logout" class="logout">Logout</a>
<div class="login-container">
<h1>🔐 TEAM LOGIN</h1>
<form id="loginForm" onsubmit="return login(event)">
<div class="form-group">
<label for="teamId">Team ID:</label>
<input type="text" id="teamId" name="teamId" required autocomplete="off">
</div>
<div class="form-group">
<label for="password">Password:</label>
<input type="password" id="password" name="password" required>
</div>
<button type="submit">LOGIN</button>
<div class="error" id="error">Invalid Team ID or Password!</div>
</form>
</div>
<script>
function login(e){e.preventDefault();const teamId=document.getElementById('teamId').value;const password=document.getElementById('password').value;fetch('/login',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'teamId='+encodeURIComponent(teamId)+'&password='+encodeURIComponent(password)}).then(r=>r.text()).then(result=>{if(result==='OK'){window.location.href='/game'}else{document.getElementById('error').style.display='block'}}).catch(err=>{console.error(err);document.getElementById('error').style.display='block'});return false}
</script>
</body>
</html>
)rawliteral";

// Game page HTML - Pattern Recognition Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Challenge 25: Pattern Recognition Game</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #0a0a0a 0%, #1a0a2e 100%);
            color: #00ff88;
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }

        .container {
            max-width: 900px;
            margin: 0 auto;
            background: rgba(0, 0, 0, 0.8);
            border: 2px solid #00ff88;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 0 30px rgba(0, 255, 136, 0.3);
        }

        h1 {
            text-align: center;
            color: #ff00ff;
            font-size: 2.5em;
            margin-bottom: 20px;
            text-shadow: 0 0 20px #ff00ff;
            animation: glow 2s ease-in-out infinite alternate;
        }

        @keyframes glow {
            from { text-shadow: 0 0 10px #ff00ff, 0 0 20px #ff00ff; }
            to { text-shadow: 0 0 20px #ff00ff, 0 0 30px #ff00ff, 0 0 40px #ff00ff; }
        }

        .instructions {
            background: rgba(138, 43, 226, 0.2);
            border: 2px solid #8a2be2;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 25px;
        }

        .instructions h2 {
            color: #8a2be2;
            margin-bottom: 10px;
        }

        .instructions p {
            color: #00ffff;
            line-height: 1.6;
            margin-bottom: 10px;
        }

        .target-display {
            background: rgba(255, 0, 255, 0.2);
            border: 2px solid #ff00ff;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            text-align: center;
        }

        .target-label {
            color: #ff00ff;
            font-size: 1.2em;
            margin-bottom: 10px;
        }

        .pattern {
            font-size: 3em;
            letter-spacing: 5px;
            margin: 10px 0;
        }

        .stats {
            display: flex;
            justify-content: space-around;
            margin-bottom: 20px;
            flex-wrap: wrap;
            gap: 15px;
        }

        .stat-box {
            background: rgba(255, 0, 255, 0.2);
            border: 2px solid #ff00ff;
            border-radius: 10px;
            padding: 15px 25px;
            text-align: center;
            min-width: 140px;
        }

        .stat-label {
            color: #ff00ff;
            font-size: 0.9em;
            margin-bottom: 5px;
        }

        .stat-value {
            color: #00ff88;
            font-size: 1.8em;
            font-weight: bold;
        }

        .log-container {
            background: rgba(0, 0, 0, 0.6);
            border: 2px solid #00ff88;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            min-height: 200px;
            max-height: 300px;
            overflow-y: auto;
        }

        .log-entry {
            background: rgba(138, 43, 226, 0.3);
            border: 2px solid #8a2be2;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 10px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            cursor: pointer;
            transition: all 0.3s;
            animation: slideIn 0.5s ease-out;
        }

        @keyframes slideIn {
            from {
                transform: translateX(-100%);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }

        .log-entry:hover {
            transform: translateY(-3px);
            box-shadow: 0 0 20px rgba(138, 43, 226, 0.6);
            border-color: #ff00ff;
        }

        .log-entry.correct {
            background: rgba(0, 255, 0, 0.2);
            border-color: #00ff00;
            animation: correctPulse 0.5s;
        }

        .log-entry.incorrect {
            background: rgba(255, 0, 0, 0.2);
            border-color: #ff0000;
            animation: shake 0.5s;
        }

        @keyframes correctPulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }

        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            75% { transform: translateX(10px); }
        }

        .log-pattern {
            font-size: 2em;
            letter-spacing: 3px;
        }

        .log-result {
            font-size: 1.5em;
        }

        .buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
        }

        button {
            background: linear-gradient(135deg, #8a2be2 0%, #ff00ff 100%);
            color: white;
            border: none;
            padding: 15px 30px;
            font-size: 1.1em;
            border-radius: 10px;
            cursor: pointer;
            font-family: 'Courier New', monospace;
            transition: all 0.3s;
            box-shadow: 0 0 15px rgba(138, 43, 226, 0.5);
        }

        button:hover {
            transform: translateY(-3px);
            box-shadow: 0 0 25px rgba(138, 43, 226, 0.8);
        }

        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        .hint-box {
            background: rgba(255, 215, 0, 0.1);
            border: 2px solid #ffd700;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
            display: none;
        }

        .hint-box.show {
            display: block;
            animation: fadeIn 0.5s;
        }

        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        .hint-box h3 {
            color: #ffd700;
            margin-bottom: 10px;
        }

        .hint-box p {
            color: #ffff00;
        }

        .victory {
            background: rgba(0, 255, 0, 0.2);
            border: 2px solid #00ff00;
            border-radius: 10px;
            padding: 30px;
            text-align: center;
            display: none;
            margin-top: 20px;
        }

        .victory.show {
            display: block;
            animation: victoryPulse 1s ease-in-out;
        }

        @keyframes victoryPulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }

        .victory h2 {
            color: #00ff00;
            font-size: 2em;
            margin-bottom: 20px;
        }

        .flag {
            background: rgba(0, 0, 0, 0.8);
            border: 2px solid #00ff88;
            border-radius: 10px;
            padding: 20px;
            font-size: 1.5em;
            color: #00ff88;
            margin: 20px 0;
            word-wrap: break-word;
            overflow-wrap: break-word;
        }

        .game-over {
            background: rgba(255, 0, 0, 0.2);
            border: 2px solid #ff0000;
            border-radius: 10px;
            padding: 20px;
            text-align: center;
            display: none;
            margin-top: 20px;
        }

        .game-over.show {
            display: block;
            animation: fadeIn 0.5s;
        }

        .game-over h2 {
            color: #ff0000;
            font-size: 1.8em;
            margin-bottom: 15px;
        }

        .confetti {
            position: fixed;
            width: 10px;
            height: 10px;
            top: -10px;
            z-index: 9999;
            animation: confettiFall 3s linear;
        }

        @keyframes confettiFall {
            to {
                transform: translateY(100vh) rotate(360deg);
                opacity: 0;
            }
        }

        @media (max-width: 600px) {
            h1 {
                font-size: 1.8em;
            }

            .pattern {
                font-size: 2em;
                letter-spacing: 3px;
            }

            .log-pattern {
                font-size: 1.5em;
                letter-spacing: 2px;
            }

            .stat-box {
                min-width: 110px;
                padding: 10px 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>PATTERN RECOGNITION GAME</h1>

        <div class="instructions">
            <h2>HOW TO PLAY:</h2>
            <p>Watch for patterns that match the target pattern shown above</p>
            <p>Click on log entries that match the target pattern</p>
            <p>Find 5 matching patterns among 25 log entries to win</p>
            <p>+100 points for correct matches, -20 points for wrong clicks</p>
            <p>Each pattern has 5 color blocks - pay attention!</p>
        </div>

        <div class="target-display">
            <div class="target-label">TARGET PATTERN:</div>
            <div class="pattern" id="targetPattern">🔴🔵🟢🟡🟣</div>
        </div>

        <div class="stats">
            <div class="stat-box">
                <div class="stat-label">MATCHES FOUND</div>
                <div class="stat-value" id="matchCount">0/5</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">SCORE</div>
                <div class="stat-value" id="score">0</div>
            </div>
        </div>

        <div class="log-container" id="logContainer">
            <p style="color: #00ffff; text-align: center; padding: 40px;">Press START GAME to begin pattern matching!</p>
        </div>

        <div class="buttons">
            <button onclick="startGame()" id="startBtn">START GAME</button>
            <button onclick="toggleHint()" id="hintBtn">SHOW HINT</button>
        </div>

        <div class="hint-box" id="hintBox">
            <h3>HINT:</h3>
            <p>Look carefully at the 5-color sequence! The target pattern will appear 5 times among 25 entries. Each pattern is unique - pay attention to the order of colors!</p>
        </div>

        <div class="game-over" id="gameOverBox">
            <h2>GAME OVER</h2>
            <p style="color: #ff6666; font-size: 1.2em;">All patterns shown! Check your score and try again for a perfect match!</p>
        </div>

        <div class="victory" id="victoryBox">
            <h2>CONGRATULATIONS!</h2>
            <p style="color: #00ffff; font-size: 1.2em; margin-bottom: 20px;">
                You've mastered pattern recognition!
            </p>
            <div class="flag" id="flagBox">{{FLAG}}<br><br>Next WiFi: <span style=\"color: #00ffff;\">{{NEXT_WIFI}}</span> | WiFi Password: <span style=\"color: #00ffff;\">{{WIFI_PASS}}</span><br><br>Clue: <span style=\"color: #00ff88;\">{{CLUE}}</span></div>
            <p style="color: #ffff00; margin-top: 20px;">Copy the flag above to continue your journey!</p>
            <button onclick="playAgain()" style="margin-top: 20px; padding: 15px 30px; font-size: 1.1em;">PLAY AGAIN</button>
        </div>
    </div>

    <script>
        const patterns = [
            '🔴🔵🟢🟡🟣', '🟡🟣🔴🔵🟢', '🔵🟢🟡🟣🔴', '🟢🟡🟣🔴🔵', '🟣🔴🔵🟢🟡',
            '🔴🟢🔵🟣🟡', '🟡🔵🟣🔴🟢', '🔵🟡🔴🟢🟣', '🟢🟣🟡🔵🔴', '🟣🟢🔴🟡🔵',
            '🔴🟡🟣🟢🔵', '🟡🔴🔵🟣🟢', '🔵🟣🟢🔴🟡', '🟢🔵🔴🟡🟣', '🟣🟡🟢🔵🔴',
            '🔴🟣🟡🔵🟢', '🟡🟢🔴🟣🔵', '🔵🔴🟣🟡🟢', '🟢🔴🟣🟡🔵', '🟣🔵🟡🟢🔴',
            '🔴🔵🟣🟡�', '🟡🟣🟢🔵�', '🔵🟢🔴🟣�', '🟢🟡🔵🟣�', '🟣🔴🟡🔵�'
        ];

        let targetPattern = '';
        let logEntries = [];
        let currentIndex = 0;
        let matchesFound = 0;
        let score = 0;
        let gameActive = false;
        let displayInterval;

        function startGame() {
            // Reset game state
            targetPattern = patterns[Math.floor(Math.random() * patterns.length)];
            logEntries = [];
            currentIndex = 0;
            matchesFound = 0;
            score = 0;
            gameActive = true;

            // Generate 25 log entries with 5 targets
            const targetIndices = [];
            while (targetIndices.length < 5) {
                const randomIndex = Math.floor(Math.random() * 25);
                if (!targetIndices.includes(randomIndex)) {
                    targetIndices.push(randomIndex);
                }
            }

            for (let i = 0; i < 25; i++) {
                if (targetIndices.includes(i)) {
                    logEntries.push({
                        pattern: targetPattern,
                        isTarget: true,
                        clicked: false
                    });
                } else {
                    let randomPattern;
                    do {
                        randomPattern = patterns[Math.floor(Math.random() * patterns.length)];
                    } while (randomPattern === targetPattern);
                    
                    logEntries.push({
                        pattern: randomPattern,
                        isTarget: false,
                        clicked: false
                    });
                }
            }

            // Update display
            document.getElementById('targetPattern').textContent = targetPattern;
            document.getElementById('logContainer').innerHTML = '';
            updateStats();

            document.getElementById('victoryBox').classList.remove('show');
            document.getElementById('gameOverBox').classList.remove('show');
            document.getElementById('startBtn').textContent = 'RESTART GAME';

            // Start showing entries
            showNextEntry();
        }

        function showNextEntry() {
            if (!gameActive || currentIndex >= logEntries.length) {
                if (currentIndex >= logEntries.length) {
                    endGame();
                }
                return;
            }

            const entry = logEntries[currentIndex];
            const logDiv = document.createElement('div');
            logDiv.className = 'log-entry';
            logDiv.dataset.index = currentIndex;

            logDiv.innerHTML = `
                <span class="log-pattern">${entry.pattern}</span>
                <span class="log-result" id="result-${currentIndex}"></span>
            `;

            logDiv.onclick = () => checkPattern(currentIndex);

            const container = document.getElementById('logContainer');
            container.insertBefore(logDiv, container.firstChild);

            // Keep only last 5 entries visible
            const allEntries = container.querySelectorAll('.log-entry');
            if (allEntries.length > 5) {
                allEntries[allEntries.length - 1].remove();
            }

            currentIndex++;
            setTimeout(showNextEntry, 1200);
        }

        function checkPattern(index) {
            if (!gameActive || logEntries[index].clicked) return;

            const entry = logEntries[index];
            entry.clicked = true;

            const logDiv = document.querySelector(`[data-index="${index}"]`);
            const resultSpan = document.getElementById(`result-${index}`);

            if (entry.isTarget) {
                matchesFound++;
                score += 100;
                logDiv.classList.add('correct');
                resultSpan.textContent = '✅';
                
                if (matchesFound >= 5) {
                    gameActive = false;
                    setTimeout(showVictory, 1000);
                }
            } else {
                score -= 20;
                logDiv.classList.add('incorrect');
                resultSpan.textContent = '❌';
            }

            updateStats();
        }

        function updateStats() {
            document.getElementById('matchCount').textContent = `${matchesFound}/5`;
            document.getElementById('score').textContent = score;
        }

        function endGame() {
            gameActive = false;
            document.getElementById('gameOverBox').classList.add('show');
        }

        function showVictory() {
            document.getElementById('victoryBox').classList.add('show');
            createConfetti();
        }

        function playAgain() {
            document.getElementById('victoryBox').classList.remove('show');
            startGame();
        }

        function createConfetti() {
            const colors = ['#ff00ff', '#00ffff', '#00ff88', '#ffd700', '#ff69b4'];
            for (let i = 0; i < 100; i++) {
                setTimeout(() => {
                    const confetti = document.createElement('div');
                    confetti.className = 'confetti';
                    confetti.style.left = Math.random() * 100 + 'vw';
                    confetti.style.background = colors[Math.floor(Math.random() * colors.length)];
                    confetti.style.animationDelay = Math.random() * 2 + 's';
                    document.body.appendChild(confetti);

                    setTimeout(() => confetti.remove(), 3000);
                }, i * 30);
            }
        }

        function toggleHint() {
            const hintBox = document.getElementById('hintBox');
            hintBox.classList.toggle('show');
        }
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
  server.send_P(200, "text/html", login_html);
}

// Route: Handle login POST
void handleLogin() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String teamId = server.arg("teamId");
  String pass = server.arg("password");

  int teamIdx = authenticateTeam(teamId, pass);

  if (teamIdx >= 0) {
    String clientIP = server.client().remoteIP().toString();
    addSession(clientIP, teamIdx);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(200, "text/plain", "FAIL");
  }
}

// Route: Game page (requires authentication)
void handleGame() {
  String clientIP = server.client().remoteIP().toString();
  int teamIdx = getTeamIndex(clientIP);

  if (teamIdx < 0) {
    // Not authenticated, redirect to login
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
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
  removeSession(clientIP);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/game", handleGame);
  server.on("/logout", handleLogout);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}