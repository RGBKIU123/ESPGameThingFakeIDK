#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "fire";
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
<title>Team Login - Challenge 4</title>
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

// Game page HTML - Network Memory Match Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Challenge 17 - Network Scanner Memory Game</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #0a0e27 0%, #1a1a2e 50%, #16213e 100%);
            color: #00ff41;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            overflow-x: hidden;
        }

        .container {
            max-width: 900px;
            width: 100%;
            background: rgba(0, 0, 0, 0.7);
            border: 3px solid #00ff41;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 0 30px rgba(0, 255, 65, 0.5), inset 0 0 20px rgba(0, 255, 65, 0.1);
        }

        h1 {
            text-align: center;
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 0 0 10px #00ff41, 0 0 20px #00ff41;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }

        .subtitle {
            text-align: center;
            color: #9d00ff;
            font-size: 1.2em;
            margin-bottom: 20px;
            text-shadow: 0 0 10px #9d00ff;
        }

        .instructions {
            background: rgba(157, 0, 255, 0.2);
            border: 2px solid #9d00ff;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
        }

        .instructions h2 {
            color: #9d00ff;
            margin-bottom: 10px;
            font-size: 1.5em;
        }

        .instructions p {
            color: #00ffff;
            line-height: 1.6;
            margin-bottom: 10px;
        }

        .stats {
            display: flex;
            justify-content: space-around;
            margin: 20px 0;
            flex-wrap: wrap;
            gap: 15px;
        }

        .stat-box {
            background: rgba(0, 255, 255, 0.1);
            border: 2px solid #00ffff;
            border-radius: 10px;
            padding: 15px 25px;
            text-align: center;
            min-width: 120px;
        }

        .stat-label {
            color: #00ffff;
            font-size: 0.9em;
            margin-bottom: 5px;
        }

        .stat-value {
            color: #00ff41;
            font-size: 1.8em;
            font-weight: bold;
            text-shadow: 0 0 10px #00ff41;
        }

        .game-board {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 15px;
            margin: 30px 0;
            perspective: 1000px;
        }

        .card {
            aspect-ratio: 1;
            background: linear-gradient(135deg, #1a1a2e 0%, #0f3460 100%);
            border: 3px solid #00ff41;
            border-radius: 10px;
            cursor: pointer;
            position: relative;
            transform-style: preserve-3d;
            transition: transform 0.6s, box-shadow 0.3s;
        }

        .card:hover {
            box-shadow: 0 0 20px rgba(0, 255, 65, 0.6);
        }

        .card.flipped {
            transform: rotateY(180deg);
        }

        .card.matched {
            border-color: #9d00ff;
            box-shadow: 0 0 20px rgba(157, 0, 255, 0.8);
            animation: matchPulse 0.5s;
        }

        @keyframes matchPulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }

        .card-front, .card-back {
            position: absolute;
            width: 100%;
            height: 100%;
            backface-visibility: hidden;
            display: flex;
            justify-content: center;
            align-items: center;
            border-radius: 8px;
        }

        .card-front {
            background: linear-gradient(135deg, #1a1a2e 0%, #0f3460 100%);
            font-size: 3em;
        }

        .card-back {
            background: linear-gradient(135deg, #0a0e27 0%, #16213e 100%);
            transform: rotateY(180deg);
            font-size: 3.5em;
        }

        .card-pattern {
            color: #00ff41;
            font-size: 2em;
            text-shadow: 0 0 10px #00ff41;
        }

        .buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
        }

        button {
            background: linear-gradient(135deg, #00ff41 0%, #00cc33 100%);
            border: none;
            border-radius: 8px;
            padding: 12px 30px;
            font-size: 1.1em;
            font-weight: bold;
            color: #0a0e27;
            cursor: pointer;
            transition: all 0.3s;
            box-shadow: 0 0 15px rgba(0, 255, 65, 0.5);
            font-family: 'Courier New', monospace;
        }

        button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 25px rgba(0, 255, 65, 0.8);
        }

        button:active {
            transform: scale(0.95);
        }

        .hint-btn {
            background: linear-gradient(135deg, #9d00ff 0%, #7700cc 100%);
            color: white;
            box-shadow: 0 0 15px rgba(157, 0, 255, 0.5);
        }

        .hint-btn:hover {
            box-shadow: 0 0 25px rgba(157, 0, 255, 0.8);
        }

        .hint-box {
            background: rgba(157, 0, 255, 0.2);
            border: 2px solid #9d00ff;
            border-radius: 10px;
            padding: 15px;
            margin: 20px 0;
            color: #00ffff;
            display: none;
        }

        .hint-box.show {
            display: block;
            animation: slideDown 0.3s;
        }

        @keyframes slideDown {
            from {
                opacity: 0;
                transform: translateY(-10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .victory {
            display: none;
            position: fixed;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            background: linear-gradient(135deg, #0a0e27 0%, #16213e 100%);
            border: 5px solid #00ff41;
            border-radius: 20px;
            padding: 40px;
            text-align: center;
            z-index: 1000;
            box-shadow: 0 0 50px rgba(0, 255, 65, 0.8);
            max-width: 500px;
            width: 90%;
        }

        .victory.show {
            display: block;
            animation: victoryPop 0.5s;
        }

        @keyframes victoryPop {
            0% {
                transform: translate(-50%, -50%) scale(0);
            }
            50% {
                transform: translate(-50%, -50%) scale(1.1);
            }
            100% {
                transform: translate(-50%, -50%) scale(1);
            }
        }

        .victory h2 {
            color: #00ff41;
            font-size: 2.5em;
            margin-bottom: 20px;
            text-shadow: 0 0 20px #00ff41;
        }

        .flag {
            background: rgba(157, 0, 255, 0.3);
            border: 3px solid #9d00ff;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            font-size: 1.5em;
            color: #00ffff;
            word-wrap: break-word;
            overflow-wrap: break-word;
            text-shadow: 0 0 10px #00ffff;
        }

        .confetti {
            position: fixed;
            width: 10px;
            height: 10px;
            top: -10px;
            z-index: 999;
            animation: confettiFall 3s linear;
        }

        @keyframes confettiFall {
            to {
                transform: translateY(100vh) rotate(360deg);
                opacity: 0;
            }
        }

        .overlay {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            z-index: 999;
        }

        .overlay.show {
            display: block;
        }

        @media (max-width: 768px) {
            h1 {
                font-size: 1.8em;
            }

            .game-board {
                grid-template-columns: repeat(4, 1fr);
                gap: 10px;
            }

            .card-back {
                font-size: 2em;
            }

            .stat-box {
                min-width: 90px;
                padding: 10px 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>NETWORK SCANNER</h1>
        <div class="subtitle">Memory Match</div>

        <div class="instructions">
            <h2>How to Play:</h2>
            <p>Match all pairs of network icons to complete the scan!</p>
            <p>Click cards to flip them over</p>
            <p>Find matching pairs - they'll stay revealed</p>
            <p>Match all pairs to discover the flag</p>
            <p>Try to complete in as few moves as possible!</p>
        </div>

        <div class="stats">
            <div class="stat-box">
                <div class="stat-label">MOVES</div>
                <div class="stat-value" id="moves">0</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">MATCHES</div>
                <div class="stat-value" id="matches">0/8</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">TIME</div>
                <div class="stat-value" id="timer">0:00</div>
            </div>
        </div>

        <div class="game-board" id="gameBoard"></div>

        <div class="buttons">
            <button onclick="resetGame()">NEW GAME</button>
            <button class="hint-btn" onclick="toggleHint()">HINT</button>
        </div>

        <div class="hint-box" id="hintBox">
            <strong>Hint:</strong> Take your time to memorize the positions! Click two cards at a time to find matching network icons. Router, Server, Laptop, Phone, WiFi, Firewall, Switch, and Cloud - find all 8 pairs!
        </div>
    </div>

    <div class="overlay" id="overlay"></div>

    <div class="victory" id="victoryScreen">
        <h2>NETWORK SCANNED!</h2>
        <p style="color: #00ffff; font-size: 1.2em; margin: 20px 0;">
            Congratulations! You've successfully mapped the entire network!
        </p>
        <div class="flag" id="flagDisplay" 
             data-flag="{{FLAG}}" 
             data-nextwifi="{{NEXT_WIFI}}" 
             data-wifipass="{{WIFI_PASS}}" 
             data-clue="{{CLUE}}"></div>
        <p style="color: #9d00ff; margin-top: 20px;">
            Time: <span id="finalTime"></span> | Moves: <span id="finalMoves"></span>
        </p>
        <button onclick="resetGame()" style="margin-top: 20px;">PLAY AGAIN</button>
    </div>

    <script>
        const icons = [
            '🖥️', '🖥️',
            '💾', '💾',
            '💻', '💻',
            '📱', '📱',
            '📡', '📡',
            '🔒', '🔒',
            '🔌', '🔌',
            '☁️', '☁️'
        ];

        let cards = [];
        let flippedCards = [];
        let matchedPairs = 0;
        let moves = 0;
        let timerInterval;
        let seconds = 0;
        let canFlip = true;
        const TIME_LIMIT = 50;

        function shuffle(array) {
            const newArray = [...array];
            for (let i = newArray.length - 1; i > 0; i--) {
                const j = Math.floor(Math.random() * (i + 1));
                [newArray[i], newArray[j]] = [newArray[j], newArray[i]];
            }
            return newArray;
        }

        function initGame() {
            cards = shuffle(icons);
            const gameBoard = document.getElementById('gameBoard');
            gameBoard.innerHTML = '';

            cards.forEach((icon, index) => {
                const card = document.createElement('div');
                card.className = 'card';
                card.dataset.index = index;
                card.innerHTML = `
                    <div class="card-front">
                        <div class="card-pattern">◆</div>
                    </div>
                    <div class="card-back">${icon}</div>
                `;
                card.addEventListener('click', () => flipCard(card, index));
                gameBoard.appendChild(card);
            });

            matchedPairs = 0;
            moves = 0;
            seconds = 0;
            updateStats();
            startTimer();
        }

        function flipCard(card, index) {
            if (!canFlip || card.classList.contains('flipped') || card.classList.contains('matched')) {
                return;
            }

            card.classList.add('flipped');
            flippedCards.push({ card, index });

            if (flippedCards.length === 2) {
                moves++;
                updateStats();
                canFlip = false;

                setTimeout(checkMatch, 800);
            }
        }

        function checkMatch() {
            const [first, second] = flippedCards;

            if (cards[first.index] === cards[second.index]) {
                first.card.classList.add('matched');
                second.card.classList.add('matched');
                matchedPairs++;
                updateStats();

                if (matchedPairs === 8) {
                    setTimeout(showVictory, 500);
                }
            } else {
                setTimeout(() => {
                    first.card.classList.remove('flipped');
                    second.card.classList.remove('flipped');
                }, 500);
            }

            flippedCards = [];
            canFlip = true;
        }

        function updateStats() {
            document.getElementById('moves').textContent = moves;
            document.getElementById('matches').textContent = `${matchedPairs}/8`;
        }

        function startTimer() {
            clearInterval(timerInterval);
            timerInterval = setInterval(() => {
                seconds++;
                const mins = Math.floor(seconds / 60);
                const secs = seconds % 60;
                document.getElementById('timer').textContent =
                    `${mins}:${secs.toString().padStart(2, '0')}`;
                
                if (seconds >= TIME_LIMIT) {
                    gameOver();
                }
            }, 1000);
        }

        function gameOver() {
            clearInterval(timerInterval);
            canFlip = false;
            alert('Time\'s up! You ran out of time. Click NEW GAME to try again.');
            setTimeout(resetGame, 1000);
        }

        function showVictory() {
            clearInterval(timerInterval);

            const overlay = document.getElementById('overlay');
            const victory = document.getElementById('victoryScreen');
            const flagDisplay = document.getElementById('flagDisplay');

            overlay.classList.add('show');
            victory.classList.add('show');

            const mins = Math.floor(seconds / 60);
            const secs = seconds % 60;
            document.getElementById('finalTime').textContent =
                `${mins}:${secs.toString().padStart(2, '0')}`;
            document.getElementById('finalMoves').textContent = moves;

            // Read data from data attributes (safe from quote issues)
            const flag = flagDisplay.getAttribute('data-flag');
            const nextWifi = flagDisplay.getAttribute('data-nextwifi');
            const wifiPass = flagDisplay.getAttribute('data-wifipass');
            const clue = flagDisplay.getAttribute('data-clue');

            flagDisplay.innerHTML = flag + '<br><br>Next WiFi: <span style="color: #00ffff;">' + nextWifi + '</span> | WiFi Password: <span style="color: #00ffff;">' + wifiPass + '</span><br><br>Clue: <span style="color: #00ff88;">' + clue + '</span>';

            createConfetti();
        }

        function createConfetti() {
            const colors = ['#00ff41', '#9d00ff', '#00ffff', '#ff00ff', '#ffff00'];
            for (let i = 0; i < 100; i++) {
                setTimeout(() => {
                    const confetti = document.createElement('div');
                    confetti.className = 'confetti';
                    confetti.style.left = Math.random() * 100 + '%';
                    confetti.style.background = colors[Math.floor(Math.random() * colors.length)];
                    confetti.style.animationDelay = Math.random() * 2 + 's';
                    document.body.appendChild(confetti);

                    setTimeout(() => confetti.remove(), 3000);
                }, i * 30);
            }
        }

        function resetGame() {
            document.getElementById('overlay').classList.remove('show');
            document.getElementById('victoryScreen').classList.remove('show');
            clearInterval(timerInterval);
            flippedCards = [];
            canFlip = true;
            initGame();
        }

        function toggleHint() {
            document.getElementById('hintBox').classList.toggle('show');
        }

        initGame();
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