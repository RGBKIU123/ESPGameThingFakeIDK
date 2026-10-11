#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "aether";
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
<title>Team Login - Challenge 14</title>
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

// Game page HTML - Sequence Solver Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sequence Solver - Challenge 6</title>
    <style>
    * {margin: 0; padding: 0; box-sizing: border-box;}
    body {
      background: linear-gradient(135deg, #1a1a3a, #2a2a4a);
      color: #00aaff;
      font-family: 'Courier New', monospace;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }

    .container {
      max-width: 600px;
      width: 100%;
      background: rgba(13, 13, 37, 0.9);
      border: 3px solid #00ccff;
      border-radius: 15px;
      padding: 30px;
      box-shadow: 0 0 30px rgba(0, 204, 255, 0.5);
      text-align: center;
    }

    h1 {
      color: #00ccff;
      font-size: 2.2rem;
      text-shadow: 0 0 10px #00ccff;
      margin-bottom: 20px;
    }

    .stats {
      display: flex;
      justify-content: space-around;
      margin: 20px 0;
      flex-wrap: wrap;
      gap: 15px;
    }

    .stat-box {
      background: rgba(0, 170, 255, 0.1);
      border: 2px solid #00aaff;
      border-radius: 8px;
      padding: 10px 15px;
      min-width: 100px;
    }

    .stat-label {
      color: #00aaff;
      font-size: 0.9rem;
      margin-bottom: 5px;
    }

    .stat-value {
      color: #00ccff;
      font-size: 1.3rem;
      font-weight: bold;
      text-shadow: 0 0 8px #00ccff;
    }

    #sequence {
      font-size: 1.8rem;
      margin: 25px 0;
      color: #00ffcc;
      text-shadow: 0 0 8px #00ffcc;
      padding: 15px;
      background: rgba(0, 255, 204, 0.1);
      border: 2px solid #00ffcc;
      border-radius: 10px;
    }

    input {
      padding: 12px;
      border-radius: 8px;
      border: 2px solid #00ccff;
      outline: none;
      font-size: 1.1rem;
      text-align: center;
      background-color: #0d0d25;
      color: #00ccff;
      box-shadow: 0 0 8px rgba(0, 204, 255, 0.3);
      margin: 15px 10px;
      width: 150px;
    }

    input:focus {
      box-shadow: 0 0 15px rgba(0, 204, 255, 0.6);
    }

    button {
      background: linear-gradient(135deg, #00ccff, #0099cc);
      color: #0d0d25;
      border: none;
      padding: 12px 25px;
      border-radius: 8px;
      font-size: 1rem;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 0 10px rgba(0, 204, 255, 0.4);
      margin: 10px;
    }

    button:hover {
      background: linear-gradient(135deg, #00aadd, #0077aa);
      box-shadow: 0 0 20px rgba(0, 204, 255, 0.6);
    }

    #message {
      margin: 15px 0;
      font-size: 1.1rem;
      min-height: 25px;
    }

    .hint {
      background: rgba(255, 170, 0, 0.1);
      border: 2px solid #ffaa00;
      border-radius: 8px;
      padding: 10px;
      margin: 15px 0;
      color: #ffaa00;
      display: none;
    }

    .hint.show {
      display: block;
    }

    .victory {
      display: none;
      position: fixed;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      background: linear-gradient(135deg, #1a1a3a, #2a2a4a);
      border: 5px solid #00ccff;
      border-radius: 16px;
      padding: 30px;
      text-align: center;
      z-index: 1000;
      box-shadow: 0 0 40px rgba(0, 204, 255, 0.8);
      max-width: 500px;
      width: 90%;
    }

    .victory.show {
      display: block;
    }

    .flag {
      background: rgba(0, 255, 204, 0.2);
      border: 3px solid #00ffcc;
      border-radius: 8px;
      padding: 15px;
      margin: 15px 0;
      font-size: 1rem;
      color: #00ffcc;
      word-wrap: break-word;
      overflow-wrap: break-word;
    }

    .overlay {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0, 0, 0, 0.7);
      z-index: 998;
    }

    .overlay.show {
      display: block;
    }

    .glow {
      animation: glowEffect 2s infinite alternate;
    }

    @keyframes glowEffect {
      from { text-shadow: 0 0 10px #00ffcc; }
      to { text-shadow: 0 0 20px #00ffcc, 0 0 30px #00ccff; }
    }

    @media(max-width: 768px) {
      .container { padding: 20px; }
      h1 { font-size: 1.8rem; }
      #sequence { font-size: 1.4rem; }
      input { width: 120px; margin: 10px 5px; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔢 SEQUENCE SOLVER</h1>
    
    <div class="stats">
      <div class="stat-box">
        <div class="stat-label">SEQUENCES</div>
        <div class="stat-value" id="progress">0/5</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">TIME LEFT</div>
        <div class="stat-value" id="timer">30</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">SCORE</div>
        <div class="stat-value" id="score">0</div>
      </div>
    </div>

    <div id="sequence" class="glow">2, 4, 6, 8, ?</div>
    
    <input type="number" id="answer" placeholder="Next number?">
    <br>
    <button id="submitBtn">SUBMIT</button>
    <button id="hintBtn">HINT</button>
    <button onclick="newGame()">NEW GAME</button>
    
    <div id="message"></div>
    <div class="hint" id="hintBox"></div>
  </div>

  <div class="overlay" id="overlay"></div>
  
  <div class="victory" id="victoryScreen">
    <h2 style="color:#00ccff">SEQUENCE MASTER!</h2>
    <p style="color:#00ffcc">Amazing! You solved all 5 sequences within 30 seconds!</p>
    <div class="flag" id="flagDisplay"></div>
    <p style="color:#ffaa00">Final Score: <span id="finalScore"></span> | Status: <span id="finalLevel"></span></p>
    <button onclick="newGame()">PLAY AGAIN</button>
  </div>

  <script>
    const sequences = [
      { seq: [2, 4, 6, 8], answer: 10, hint: "Even numbers (+2)" },
      { seq: [1, 4, 9, 16], answer: 25, hint: "Perfect squares (n²)" },
      { seq: [2, 3, 5, 7], answer: 11, hint: "Prime numbers" },
      { seq: [5, 10, 20, 40], answer: 80, hint: "Double each time (×2)" },
      { seq: [1, 1, 2, 3, 5], answer: 8, hint: "Fibonacci sequence" },
      { seq: [10, 20, 30, 40], answer: 50, hint: "Multiples of 10" },
      { seq: [3, 6, 12, 24], answer: 48, hint: "Multiply by 2" },
      { seq: [1, 2, 4, 8, 16], answer: 32, hint: "Powers of 2" },
      { seq: [1, 3, 5, 7], answer: 9, hint: "Odd numbers (+2)" },
      { seq: [0, 1, 4, 9, 16], answer: 25, hint: "Squares starting from 0" },
      { seq: [2, 6, 18, 54], answer: 162, hint: "Multiply by 3" },
      { seq: [1, 4, 7, 10], answer: 13, hint: "Add 3 each time" },
      { seq: [100, 81, 64, 49], answer: 36, hint: "Descending squares" },
      { seq: [1, 2, 6, 24], answer: 120, hint: "Factorials (n!)" },
      { seq: [3, 5, 9, 17], answer: 33, hint: "Double and add 1" }
    ];

    let sequencesSolved = 0;
    let timeLeft = 30;
    let timer;
    let currentAnswer;
    let currentHint;
    let gameSequences = [];
    let score = 0;

    const sequenceDisplay = document.getElementById('sequence');
    const answerInput = document.getElementById('answer');
    const submitBtn = document.getElementById('submitBtn');
    const hintBtn = document.getElementById('hintBtn');
    const message = document.getElementById('message');
    const progressDisplay = document.getElementById('progress');
    const timerDisplay = document.getElementById('timer');
    const scoreDisplay = document.getElementById('score');
    const hintBox = document.getElementById('hintBox');

    function startGame() {
      // Select 5 random sequences for this game
      gameSequences = shuffleArray([...sequences]).slice(0, 5);
      sequencesSolved = 0;
      timeLeft = 30;
      score = 0;
      updateDisplay();
      nextSequence();
      startTimer();
    }

    function shuffleArray(array) {
      const shuffled = [...array];
      for (let i = shuffled.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [shuffled[i], shuffled[j]] = [shuffled[j], shuffled[i]];
      }
      return shuffled;
    }

    function startTimer() {
      clearInterval(timer);
      timer = setInterval(() => {
        timeLeft--;
        timerDisplay.textContent = timeLeft;
        if (timeLeft <= 0) {
          clearInterval(timer);
          gameOver();
        }
      }, 1000);
    }

    function nextSequence() {
      if (sequencesSolved >= 5) {
        gameWon();
        return;
      }

      message.textContent = '';
      answerInput.value = '';
      hintBox.classList.remove('show');
      
      const seqData = gameSequences[sequencesSolved];
      currentAnswer = seqData.answer;
      currentHint = seqData.hint;
      sequenceDisplay.textContent = seqData.seq.join(', ') + ', ?';
    }

    submitBtn.addEventListener('click', checkAnswer);
    answerInput.addEventListener('keypress', function(e) {
      if (e.key === 'Enter') {
        checkAnswer();
      }
    });

    function checkAnswer() {
      const playerAnswer = parseInt(answerInput.value);
      if (isNaN(playerAnswer)) {
        message.textContent = "⚠️ Enter a valid number!";
        message.style.color = "#ffaa00";
        return;
      }
      
      if (playerAnswer === currentAnswer) {
        sequencesSolved++;
        score += Math.max(50, Math.floor(timeLeft * 2)); // More points for faster solving
        message.textContent = "✅ Correct!";
        message.style.color = "#00ff99";
        updateDisplay();
        
        setTimeout(() => {
          nextSequence();
        }, 800);
      } else {
        timeLeft = Math.max(0, timeLeft - 3); // 3 second penalty for wrong answer
        message.textContent = `❌ Wrong! -3 seconds. Answer was ${currentAnswer}`;
        message.style.color = "#ff6666";
        updateDisplay();
        setTimeout(() => {
          nextSequence();
        }, 1500);
      }
    }

    hintBtn.addEventListener('click', () => {
      hintBox.textContent = `Hint: ${currentHint}`;
      hintBox.classList.toggle('show');
      score = Math.max(0, score - 10); // Penalty for using hints
      updateDisplay();
    });

    function updateDisplay() {
      progressDisplay.textContent = sequencesSolved + '/5';
      scoreDisplay.textContent = score;
      timerDisplay.textContent = timeLeft;
    }

    function gameWon() {
      clearInterval(timer);
      document.getElementById('overlay').classList.add('show');
      const victory = document.getElementById('victoryScreen');
      victory.classList.add('show');
      document.getElementById('finalScore').textContent = score;
      document.getElementById('finalLevel').textContent = '5/5 COMPLETE';
      
      const flagDisplay = document.getElementById('flagDisplay');
      flagDisplay.innerHTML = document.getElementById('flagContainer').innerHTML;
    }

    function gameOver() {
      clearInterval(timer);
      alert(`⏰ Time's up! You solved ${sequencesSolved}/5 sequences. You need to solve all 5 within 30 seconds to unlock the flag!`);
      newGame();
    }

    function newGame() {
      document.getElementById('overlay').classList.remove('show');
      document.getElementById('victoryScreen').classList.remove('show');
      clearInterval(timer);
      startGame();
    }

    // Start the game when page loads
    startGame();
  </script>

  <!-- Server will replace the placeholders below -->
  <div id="flagContainer" style="display:none">{{FLAG}}<br><br>Next WiFi: <span style="color:#00ffcc">{{NEXT_WIFI}}</span> | WiFi Password: <span style="color:#00ffcc">{{WIFI_PASS}}</span><br><br>{{CLUE}}</div>
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