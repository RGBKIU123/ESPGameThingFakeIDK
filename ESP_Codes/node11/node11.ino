#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "shadow";
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
<title>Team Login - Challenge 11</title>
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
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Game 1: Memory Leak Hunter</title>
    <style>
      :root {
        --primary-glow: #00ff88;
        --secondary-glow: #ff00ff;
        --accent-glow: #8a2be2;
        --text-info: #00ffff;
        --danger-glow: #ff0000;
        --warning-glow: #ffd700;
        --background-start: #0a0a0a;
        --background-end: #1a0a2e;
      }
      * {
        margin: 0;
        padding: 0;
        box-sizing: border-box;
      }
      body {
        font-family: "Courier New", monospace;
        background: linear-gradient(
          135deg,
          var(--background-start) 0%,
          var(--background-end) 100%
        );
        color: var(--primary-glow);
        min-height: 100vh;
        padding: 10px; /* Reduced padding for mobile */
        overflow-x: hidden;
      }
      .container {
        max-width: 900px;
        margin: 0 auto;
        background: rgba(0, 0, 0, 0.8);
        border: 2px solid var(--primary-glow);
        border-radius: 15px;
        padding: 20px; /* Reduced padding */
        box-shadow: 0 0 30px rgba(0, 255, 136, 0.3);
      }
      h1 {
        text-align: center;
        color: var(--secondary-glow);
        font-size: 2em; /* Slightly reduced base font size */
        margin-bottom: 20px;
        text-shadow: 0 0 20px var(--secondary-glow);
        animation: glow 2s ease-in-out infinite alternate;
      }
      @keyframes glow {
        from {
          text-shadow: 0 0 10px var(--secondary-glow),
            0 0 20px var(--secondary-glow);
        }
        to {
          text-shadow: 0 0 20px var(--secondary-glow),
            0 0 30px var(--secondary-glow), 0 0 40px var(--secondary-glow);
        }
      }
      .instructions {
        background: rgba(138, 43, 226, 0.2);
        border: 2px solid var(--accent-glow);
        border-radius: 10px;
        padding: 15px; /* Reduced padding */
        margin-bottom: 25px;
      }
      .instructions h2 {
        color: var(--accent-glow);
        margin-bottom: 10px;
      }
      .instructions p {
        color: var(--text-info);
        line-height: 1.6;
        margin-bottom: 10px;
      }

      .stats {
        display: flex;
        justify-content: space-around;
        margin-bottom: 20px;
        flex-wrap: wrap;
        gap: 10px; /* Reduced gap */
      }
      .stat-box {
        background: rgba(255, 0, 255, 0.2);
        border: 2px solid var(--secondary-glow);
        border-radius: 10px;
        padding: 10px 15px; /* Reduced padding */
        text-align: center;
        flex: 1 1 120px; /* Allow boxes to grow and shrink */
      }
      .stat-label {
        color: var(--secondary-glow);
        font-size: 0.8em; /* Adjusted size */
        margin-bottom: 5px;
      }
      .stat-value {
        color: var(--primary-glow);
        font-size: 1.5em; /* Adjusted size */
        font-weight: bold;
      }

      .memory-grid {
        display: grid;
        /* Default to a mobile-friendly 3-column grid */
        grid-template-columns: repeat(3, 1fr);
        gap: 10px; /* Reduced gap */
        margin-bottom: 20px;
        padding: 15px; /* Reduced padding */
        background: rgba(0, 0, 0, 0.6);
        border: 2px solid var(--primary-glow);
        border-radius: 10px;
      }
      #memoryGrid p {
          color: var(--text-info);
          text-align: center;
          padding: 20px 0;
          grid-column: 1 / -1; /* Make message span all columns */
          font-size: 0.9em;
      }

      .memory-card {
        background: rgba(138, 43, 226, 0.3);
        border: 2px solid var(--accent-glow);
        border-radius: 10px;
        aspect-ratio: 1 / 1; /* Keep cards square */
        cursor: pointer;
        transition: all 0.3s;
        display: flex;
        justify-content: center;
        align-items: center;
        position: relative;
      }

      .memory-card:hover:not(.flipped):not(.matched) {
        transform: translateY(-5px);
        box-shadow: 0 0 20px rgba(138, 43, 226, 0.6);
        border-color: var(--secondary-glow);
      }

      .memory-card.flipped {
        background: rgba(255, 0, 255, 0.3);
        border-color: var(--secondary-glow);
      }

      .memory-card.matched {
        background: rgba(255, 0, 0, 0.3);
        border-color: var(--danger-glow);
        animation: pulse 0.5s;
      }

      @keyframes pulse { 0%, 100% { transform: scale(1); } 50% { transform: scale(1.1); } }

      .card-front {
        font-size: 2em;
        color: var(--text-info);
      }
      .card-back {
        font-size: 0.9em; /* Reduced for smaller cards */
        font-weight: bold;
        color: var(--danger-glow);
        display: none;
        word-wrap: break-word; /* Break long address if needed */
        overflow-wrap: break-word;
        padding: 5px;
      }
      .memory-card.flipped .card-front { display: none; }
      .memory-card.flipped .card-back,
      .memory-card.matched .card-back { display: block; }
      .memory-card.matched .card-front { display: none; }

      .buttons {
        display: flex;
        gap: 15px;
        justify-content: center;
        flex-wrap: wrap;
      }
      button {
        background: linear-gradient(
          135deg,
          var(--accent-glow) 0%,
          var(--secondary-glow) 100%
        );
        color: white;
        border: none;
        padding: 12px 25px; /* Reduced padding */
        font-size: 1em; /* Reduced size */
        border-radius: 10px;
        cursor: pointer;
        font-family: "Courier New", monospace;
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

      .victory {
        background: rgba(0, 255, 0, 0.2);
        border: 2px solid #00ff00;
        border-radius: 10px;
        padding: 20px;
        text-align: center;
        display: none;
        margin-top: 20px;
      }
      .victory.show { display: block; animation: victoryPulse 1s ease-in-out; }
      @keyframes victoryPulse { 0%, 100% { transform: scale(1); } 50% { transform: scale(1.05); } }
      .victory h2 {
        color: #00ff00;
        font-size: 1.5em; /* Adjusted size */
        margin-bottom: 15px;
      }
      .flag {
        background: rgba(0, 0, 0, 0.8);
        border: 2px solid var(--primary-glow);
        border-radius: 10px;
        padding: 15px;
        font-size: 1.1em; /* Adjusted size */
        color: var(--primary-glow);
        margin: 20px 0;
        word-wrap: break-word;
        overflow-wrap: break-word;
      }
      .hint-box {
        background: rgba(255, 215, 0, 0.1);
        border: 2px solid var(--warning-glow);
        border-radius: 10px;
        padding: 15px;
        margin-top: 20px;
        display: none;
      }
      .hint-box.show { display: block; animation: fadeIn 0.5s; }
      @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }
      .hint-box h3 { color: var(--warning-glow); margin-bottom: 10px; }
      .hint-box p { color: #ffff00; }

      /* Media Query for larger screens (tablets and desktops) */
      @media (min-width: 600px) {
        body { padding: 20px; }
        .container { padding: 30px; }
        h1 { font-size: 2.5em; }
        .memory-grid {
          /* Use a 4-column grid on larger screens */
          grid-template-columns: repeat(4, 1fr);
          gap: 15px;
        }
        .card-back { font-size: 1.1em; }
        .victory h2 { font-size: 2em; }
        .flag { font-size: 1.5em; }
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>🧠 MEMORY LEAK HUNTER 🧠</h1>
      <div class="instructions">
        <h2>MISSION BRIEFING:</h2>
        <p>💾 Your ESP32 system has memory leaks!</p>
        <p>🔍 Find matching pairs of leaked memory addresses</p>
        <p>🎯 Match all 6 pairs to fix the system!</p>
        <p>⏱️ Complete in minimum moves to maximize your score</p>
      </div>

      <div class="stats">
        <div class="stat-box">
          <div class="stat-label">PAIRS FOUND</div>
          <div class="stat-value" id="pairsFound">0/6</div>
        </div>
        <div class="stat-box">
          <div class="stat-label">MOVES</div>
          <div class="stat-value" id="moves">0</div>
        </div>
        <div class="stat-box">
          <div class="stat-label">SCORE</div>
          <div class="stat-value" id="score">0</div>
        </div>
      </div>

      <div class="memory-grid" id="memoryGrid"></div>

      <div class="buttons">
        <button onclick="startGame()" id="startBtn">START GAME</button>
        <button onclick="toggleHint()" id="hintBtn">SHOW HINT</button>
      </div>

      <div class="hint-box" id="hintBox">
        <h3>💡 HINT:</h3>
        <p>
          Memory addresses that match indicate the same leak location. The
          faster you complete it, the higher your score!
        </p>
      </div>

      <div class="victory" id="victoryBox">
        <h2>🎉 ALL MEMORY LEAKS PATCHED! 🎉</h2>
        <p style="color: #00ffff; font-size: 1.2em; margin-bottom: 20px">
          Your ESP32 system is now optimized and leak-free!
        </p>
        <div class="flag" id="flagBox">{{FLAG}}</div>
        <p style="color: #ffff00; margin-top: 15px">Next WiFi: <span style="color: #00ffff;">{{NEXT_WIFI}}</span> | WiFi Password: <span style="color: #00ffff;">{{WIFI_PASS}}</span></p>
        <p style="color: #00ff88; margin-top: 15px">Clue: {{CLUE}}</p>
        <p style="color: #ffff00; margin-top: 20px">
          Copy the flag above to continue your journey!
        p>
      </div>
    </div>

    <script>
      const memoryAddresses = [
        "0x3FFB0000",
        "0x3FFB1234",
        "0x3FFB5678",
        "0x3FFC9ABC",
        "0x3FFCDEF0",
        "0x3FFD4321",
      ];

      let cards = [],
        flippedCards = [],
        matchedPairs = 0,
        moves = 0,
        score = 0;
      let gameActive = false,
        canFlip = true;

      window.onload = () => {
        if (localStorage.getItem("flagUnlocked") === "true") {
          document.getElementById("victoryBox").classList.add("show");
        } else {
          document.getElementById("memoryGrid").innerHTML =
            '<p>Press START GAME to begin hunting memory leaks!</p>';
        }
      };

      function startGame() {
        if (localStorage.getItem("flagUnlocked") === "true") {
          alert("🎉 You've already completed this challenge!");
          return;
        }

        cards = [];
        flippedCards = [];
        matchedPairs = 0;
        moves = 0;
        score = 1000;
        gameActive = true;
        canFlip = true;

        const addresses = [...memoryAddresses, ...memoryAddresses];
        addresses.sort(() => Math.random() - 0.5);

        cards = addresses.map((addr, i) => ({
          id: i,
          address: addr,
          flipped: false,
          matched: false,
        }));

        renderCards();
        updateDisplay();

        document.getElementById("victoryBox").classList.remove("show");
        document.getElementById("startBtn").textContent = "RESTART GAME";
      }

      function renderCards() {
        const grid = document.getElementById("memoryGrid");
        grid.innerHTML = "";
        cards.forEach((card, i) => {
          const div = document.createElement("div");
          div.className = "memory-card";
          if (card.flipped) div.classList.add("flipped");
          if (card.matched) div.classList.add("matched");
          div.innerHTML = `<div class="card-front">🔒</div><div class="card-back">${card.address}</div>`;
          div.onclick = () => flipCard(i);
          grid.appendChild(div);
        });
      }

      function flipCard(i) {
        if (!gameActive || !canFlip || cards[i].matched || cards[i].flipped)
          return;
        if (flippedCards.length >= 2) return;

        cards[i].flipped = true;
        flippedCards.push(i);
        renderCards();

        if (flippedCards.length === 2) {
          moves++;
          score = Math.max(0, score - 10);
          updateDisplay();
          checkMatch();
        }
      }

      function checkMatch() {
        canFlip = false;
        const [a, b] = flippedCards;
        setTimeout(() => {
          if (cards[a].address === cards[b].address) {
            cards[a].matched = true;
            cards[b].matched = true;
            matchedPairs++;
            score += 100;
            if (matchedPairs === 6) setTimeout(showVictory, 500);
          } else {
            cards[a].flipped = false;
            cards[b].flipped = false;
          }
          flippedCards = [];
          canFlip = true;
          renderCards();
          updateDisplay();
        }, 1000);
      }

      function updateDisplay() {
        document.getElementById("pairsFound").textContent = `${matchedPairs}/6`;
        document.getElementById("moves").textContent = moves;
        document.getElementById("score").textContent = score;
      }

      function showVictory() {
        gameActive = false;
        document.getElementById("victoryBox").classList.add("show");
        localStorage.setItem("flagUnlocked", "true");
      }

      function toggleHint() {
        document.getElementById("hintBox").classList.toggle("show");
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