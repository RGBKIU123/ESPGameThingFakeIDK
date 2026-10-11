#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "light";
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
<title>Team Login - Challenge 10</title>
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

// Game page HTML - File Detective Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Password Cracker - Challenge 1</title>
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
            max-width: 700px;
            text-align: center;
        }

        h1 {
            text-shadow: 0 0 10px #00ff00;
            margin-bottom: 20px;
            font-size: 2.5em;
        }

        .intro {
            background: #002200;
            border: 2px solid #00ff00;
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            font-size: 16px;
            line-height: 1.6;
        }

        .password-hint {
            background: #001100;
            border: 2px solid #00ff00;
            padding: 25px;
            border-radius: 15px;
            margin: 30px 0;
        }

        .hint-item {
            margin: 15px 0;
            padding: 15px;
            background: #002200;
            border-left: 4px solid #00ff00;
            text-align: left;
            border-radius: 5px;
        }

        .hint-label {
            color: #ffff00;
            font-weight: bold;
            margin-bottom: 5px;
        }

        .password-input {
            background: #001100;
            border: 2px solid #00ff00;
            color: #00ff00;
            padding: 20px;
            font-size: 24px;
            font-family: 'Courier New', monospace;
            width: 100%;
            max-width: 400px;
            text-align: center;
            border-radius: 10px;
            margin: 20px 0;
            letter-spacing: 5px;
        }

        button {
            background: linear-gradient(45deg, #00ff00, #008800);
            border: none;
            color: black;
            padding: 18px 40px;
            font-size: 20px;
            cursor: pointer;
            border-radius: 10px;
            font-weight: bold;
            margin: 10px;
            transition: all 0.3s;
        }

        button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 20px #00ff00;
        }

        .message {
            margin: 20px 0;
            font-size: 20px;
            min-height: 30px;
            font-weight: bold;
        }

        .flag {
            background: #001100;
            border: 2px solid #00ff00;
            padding: 25px;
            border-radius: 15px;
            margin-top: 30px;
            font-size: 22px;
            font-weight: bold;
            display: none;
            animation: reveal 1s ease-in-out;
        }

        @keyframes reveal {
            0% { transform: scale(0.8); opacity: 0; }
            100% { transform: scale(1); opacity: 1; }
        }

        .attempts {
            color: #ffff00;
            font-size: 18px;
            margin: 10px 0;
        }

        .security-bar {
            width: 100%;
            height: 20px;
            background: #001100;
            border: 2px solid #00ff00;
            border-radius: 10px;
            margin: 20px 0;
            overflow: hidden;
        }

        .security-fill {
            height: 100%;
            background: linear-gradient(90deg, #ff0000, #ffff00, #00ff00);
            width: 0%;
            transition: width 0.5s;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔐 PASSWORD CRACKER</h1>
        
        <div class="intro">
            <strong>Mission:</strong> Break into the secure system!<br>
            Use the clues below to crack the 4-digit password.
        </div>

        <div class="password-hint">
            <h2 style="color: #ffff00; margin-bottom: 20px;">🔍 Password Clues:</h2>
            
            <div class="hint-item">
                <div class="hint-label">Clue 1:</div>
                <div>First digit: HTTP response code for "Found" (divided by 100)</div>
            </div>
            
            <div class="hint-item">
                <div class="hint-label">Clue 2:</div>
                <div>Second digit: TCP uses this protocol number in IP headers</div>
            </div>
            
            <div class="hint-item">
                <div class="hint-label">Clue 3:</div>
                <div>Third digit: Standard SSH port divided by 22</div>
            </div>
            
            <div class="hint-item">
                <div class="hint-label">Clue 4:</div>
                <div>Fourth digit: IPv6 uses this many bits per hextet divided by 2</div>
            </div>
        </div>

        <div class="attempts" id="attempts">Attempts: 0 / 5</div>
        
        <div class="security-bar">
            <div class="security-fill" id="securityFill"></div>
        </div>

        <input type="text" class="password-input" id="passwordInput" placeholder="?????" maxlength="4" pattern="[0-9]*">
        
        <br>
        <button onclick="crackPassword()">🚀 CRACK PASSWORD</button>
        <button onclick="resetGame()" style="background: linear-gradient(45deg, #666, #444);">🔄 RESET</button>

        <div class="message" id="message">Enter the 4-digit password...</div>

        <div class="flag" id="flag">
            🎉 ACCESS GRANTED! 🎉<br><br>
            Flag: <span style="color: #ffff00;">{{FLAG}}</span><br><br>
            Next WiFi: <span style=\"color: #00ffff;\">{{NEXT_WIFI}}</span> | WiFi Password: <span style=\"color: #00ffff;\">{{WIFI_PASS}}</span><br><br>
            Clue: <span style=\"color: #00ff88;\">{{CLUE}}</span><br><br>
            <small>The password was: 3618</small>
        </div>
    </div>

    <script>
        // HTTP 302 / 100 = 3
        // TCP = protocol 6
        // SSH port 22 / 22 = 1
        // IPv6 hextet 16 bits / 2 = 8
        // Password: 3618
        const correctPassword = "3618";
        let attempts = 0;
        const maxAttempts = 5;

        function crackPassword() {
            const input = document.getElementById('passwordInput');
            const message = document.getElementById('message');
            const flag = document.getElementById('flag');
            const userPassword = input.value.trim();

            if (userPassword.length !== 4) {
                message.textContent = '⚠️ Password must be 4 digits!';
                message.style.color = '#ff0000';
                return;
            }

            attempts++;
            updateAttempts();

            if (userPassword === correctPassword) {
                message.textContent = '✅ PASSWORD CRACKED! Access Granted!';
                message.style.color = '#00ff00';
                flag.style.display = 'block';
                input.disabled = true;
                
                // Fill security bar to 100%
                document.getElementById('securityFill').style.width = '100%';
                
                // Celebration
                for (let i = 0; i < 20; i++) {
                    setTimeout(() => createConfetti(), i * 100);
                }
            } else {
                if (attempts >= maxAttempts) {
                    message.textContent = '🔒 System Locked! Too many attempts. Click RESET to try again.';
                    message.style.color = '#ff0000';
                    input.disabled = true;
                } else {
                    // Give hints based on how close they are
                    let correct = 0;
                    for (let i = 0; i < 4; i++) {
                        if (userPassword[i] === correctPassword[i]) correct++;
                    }
                    
                    message.textContent = `❌ Wrong! ${correct} digit(s) are correct and in the right position.`;
                    message.style.color = '#ff0000';
                    
                    // Update security bar
                    const progress = (correct / 4) * 100;
                    document.getElementById('securityFill').style.width = progress + '%';
                }
            }
        }

        function updateAttempts() {
            const attemptsDiv = document.getElementById('attempts');
            attemptsDiv.textContent = `Attempts: ${attempts} / ${maxAttempts}`;
            
            if (attempts >= maxAttempts - 1) {
                attemptsDiv.style.color = '#ff0000';
            } else if (attempts >= 3) {
                attemptsDiv.style.color = '#ffff00';
            }
        }

        function resetGame() {
            attempts = 0;
            document.getElementById('passwordInput').value = '';
            document.getElementById('passwordInput').disabled = false;
            document.getElementById('message').textContent = 'Enter the 4-digit password...';
            document.getElementById('message').style.color = '#00ff00';
            document.getElementById('flag').style.display = 'none';
            document.getElementById('securityFill').style.width = '0%';
            updateAttempts();
            document.getElementById('attempts').style.color = '#ffff00';
        }

        function createConfetti() {
            const confetti = document.createElement('div');
            confetti.textContent = ['🎉', '⭐', '✨', '🔓'][Math.floor(Math.random() * 4)];
            confetti.style.position = 'fixed';
            confetti.style.left = Math.random() * window.innerWidth + 'px';
            confetti.style.top = '-50px';
            confetti.style.fontSize = '30px';
            confetti.style.zIndex = '1000';
            confetti.style.animation = 'fall 3s linear forwards';
            document.body.appendChild(confetti);
            setTimeout(() => confetti.remove(), 3000);
        }

        const style = document.createElement('style');
        style.textContent = `
            @keyframes fall {
                to {
                    transform: translateY(${window.innerHeight + 100}px) rotate(360deg);
                    opacity: 0;
                }
            }
        `;
        document.head.appendChild(style);

        // Allow Enter key to submit
        document.getElementById('passwordInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                crackPassword();
            }
        });

        // Only allow numbers
        document.getElementById('passwordInput').addEventListener('input', function(e) {
            this.value = this.value.replace(/[^0-9]/g, '');
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