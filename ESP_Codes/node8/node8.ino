#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "lightning";
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
<title>Team Login - Challenge 8</title>
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

// Game page HTML - Code Breaker Challenge
const char PROGMEM game_html[] = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Code Breaking - Challenge 22</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #1a0a2e 0%, #0a1a3e 50%, #1a0a2e 100%);
            color: #00ccff;
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }

        .container {
            max-width: 800px;
            width: 100%;
            background: rgba(10, 10, 40, 0.95);
            border: 3px solid #00ccff;
            border-radius: 20px;
            padding: 35px;
            box-shadow: 0 0 40px rgba(0, 204, 255, 0.4), inset 0 0 30px rgba(0, 204, 255, 0.05);
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
            color: #00ff88;
            margin-bottom: 25px;
            font-size: 1.3em;
            text-shadow: 0 0 10px #00ff88;
        }

        .instructions {
            background: rgba(0, 204, 255, 0.1);
            border: 2px solid #00ccff;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 25px;
        }

        .instructions h2 {
            color: #ff00ff;
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
            color: #00ff88;
            font-size: 1.2em;
        }

        .cipher-box {
            background: rgba(255, 0, 255, 0.1);
            border: 3px solid #ff00ff;
            border-radius: 15px;
            padding: 30px;
            margin: 25px 0;
            text-align: center;
        }

        .cipher-label {
            color: #00ccff;
            font-size: 1.2em;
            margin-bottom: 15px;
            font-weight: bold;
        }

        .cipher-text {
            font-size: 3em;
            color: #ff00ff;
            letter-spacing: 15px;
            margin: 20px 0;
            text-shadow: 0 0 20px #ff00ff;
            font-weight: bold;
        }

        .helper-table {
            background: rgba(0, 255, 136, 0.1);
            border: 2px solid #00ff88;
            border-radius: 15px;
            padding: 25px;
            margin: 25px 0;
        }

        .helper-title {
            color: #00ff88;
            font-size: 1.3em;
            margin-bottom: 15px;
            text-align: center;
            font-weight: bold;
        }

        .conversion-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(80px, 1fr));
            gap: 10px;
            margin-top: 15px;
        }

        .conversion-item {
            background: rgba(0, 204, 255, 0.15);
            border: 1px solid #00ccff;
            border-radius: 8px;
            padding: 10px;
            text-align: center;
            transition: all 0.3s ease;
        }

        .conversion-item:hover {
            background: rgba(0, 204, 255, 0.3);
            transform: scale(1.05);
            box-shadow: 0 0 15px rgba(0, 204, 255, 0.5);
        }

        .number {
            color: #ff00ff;
            font-size: 1.3em;
            font-weight: bold;
        }

        .letter {
            color: #00ff88;
            font-size: 1.1em;
            margin-top: 5px;
        }

        .input-section {
            background: rgba(0, 204, 255, 0.1);
            border: 2px solid #00ccff;
            border-radius: 15px;
            padding: 25px;
            margin: 25px 0;
        }

        .input-label {
            color: #00ccff;
            font-size: 1.2em;
            margin-bottom: 15px;
            text-align: center;
            font-weight: bold;
        }

        input[type="text"] {
            width: 100%;
            padding: 20px;
            background: rgba(0, 0, 0, 0.6);
            border: 3px solid #00ff88;
            border-radius: 12px;
            color: #00ff88;
            font-family: 'Courier New', monospace;
            font-size: 1.5em;
            text-transform: uppercase;
            text-align: center;
            letter-spacing: 5px;
        }

        input[type="text"]:focus {
            outline: none;
            box-shadow: 0 0 25px rgba(0, 255, 136, 0.6);
            border-color: #00ff88;
        }

        .button-container {
            display: flex;
            gap: 15px;
            margin-top: 20px;
        }

        button {
            flex: 1;
            padding: 18px;
            background: linear-gradient(135deg, #ff00ff, #00ccff);
            border: none;
            border-radius: 12px;
            color: white;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            font-size: 1.1em;
        }

        button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 30px rgba(255, 0, 255, 0.6);
        }

        .hint-btn {
            background: linear-gradient(135deg, #ff9900, #ff0088);
        }

        .clear-btn {
            background: linear-gradient(135deg, #666, #999);
        }

        .feedback {
            text-align: center;
            margin-top: 20px;
            font-size: 1.3em;
            min-height: 35px;
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

        .hint-box {
            background: rgba(255, 170, 0, 0.1);
            border: 2px solid #ffaa00;
            border-radius: 12px;
            padding: 20px;
            margin-top: 20px;
            display: none;
            animation: slideDown 0.3s ease;
        }

        .hint-box.show {
            display: block;
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

        .hint-content {
            color: #ffaa00;
            text-align: center;
            font-size: 1.1em;
            line-height: 1.6;
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
            font-size: 1.8em;
            padding: 20px;
            background: rgba(0, 0, 0, 0.6);
            border-radius: 15px;
            border: 2px solid #00ff88;
            text-shadow: 0 0 20px #00ff88;
            font-weight: bold;
            letter-spacing: 3px;
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
                font-size: 1.8em;
                padding: 15px 10px;
            }

            .subtitle {
                font-size: 1em;
            }

            .container {
                padding: 15px;
                margin: 10px;
            }

            .instructions {
                padding: 15px;
                margin: 15px 0;
            }

            .instructions h2 {
                font-size: 1.3em;
            }

            .instructions ul {
                font-size: 0.95em;
            }

            .cipher-box {
                padding: 20px;
                margin: 15px 0;
            }

            .cipher-label {
                font-size: 1em;
            }

            .cipher-text {
                font-size: 1.5em !important;
                letter-spacing: 5px !important;
                word-wrap: break-word;
                line-height: 1.4;
            }

            .input-section {
                padding: 15px;
                margin: 15px 0;
            }

            .input-label {
                font-size: 1em;
            }

            input[type="text"] {
                padding: 15px;
                font-size: 1.2em;
                letter-spacing: 3px;
            }

            .button-container {
                flex-direction: column;
                gap: 10px;
            }

            button {
                padding: 15px;
                font-size: 1em;
            }

            .feedback {
                font-size: 1.1em;
                margin-top: 15px;
            }

            .hint-box {
                padding: 15px;
                margin-top: 15px;
            }

            .hint-content {
                font-size: 0.95em;
                line-height: 1.5;
            }

            .flag-display {
                padding: 25px;
                margin-top: 20px;
            }

            .flag-display h2 {
                font-size: 1.8em;
                margin-bottom: 15px;
            }

            .flag-value {
                font-size: 1.2em;
                padding: 15px;
                letter-spacing: 2px;
                word-wrap: break-word;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>CODE BREAKER</h1>
        <div class="subtitle">Challenge 22: Number Cipher Decoder</div>

        <div class="instructions">
            <h2>How to Play:</h2>
            <ul>
                <li>Decode the secret message using the cipher below</li>
                <li>Each number represents a letter position in the alphabet</li>
                <li>Use your knowledge: A=1, B=2, C=3... Z=26</li>
                <li>Enter the complete decoded message to reveal the FLAG!</li>
            </ul>
        </div>

        <div class="cipher-box">
            <div class="cipher-label">SECRET CODE TO DECODE:</div>
            <div class="cipher-text" id="cipher-text" style="font-size: 2em; letter-spacing: 8px;">3-18-1-3-11 20-8-5 3-15-4-5</div>
        </div>

        <div class="input-section">
            <div class="input-label">Enter Decoded Message:</div>
            <input type="text" id="answer-input" placeholder="TYPE YOUR ANSWER HERE" maxlength="20">
            <div class="button-container">
                <button onclick="checkAnswer()">DECODE & SUBMIT</button>
                <button class="hint-btn" onclick="showHint()">SHOW HINT</button>
                <button class="clear-btn" onclick="clearAnswer()">CLEAR</button>
            </div>
            <div class="feedback" id="feedback"></div>
            <div class="hint-box" id="hint-box">
                <div class="hint-content">
                    <strong>Hint:</strong><br>
                    Remember: A=1, B=2, C=3... and so on up to Z=26.<br>
                    The first number is 3, which represents the 3rd letter: C.<br>
                    Decode all numbers to reveal the secret phrase!
                </div>
            </div>
        </div>

        <div class="flag-display" id="flag-display" 
             data-flag="{{FLAG}}" 
             data-nextwifi="{{NEXT_WIFI}}" 
             data-wifipass="{{WIFI_PASS}}" 
             data-clue="{{CLUE}}">
            <h2>CODE CRACKED!</h2>
            <p style="color: #00ccff; margin-bottom: 20px; font-size: 1.3em;">You've successfully decoded the cipher!</p>
            <div class="flag-value" id="flag-value"></div>
        </div>
    </div>

    <script>
        function checkAnswer() {
            const input = document.getElementById('answer-input');
            const answer = input.value.trim().toUpperCase().replace(/\s+/g, '');
            const feedback = document.getElementById('feedback');

            if (!answer) {
                feedback.textContent = 'Please enter your answer!';
                feedback.className = 'feedback incorrect';
                return;
            }

            // The cipher "3-18-1-3-11 20-8-5 3-15-4-5" decodes to "CRACK THE CODE"
            const correctAnswers = ['CRACKTHECODE', 'CRACK THE CODE'];
            if (correctAnswers.includes(answer) || correctAnswers.includes(input.value.trim().toUpperCase())) {
                feedback.textContent = 'CORRECT! Code Cracked!';
                feedback.className = 'feedback correct';
                input.disabled = true;
                createConfetti();
                setTimeout(showFlag, 1000);
            } else {
                feedback.textContent = 'Incorrect! Try again...';
                feedback.className = 'feedback incorrect';
            }
        }

        function showHint() {
            const hintBox = document.getElementById('hint-box');
            hintBox.classList.add('show');
        }

        function clearAnswer() {
            document.getElementById('answer-input').value = '';
            document.getElementById('feedback').textContent = '';
            document.getElementById('feedback').className = 'feedback';
        }

        function createConfetti() {
            const colors = ['#ff00ff', '#00ff88', '#00ccff', '#ff0088', '#ffaa00'];
            for (let i = 0; i < 50; i++) {
                setTimeout(() => {
                    const confetti = document.createElement('div');
                    confetti.className = 'confetti';
                    confetti.style.left = Math.random() * 100 + '%';
                    confetti.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
                    confetti.style.animationDuration = (Math.random() * 2 + 2) + 's';
                    document.body.appendChild(confetti);

                    setTimeout(() => confetti.remove(), 3000);
                }, i * 20);
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

            // Extra confetti
            for (let i = 0; i < 3; i++) {
                setTimeout(createConfetti, i * 200);
            }
        }

        // Allow Enter key to submit
        document.getElementById('answer-input').addEventListener('keypress', function(e) {
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
