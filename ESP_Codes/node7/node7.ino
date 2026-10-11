#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "void";
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
<title>Team Login - Challenge 7</title>
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

// Game page HTML - Word Scramble Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Word Scramble</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #0a0a1a 0%, #1a0a2e 50%, #0a1a2e 100%);
            color: #00ff88;
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .container {
            max-width: 900px;
            width: 100%;
            background: rgba(10, 10, 30, 0.95);
            border: 3px solid #00ff88;
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 0 40px rgba(0, 255, 136, 0.4), inset 0 0 30px rgba(0, 255, 136, 0.05);
        }

        h1 {
            text-align: center;
            color: #ff00ff;
            text-shadow: 0 0 20px #ff00ff, 0 0 40px #ff00ff;
            margin-bottom: 10px;
            font-size: 2.8em;
            animation: glow 2s ease-in-out infinite alternate;
        }

        @keyframes glow {
            from {
                text-shadow: 0 0 20px #ff00ff, 0 0 40px #ff00ff;
            }
            to {
                text-shadow: 0 0 30px #ff00ff, 0 0 60px #ff00ff, 0 0 80px #ff00ff;
            }
        }

        .subtitle {
            text-align: center;
            color: #00ccff;
            margin-bottom: 25px;
            font-size: 1.3em;
            text-shadow: 0 0 10px #00ccff;
        }

        .instructions {
            background: rgba(0, 255, 136, 0.1);
            border: 2px solid #00ff88;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 25px;
        }

        .instructions h2 {
            color: #00ccff;
            margin-bottom: 12px;
            font-size: 1.4em;
        }

        .instructions ul {
            list-style: none;
            padding-left: 20px;
            line-height: 1.8;
        }

        .instructions li::before {
            content: "▹ ";
            color: #ff00ff;
            font-size: 1.2em;
        }

        .score-board {
            display: flex;
            justify-content: space-around;
            margin-bottom: 25px;
            padding: 20px;
            background: linear-gradient(135deg, rgba(255, 0, 255, 0.15), rgba(0, 204, 255, 0.15));
            border: 2px solid #ff00ff;
            border-radius: 15px;
        }

        .score-item {
            text-align: center;
        }

        .score-label {
            color: #00ccff;
            font-size: 1em;
            margin-bottom: 8px;
        }

        .score-value {
            color: #00ff88;
            font-size: 2em;
            font-weight: bold;
            text-shadow: 0 0 15px #00ff88;
        }

        .word-card {
            background: rgba(0, 204, 255, 0.1);
            border: 2px solid #00ccff;
            border-radius: 15px;
            padding: 25px;
            margin-bottom: 20px;
            transition: all 0.3s ease;
        }

        .word-card.solved {
            border-color: #00ff88;
            background: rgba(0, 255, 136, 0.15);
            opacity: 0.7;
        }

        .word-card:not(.solved):hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(0, 204, 255, 0.4);
        }

        .word-number {
            color: #ff00ff;
            font-size: 1.3em;
            margin-bottom: 10px;
            font-weight: bold;
        }

        .scrambled-word {
            font-size: 2.5em;
            color: #00ccff;
            text-align: center;
            letter-spacing: 8px;
            margin: 20px 0;
            text-shadow: 0 0 15px #00ccff;
            font-weight: bold;
        }

        .input-group {
            display: flex;
            gap: 10px;
            margin-top: 20px;
        }

        input[type="text"] {
            flex: 1;
            padding: 15px;
            background: rgba(0, 0, 0, 0.6);
            border: 2px solid #00ff88;
            border-radius: 10px;
            color: #00ff88;
            font-family: 'Courier New', monospace;
            font-size: 1.2em;
            text-transform: uppercase;
        }

        input[type="text"]:focus {
            outline: none;
            box-shadow: 0 0 20px rgba(0, 255, 136, 0.6);
            border-color: #00ff88;
        }

        input[type="text"]:disabled {
            opacity: 0.5;
        }

        button {
            padding: 15px 25px;
            background: linear-gradient(135deg, #ff00ff, #00ccff);
            border: none;
            border-radius: 10px;
            color: white;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            font-size: 1em;
        }

        button:hover:not(:disabled) {
            transform: scale(1.05);
            box-shadow: 0 0 25px rgba(255, 0, 255, 0.6);
        }

        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        .hint-btn {
            background: linear-gradient(135deg, #ff9900, #ff0088);
        }

        .hint-text {
            color: #ffaa00;
            margin-top: 12px;
            font-style: italic;
            display: none;
            padding: 10px;
            background: rgba(255, 170, 0, 0.1);
            border-radius: 8px;
            border-left: 4px solid #ffaa00;
        }

        .hint-text.show {
            display: block;
            animation: slideDown 0.3s ease;
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

        .status {
            text-align: center;
            margin-top: 12px;
            font-size: 1.2em;
            min-height: 30px;
            font-weight: bold;
        }

        .correct {
            color: #00ff88;
            text-shadow: 0 0 15px #00ff88;
            animation: pulse 0.5s ease;
        }

        .incorrect {
            color: #ff0088;
            text-shadow: 0 0 15px #ff0088;
            animation: shake 0.5s ease;
        }

        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }

        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            75% { transform: translateX(10px); }
        }

        .flag-display {
            background: linear-gradient(135deg, rgba(255, 0, 255, 0.2), rgba(0, 255, 136, 0.2));
            border: 3px solid #00ff88;
            border-radius: 20px;
            padding: 40px;
            text-align: center;
            margin-top: 30px;
            display: none;
        }

        .flag-display.show {
            display: block;
            animation: slideIn 0.6s ease;
        }

        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .flag-display h2 {
            color: #ff00ff;
            margin-bottom: 20px;
            font-size: 2.5em;
            text-shadow: 0 0 20px #ff00ff;
        }

        .flag-value {
            color: #00ff88;
            font-size: 2em;
            padding: 20px;
            background: rgba(0, 0, 0, 0.6);
            border-radius: 15px;
            border: 2px solid #00ff88;
            word-wrap: break-word;
            overflow-wrap: break-word;
            text-shadow: 0 0 20px #00ff88;
            font-weight: bold;
            letter-spacing: 2px;
        }

        .confetti {
            position: fixed;
            width: 10px;
            height: 10px;
            top: -10px;
            animation: confettiFall 3s linear;
            pointer-events: none;
            z-index: 1000;
        }

        @keyframes confettiFall {
            to {
                transform: translateY(100vh) rotate(720deg);
                opacity: 0;
            }
        }

        @media (max-width: 768px) {
            h1 {
                font-size: 2em;
            }

            .container {
                padding: 20px;
            }

            .scrambled-word {
                font-size: 1.8em;
                letter-spacing: 4px;
            }

            .input-group {
                flex-direction: column;
            }

            button {
                width: 100%;
            }

            .score-board {
                flex-direction: column;
                gap: 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>WORD SCRAMBLE</h1>
        <div class="subtitle">Challenge: Cyber Word Unscrambler</div>

        <div class="instructions">
            <h2>How to Play:</h2>
            <ul>
                <li>Unscramble each cybersecurity-related word</li>
                <li>Type your answer and click CHECK</li>
                <li>Use HINT if you need help (costs 10 points)</li>
                <li>Solve all 8 words to reveal the FLAG!</li>
            </ul>
        </div>

        <div class="score-board">
            <div class="score-item">
                <div class="score-label">WORDS SOLVED</div>
                <div class="score-value" id="solved">0/8</div>
            </div>
            <div class="score-item">
                <div class="score-label">SCORE</div>
                <div class="score-value" id="score">0</div>
            </div>
        </div>

        <div id="words-container"></div>

        <div class="flag-display" id="flag-display" 
             data-flag="{{FLAG}}" 
             data-nextwifi="{{NEXT_WIFI}}" 
             data-wifipass="{{WIFI_PASS}}" 
             data-clue="{{CLUE}}">
            <h2>CONGRATULATIONS!</h2>
            <p style="color: #00ccff; margin-bottom: 20px; font-size: 1.3em;">You've unscrambled all words!</p>
            <div class="flag-value" id="flag-value"></div>
        </div>
    </div>

    <script>
        const words = [
            { scrambled: "SIVRU", answer: "VIRUS", hint: "Malicious software that can replicate itself" },
            { scrambled: "REWALLIF", answer: "FIREWALL", hint: "Security system that monitors network traffic" },
            { scrambled: "TPRYENC", answer: "ENCRYPT", hint: "Convert data into a coded format" },
            { scrambled: "REHAKC", answer: "HACKER", hint: "Someone who breaks into computer systems" },
            { scrambled: "DROWSSAP", answer: "PASSWORD", hint: "Secret word used for authentication" },
            { scrambled: "AWEMLRA", answer: "MALWARE", hint: "Malicious software designed to harm computers" },
            { scrambled: "AATADBSE", answer: "DATABASE", hint: "Organized collection of structured information" },
            { scrambled: "YINBRA", answer: "BINARY", hint: "Number system using only 0 and 1" }
        ];

        let solvedCount = 0;
        let totalScore = 0;
        let hintsUsed = new Set();

        function createWordCards() {
            const container = document.getElementById('words-container');
            container.innerHTML = '';

            words.forEach((word, index) => {
                const card = document.createElement('div');
                card.className = 'word-card';
                card.id = `word-${index}`;

                card.innerHTML = `
                    <div class="word-number">Word ${index + 1} of 8</div>
                    <div class="scrambled-word">${word.scrambled}</div>
                    <div class="input-group">
                        <input type="text" id="input-${index}" placeholder="Your answer..." maxlength="15">
                        <button onclick="checkAnswer(${index})">CHECK</button>
                        <button class="hint-btn" onclick="showHint(${index})">HINT</button>
                    </div>
                    <div class="hint-text" id="hint-${index}">Hint: ${word.hint}</div>
                    <div class="status" id="status-${index}"></div>
                `;

                container.appendChild(card);
            });
        }

        function checkAnswer(index) {
            const input = document.getElementById(`input-${index}`);
            const answer = input.value.trim().toUpperCase();
            const status = document.getElementById(`status-${index}`);
            const card = document.getElementById(`word-${index}`);

            if (!answer) {
                status.textContent = 'Please enter an answer!';
                status.className = 'status incorrect';
                return;
            }

            if (answer === words[index].answer) {
                status.textContent = 'CORRECT!';
                status.className = 'status correct';
                card.classList.add('solved');
                input.disabled = true;

                // Calculate points
                let points = 100;
                if (hintsUsed.has(index)) {
                    points -= 10;
                }

                totalScore += points;
                solvedCount++;

                updateScoreBoard();
                createConfetti();

                if (solvedCount === words.length) {
                    setTimeout(showFlag, 1000);
                }
            } else {
                status.textContent = 'Try again!';
                status.className = 'status incorrect';
                input.value = '';
            }
        }

        function showHint(index) {
            const hint = document.getElementById(`hint-${index}`);
            if (!hintsUsed.has(index)) {
                hintsUsed.add(index);
                if (solvedCount < words.length) {
                    totalScore -= 10;
                    updateScoreBoard();
                }
            }
            hint.classList.add('show');
        }

        function updateScoreBoard() {
            document.getElementById('solved').textContent = `${solvedCount}/8`;
            document.getElementById('score').textContent = Math.max(0, totalScore);
        }

        function createConfetti() {
            const colors = ['#ff00ff', '#00ff88', '#00ccff', '#ff0088', '#ffaa00'];
            for (let i = 0; i < 30; i++) {
                setTimeout(() => {
                    const confetti = document.createElement('div');
                    confetti.className = 'confetti';
                    confetti.style.left = Math.random() * 100 + '%';
                    confetti.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
                    confetti.style.animationDelay = '0s';
                    confetti.style.animationDuration = (Math.random() * 2 + 2) + 's';
                    document.body.appendChild(confetti);

                    setTimeout(() => confetti.remove(), 3000);
                }, i * 30);
            }
        }

        function showFlag() {
            const flagDisplay = document.getElementById('flag-display');
            const flagValue = document.getElementById('flag-value');

            // Read data from data attributes (safe from quote issues)
            const flag = flagDisplay.getAttribute('data-flag');
            const nextWifi = flagDisplay.getAttribute('data-nextwifi');
            const wifiPass = flagDisplay.getAttribute('data-wifipass');
            const clue = flagDisplay.getAttribute('data-clue');

            flagValue.innerHTML = flag + '<br><br>Next WiFi: <span style="color: #00ffff;">' + nextWifi + '</span> | WiFi Password: <span style="color: #00ffff;">' + wifiPass + '</span><br><br>Clue: <span style="color: #00ff88;">' + clue + '</span>';
            flagDisplay.classList.add('show');

            // Extra confetti for completion
            for (let i = 0; i < 3; i++) {
                setTimeout(createConfetti, i * 200);
            }
        }

        // Allow Enter key to submit
        document.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                const input = e.target;
                if (input.tagName === 'INPUT' && input.type === 'text') {
                    const index = parseInt(input.id.split('-')[1]);
                    checkAnswer(index);
                }
            }
        });

        // Initialize
        createWordCards();
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