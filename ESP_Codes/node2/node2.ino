#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "water";
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
<title>Team Login - Challenge 2</title>
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
    <title>Binary Converter - Challenge 6</title>
    <style>
        body {
            background: linear-gradient(135deg, #0f0f23, #1a1a2e);
            color: #00ff00;
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .container {
            max-width: 600px;
            text-align: center;
        }

        h1 {
            text-shadow: 0 0 10px #00ff00;
            margin-bottom: 20px;
        }

        .binary-display {
            background: #002200;
            border: 2px solid #00ff00;
            padding: 30px;
            border-radius: 15px;
            margin: 30px 0;
            font-size: 28px;
            letter-spacing: 4px;
            font-weight: bold;
            color: #ffff00;
            text-shadow: 0 0 5px #ffff00;
        }

        .conversion-table {
            background: #001a1a;
            border: 1px solid #00ffff;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            font-size: 14px;
        }

        .table-row {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 20px;
            margin: 5px 0;
            padding: 5px;
        }

        .table-header {
            color: #00ffff;
            font-weight: bold;
            border-bottom: 1px solid #00ffff;
            padding-bottom: 5px;
        }

        .input-section {
            margin: 20px 0;
        }

        input {
            background: #001100;
            border: 2px solid #00ff00;
            color: #00ff00;
            padding: 15px;
            font-size: 18px;
            font-family: 'Courier New', monospace;
            width: 200px;
            text-align: center;
            border-radius: 5px;
        }

        button {
            background: linear-gradient(45deg, #00ff00, #008800);
            border: none;
            color: black;
            padding: 15px 25px;
            font-size: 18px;
            cursor: pointer;
            border-radius: 5px;
            font-weight: bold;
            margin-left: 15px;
        }

        .flag {
            background: #001100;
            border: 2px solid #00ff00;
            padding: 15px;
            border-radius: 10px;
            margin-top: 20px;
            font-size: 20px;
            font-weight: bold;
            display: none;
        }

        .message {
            margin: 15px 0;
            font-size: 18px;
            min-height: 24px;
        }

        .hint {
            background: #002200;
            border: 1px solid #00ff00;
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
            font-style: italic;
        }

        .logout{position:absolute;top:20px;right:20px;background:#880000;color:#ff0000;padding:8px 15px;border-radius:5px;text-decoration:none;font-size:14px}
    </style>
</head>
<body>
    <a href="/logout" class="logout">Logout</a>
    <div class="container">
        <h1>🔢 BINARY DECODER</h1>
        <p>Convert the binary code to reveal the hidden message!</p>

        <div class="binary-display">
            01001000 01000101 01001100 01001100 01001111
        </div>

        <div class="conversion-table">
            <div class="table-row table-header">
                <div>Letter</div>
                <div>ASCII</div>
                <div>Binary</div>
            </div>
            <div class="table-row">
                <div>A</div>
                <div>65</div>
                <div>01000001</div>
            </div>
            <div class="table-row">
                <div>B</div>
                <div>66</div>
                <div>01000010</div>
            </div>
            <div class="table-row">
                <div>H</div>
                <div>72</div>
                <div>01001000</div>
            </div>
            <div class="table-row">
                <div>...</div>
                <div>...</div>
                <div>...</div>
            </div>
        </div>

        <div class="hint">
            <strong>💡 Hint:</strong> Each 8-digit binary number represents one letter.<br>
            Convert each binary to decimal, then find the corresponding ASCII character.
        </div>

        <div class="input-section">
            <input type="text" id="answerInput" placeholder="Enter decoded message">
            <button type="button" onclick="checkAnswer()">Decode</button>
        </div>

        <div class="message" id="message">What word does the binary spell?</div>

        <div class="flag" id="flag">
            🎉 Binary master!
            <br><br>Flag: <span style="color: #ffff00;">{{FLAG}}</span>
            <br><br>Next WiFi: <span style="color: #00ffff;">{{NEXT_WIFI}}</span>
            <br>WiFi Password: <span style="color: #00ffff;">{{WIFI_PASS}}</span>
            <br><br>💡 Clue: <span style="color: #ffaa00;">{{CLUE}}</span>
        </div>
    </div>

    <script>
        // Binary: 01001000 01000101 01001100 01001100 01001111
        // Decimal: 72 69 76 76 79
        // ASCII: H E L L O
        const correctAnswer = "HELLO";

        function checkAnswer() {
            const input = document.getElementById('answerInput');
            const message = document.getElementById('message');
            const flag = document.getElementById('flag');

            const userAnswer = input.value.toUpperCase().trim();

            if (userAnswer === correctAnswer) {
                message.textContent = '🎉 Perfect! You decoded the binary message!';
                message.style.color = '#ffff00';
                flag.style.display = 'block';
                showConversion();
            } else {
                message.textContent = '❌ Try again! Convert each binary group to a letter.';
                message.style.color = '#ff0000';
            }
        }

        function showConversion() {
            const display = document.querySelector('.binary-display');
            setTimeout(() => {
                display.innerHTML = `
                    <div style="font-size: 16px; margin-bottom: 10px;">
                        01001000 → 72 → H<br>
                        01000101 → 69 → E<br>
                        01001100 → 76 → L<br>
                        01001100 → 76 → L<br>
                        01001111 → 79 → O
                    </div>
                    <div style="font-size: 28px; color: #ffff00;">HELLO</div>
                `;
            }, 1000);
        }

        // Allow Enter key to submit
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

void consumeMemory() {
  static void *blocks[100];
  static int count = 0; static int i=20;

  if (count < 100) {
    blocks[count] = malloc(5120);

    if (blocks[count] != NULL) {
      count++; i--;
      Serial.printf("Allocated: %d KB\n %d\n", count * 10, i);
    } else {
      Serial.println("MEMORY FULL!");
    }
  }
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

void loop() 
{
  server.handleClient();

    static unsigned long last = 0;

  if (millis() - last >= 1000) {
    last = millis();
    consumeMemory();
  }
}
