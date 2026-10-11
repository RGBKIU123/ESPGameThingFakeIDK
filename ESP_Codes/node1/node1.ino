#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "air";
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
<title>Team Login - Challenge 1</title>
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
</style>
</head>
<body>
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

// Game page HTML - now uses placeholders for team-specific data
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Memory Game - Challenge 1</title>
<style>
body{background:linear-gradient(135deg,#0f0f23,#1a1a2e);color:#00ff00;font-family:'Courier New',monospace;margin:0;padding:20px;min-height:100vh;display:flex;flex-direction:column;align-items:center}
.container{max-width:600px;text-align:center}
h1{text-shadow:0 0 10px #00ff00;margin-bottom:20px}
.game-board{display:grid;grid-template-columns:repeat(2,150px);gap:10px;justify-content:center;margin:30px 0}
.color-btn{width:150px;height:150px;border:3px solid #333;border-radius:10px;cursor:pointer;transition:all 0.3s}
.red{background-color:#ff0000}.blue{background-color:#0000ff}.green{background-color:#00ff00}.yellow{background-color:#ffff00}
.color-btn.active{transform:scale(1.1);box-shadow:0 0 20px currentColor}
.controls{margin:20px 0}
button{background:linear-gradient(45deg,#00ff00,#008800);border:none;color:black;padding:10px 20px;font-size:16px;cursor:pointer;border-radius:5px;font-weight:bold}
.status{font-size:18px;margin:20px 0;min-height:24px}
.flag{background:#001100;border:2px solid #00ff00;padding:15px;border-radius:10px;margin-top:20px;font-size:18px;font-weight:bold;display:none}
.clue{background:#001a1a;border:2px solid #00ffff;padding:10px;border-radius:5px;margin-top:10px;font-size:14px;color:#00ffff}
.logout{position:absolute;top:20px;right:20px;background:#880000;color:#ff0000;padding:8px 15px;border-radius:5px;text-decoration:none;font-size:14px}
</style>
</head>
<body>
<a href="/logout" class="logout">Logout</a>
<div class="container">
<h1>🧠 MEMORY SEQUENCE GAME</h1>
<p>Follow the pattern! Remember the sequence and repeat it.</p>
<div class="game-board">
<div class="color-btn red" data-color="red"></div>
<div class="color-btn blue" data-color="blue"></div>
<div class="color-btn green" data-color="green"></div>
<div class="color-btn yellow" data-color="yellow"></div>
</div>
<div class="controls">
<button onclick="startGame()">Start Game</button>
<button onclick="resetGame()">Reset</button>
</div>
<div class="status" id="status">Click Start to begin!</div>
<div class="flag" id="flag">🎉 Well done!
<br><br>Flag: <span style="color: #ffff00;">{{FLAG}}</span>
<br><br>Next WiFi: <span style="color: #00ffff;">{{NEXT_WIFI}}</span>
<br>WiFi Password: <span style="color: #00ffff;">{{WIFI_PASS}}</span>
<br><br>💡 Clue: <span style="color: #ffaa00;">{{CLUE}}</span>
</div>
</div>
<script>
let sequence=[],playerSequence=[],level=0,gameActive=false,showing=false;
const colors=['red','blue','green','yellow'];
const buttons=document.querySelectorAll('.color-btn');
const status=document.getElementById('status');
const flag=document.getElementById('flag');
buttons.forEach(btn=>{btn.addEventListener('click',()=>{if(!gameActive||showing)return;const color=btn.dataset.color;playerSequence.push(color);playSound(color);if(playerSequence[playerSequence.length-1]!==sequence[playerSequence.length-1]){gameOver();return}if(playerSequence.length===sequence.length){if(level>=8){winGame()}else{nextLevel()}}})});
function startGame(){sequence=[];level=0;gameActive=true;flag.style.display='none';nextLevel()}
function nextLevel(){level++;playerSequence=[];sequence.push(colors[Math.floor(Math.random()*colors.length)]);status.textContent=`Level ${level} - Watch the sequence!`;setTimeout(()=>{showSequence()},1000)}
function showSequence(){showing=true;let i=0;const interval=setInterval(()=>{if(i<sequence.length){flashButton(sequence[i]);i++}else{clearInterval(interval);showing=false;status.textContent='Your turn! Repeat the sequence.'}},800)}
function flashButton(color){const btn=document.querySelector(`[data-color="${color}"]`);btn.classList.add('active');playSound(color);setTimeout(()=>{btn.classList.remove('active')},400)}
function playSound(color){try{const audioContext=new(window.AudioContext||window.webkitAudioContext)();const oscillator=audioContext.createOscillator();const gainNode=audioContext.createGain();const frequencies={red:220,blue:277,green:330,yellow:440};oscillator.connect(gainNode);gainNode.connect(audioContext.destination);oscillator.frequency.value=frequencies[color];oscillator.type='sine';gainNode.gain.setValueAtTime(0.3,audioContext.currentTime);gainNode.gain.exponentialRampToValueAtTime(0.01,audioContext.currentTime+0.3);oscillator.start(audioContext.currentTime);oscillator.stop(audioContext.currentTime+0.3)}catch(e){}}
function gameOver(){gameActive=false;status.textContent='❌ Wrong! Try again.'}
function winGame(){gameActive=false;status.textContent='🎉 Congratulations! You completed all levels!';flag.style.display='block'}
function resetGame(){gameActive=false;sequence=[];playerSequence=[];level=0;status.textContent='Click Start to begin!';flag.style.display='none'}
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

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}