#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "ice";
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
<title>Team Login - Challenge 9</title>
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

// Game page HTML - Hex Decoder Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hex Decoder</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            background: linear-gradient(135deg, #0a1a2e, #1a2a3e);
            color: #00ccff;
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .container {
            max-width: 700px;
            text-align: center;
        }

        h1 {
            text-shadow: 0 0 15px #00ccff;
            margin-bottom: 20px;
            font-size: 2.5em;
        }

        .intro {
            background: #001122;
            border: 2px solid #00ccff;
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            line-height: 1.6;
        }

        .hex-display {
            background: #002233;
            border: 3px solid #00ccff;
            padding: 35px;
            border-radius: 15px;
            margin: 30px 0;
            font-size: 28px;
            letter-spacing: 4px;
            color: #ffaa00;
            font-weight: bold;
            text-shadow: 0 0 10px #ffaa00;
        }

        .conversion-help {
            background: #001122;
            border: 2px solid #00ccff;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            font-size: 14px;
        }

        .help-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 10px;
            margin-top: 15px;
        }

        .help-item {
            background: #002233;
            padding: 10px;
            border-radius: 5px;
            border: 1px solid #00ccff;
        }

        input {
            background: #001122;
            border: 2px solid #00ccff;
            color: #00ccff;
            padding: 20px;
            font-size: 22px;
            font-family: 'Courier New', monospace;
            width: 90%;
            max-width: 500px;
            text-align: center;
            border-radius: 10px;
            margin: 20px 0;
        }

        button {
            background: linear-gradient(45deg, #00ccff, #0088aa);
            border: none;
            color: #000;
            padding: 18px 40px;
            font-size: 20px;
            cursor: pointer;
            border-radius: 10px;
            font-weight: bold;
            margin: 10px;
        }

        button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 20px #00ccff;
        }

        .message {
            margin: 20px 0;
            font-size: 20px;
            min-height: 30px;
        }

        .flag {
            background: #001122;
            border: 2px solid #00ccff;
            padding: 25px;
            border-radius: 15px;
            margin-top: 30px;
            font-size: 22px;
            font-weight: bold;
            display: none;
        }

        .hint {
            background: #112200;
            border: 2px solid #00ff00;
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
            color: #00ff00;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔤 HEX DECODER</h1>
        
        <div class="intro">
            <strong>Mission:</strong> Decode the hexadecimal message!<br>
            Hex (base-16) uses digits 0-9 and letters A-F.<br>
            Convert the hex to text to reveal the secret word.
        </div>

        <div class="hex-display">
            49 4D 50 4F 53 54 45 52
        </div>

        <div class="conversion-help">
            <strong>Hexadecimal to ASCII Guide:</strong>
            <div class="help-grid">
                <div class="help-item">30 = 0</div>
                <div class="help-item">31 = 1</div>
                <div class="help-item">...</div>
                <div class="help-item">39 = 9</div>
                <div class="help-item">41 = A</div>
                <div class="help-item">42 = B</div>
                <div class="help-item">...</div>
                <div class="help-item">5A = Z</div>
            </div>
        </div>

        <div class="hint">
            💡 <strong>Hint:</strong> Each hex pair is a character.<br>
            Use an ASCII table or converter to decode all pairs.
        </div>

        <input type="text" id="answerInput" placeholder="Enter decoded message">
        <br>
        <button onclick="checkAnswer()">🔓 DECODE</button>

        <div class="message" id="message">What word does the hex spell?</div>

        <div class="flag" id="flag">
            🎉 Hex decoded successfully!<br><br>
            Flag: <span style="color: #ffaa00;">{{FLAG}}</span><br><br>
            Next WiFi: <span style=\"color: #00ffff;\">{{NEXT_WIFI}}</span> | WiFi Password: <span style=\"color: #00ffff;\">{{WIFI_PASS}}</span><br><br>
            Clue: <span style=\"color: #00ff88;\">{{CLUE}}</span>
        </div>
    </div>

    <script>
        // 49 4D 50 4F 53 54 45 52 = IMPOSTER
        const correctAnswer = "IMPOSTER";

        function checkAnswer() {
            const input = document.getElementById('answerInput');
            const message = document.getElementById('message');
            const flag = document.getElementById('flag');

            const userAnswer = input.value.toUpperCase().trim();

            if (userAnswer === correctAnswer) {
                message.textContent = '✅ Perfect! You decoded the hex message!';
                message.style.color = '#00ff00';
                flag.style.display = 'block';
                input.disabled = true;
            } else {
                message.textContent = '❌ Incorrect! Check each hex pair carefully.';
                message.style.color = '#ff0000';
            }
        }

        document.getElementById('answerInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                checkAnswer();
            }
        });
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