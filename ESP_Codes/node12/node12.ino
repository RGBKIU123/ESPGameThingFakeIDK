#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "spirit";
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
<title>Team Login - Challenge 12</title>
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

// Game page HTML - WiFi Memory Game Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SQL Injection - Challenge 12</title>
    <style>
        body {
            background: linear-gradient(135deg, #1a0a2a, #2a1a3a);
            color: #cc88ff;
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
            text-shadow: 0 0 15px #cc88ff;
            margin-bottom: 20px;
        }

        .database {
            background: #110022;
            border: 3px solid #cc88ff;
            padding: 25px;
            border-radius: 15px;
            margin: 30px 0;
        }

        .query-box {
            background: #000000;
            border: 2px solid #cc88ff;
            padding: 15px;
            border-radius: 10px;
            margin: 15px 0;
            text-align: left;
            font-family: monospace;
            color: #00ff00;
        }

        .vulnerable {
            background: #220000;
            border-color: #ff0000;
        }

        input {
            background: #110022;
            border: 2px solid #cc88ff;
            color: #cc88ff;
            padding: 20px;
            font-size: 20px;
            font-family: 'Courier New', monospace;
            width: 90%;
            text-align: center;
            border-radius: 10px;
            margin: 20px 0;
        }

        button {
            background: linear-gradient(45deg, #cc88ff, #8844aa);
            border: none;
            color: #000;
            padding: 18px 40px;
            font-size: 20px;
            cursor: pointer;
            border-radius: 10px;
            font-weight: bold;
        }

        .flag {
            background: #110022;
            border: 2px solid #cc88ff;
            padding: 25px;
            border-radius: 15px;
            margin-top: 30px;
            font-size: 22px;
            display: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>💉 SQL INJECTION DETECTOR</h1>
        
        <div style="background: #110022; border: 2px solid #cc88ff; padding: 20px; border-radius: 15px; margin: 20px 0;">
            <strong>Mission:</strong> Identify the SQL injection attack!<br>
            Find the malicious query.
        </div>

        <div class="database">
            <h3>Database Query Log:</h3>
            
            <div class="query-box">
                SELECT * FROM users WHERE id = 1;
            </div>
            
            <div class="query-box">
                SELECT * FROM users WHERE name = '' OR '1'='1';
            </div>
            
            <div class="query-box">
                SELECT email FROM users WHERE active = true;
            </div>

            <div class="query-box">
                SELECT * FROM orders WHERE user_id = 42;
            </div>

            <div class="query-box">
                SELECT COUNT(*) FROM sessions WHERE timeout > NOW();
            </div>

            <div class="query-box">
                DELETE FROM logs WHERE date < '2024-01-01';
            </div>
        </div>

        <div style="background: #112200; border: 2px solid #00ff00; padding: 15px; border-radius: 10px; margin: 20px 0; color: #00ff00;">
            💡 <strong>Hint:</strong> SQL injection often uses tautologies (always true conditions).
        </div>

        <input type="text" id="answerInput" placeholder="Enter the SQL injection pattern">
        <br>
        <button onclick="checkAnswer()">🛡️ IDENTIFY ATTACK</button>

        <div style="margin: 20px 0; font-size: 20px;" id="message">Find the injection pattern...</div>

        <div class="flag" id="flag">
            🎉 SQL Injection blocked!<br><br>
            Flag: <span style="color: #ffff00;">{{FLAG}}</span><br><br>
            Next WiFi: <span style=\"color: #00ffff;\">{{NEXT_WIFI}}</span> | WiFi Password: <span style=\"color: #00ffff;\">{{WIFI_PASS}}</span><br><br>
            Clue: <span style=\"color: #00ff88;\">{{CLUE}}</span>
        </div>
    </div>

    <script>
        function checkAnswer() {
            const answer = document.getElementById('answerInput').value.toLowerCase().trim();
            const message = document.getElementById('message');
            
            if (answer.includes("1=1") || answer.includes("1'='1") || answer.includes("or 1=1") || answer.includes("'1'='1'")) {
                message.textContent = '✅ Correct! SQL injection pattern identified!';
                message.style.color = '#00ff00';
                document.getElementById('flag').style.display = 'block';
            } else if (answer.includes("or")) {
                message.textContent = '❌ Close! But be more specific about the condition.';
                message.style.color = '#ff0000';
            } else {
                message.textContent = '❌ Incorrect! Look for always-true conditions.';
                message.style.color = '#ff0000';
            }
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