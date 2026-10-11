#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"
#include "login.h"

// Access Point credentials
const char* ssid = "metal";
const char* password = "projectcell";

WebServer server(80);

/*new*/ 
const char* getLoggingTeamID();
void logEvent(const char* eventName);
void sendHttpError(int statusCode, const char* message);
void redirectWithLog(const char* reason, const char* destination);
void handleBrowserEventRoute();
void handleUnknownRoute();
void printHeapStats(const char* label);

/*
void handleInvalidAccess(
  const char* reason,
  int statusCode,
  const char* message
);

void resetInvalidAccessCount(String ip);*/

/*end of new*/


// Session management - stores team index for each IP address
String authenticatedIPs[10]; // Support up to 10 simultaneous sessions
int teamIndexForIP[10];
int sessionCount = 0;


// Game page HTML - Letter Shift Decoder Challenge
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Letter Shift Game - Challenge 18</title>
    <style>
        body {
            background: linear-gradient(135deg, #1a0a2e, #2e1a3e);
            color: #bb88ff;
            font-family: 'Courier New', monospace;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .container {
            max-width: 800px;
            text-align: center;
        }

        h1 {
            text-shadow: 0 0 15px #bb88ff;
            margin-bottom: 20px;
        }

        .game-intro {
            background: #220033;
            border: 2px solid #bb88ff;
            padding: 25px;
            border-radius: 15px;
            margin: 20px 0;
            font-size: 16px;
            line-height: 1.6;
        }

        .secret-message {
            background: #330044;
            border: 3px solid #bb88ff;
            padding: 30px;
            border-radius: 15px;
            margin: 30px 0;
            font-size: 32px;
            letter-spacing: 5px;
            color: #ff88cc;
            font-weight: bold;
        }

        .decoder-slider {
            margin: 30px 0;
        }

        .slider-container {
            background: #220033;
            border: 2px solid #bb88ff;
            padding: 30px;
            border-radius: 15px;
        }

        input[type="range"] {
            width: 80%;
            height: 15px;
            background: #330044;
            outline: none;
            border-radius: 10px;
            margin: 20px 0;
        }

        input[type="range"]::-webkit-slider-thumb {
            width: 30px;
            height: 30px;
            background: #bb88ff;
            cursor: pointer;
            border-radius: 50%;
        }

        .shift-value {
            font-size: 48px;
            color: #ffaa88;
            margin: 20px 0;
        }

        .decoded-text {
            background: #002200;
            border: 2px solid #44ff44;
            padding: 30px;
            border-radius: 15px;
            margin: 30px 0;
            font-size: 32px;
            letter-spacing: 5px;
            color: #44ff44;
            min-height: 80px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        button {
            background: linear-gradient(45deg, #bb88ff, #9966cc);
            border: none;
            color: white;
            padding: 20px 40px;
            font-size: 20px;
            cursor: pointer;
            border-radius: 10px;
            font-weight: bold;
            margin: 20px;
            transition: all 0.3s;
        }

        button:hover {
            transform: scale(1.1);
            box-shadow: 0 0 20px #bb88ff;
        }

        .flag {
            background: #220033;
            border: 3px solid #bb88ff;
            padding: 30px;
            border-radius: 15px;
            margin-top: 30px;
            font-size: 24px;
            font-weight: bold;
            display: none;
            animation: victory 1s ease-in-out;
        }

        @keyframes victory {
            0% { transform: scale(0.8); opacity: 0; }
            100% { transform: scale(1); opacity: 1; }
        }

        .alphabet-helper {
            background: #112233;
            border: 2px solid #ffaa44;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            font-size: 14px;
        }

        .hint {
            color: #ffaa44;
            font-size: 18px;
            margin: 15px 0;
        }


        .input-section {
            margin: 30px 0;
        }

        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            75% { transform: translateX(10px); }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔤 LETTER SHIFT DECODER</h1>

        <div class="game-intro">
            <h2>🎮 How to Play:</h2>
            <p><strong>Mission:</strong> Decode the secret message!</p>
            <p><strong>Tip:</strong> Each letter has been shifted by a secret number</p>
            <p><strong>Goal:</strong> Type what you think the decoded message is!</p>
        </div>

        <div class="secret-message" id="secretMessage">
            WKH VHFUHW ZRUG LV FUBSWRJUDSKB
        </div>


        <div class="input-section">
            <h3>🔤 Enter Your Decoded Message</h3>
            <input type="text" id="answerInput" placeholder="Type the decoded message here..." style="
                width: 80%;
                padding: 20px;
                font-size: 20px;
                background: #220033;
                border: 2px solid #bb88ff;
                color: #bb88ff;
                border-radius: 10px;
                text-align: center;
                margin: 20px 0;
            ">
        </div>

        <div class="feedback" id="feedback" style="font-size: 20px; min-height: 40px; color: #ffaa44; margin: 20px 0;">
            Type your answer and click Submit!
        </div>

        <div class="alphabet-helper">
            <strong>Alphabet Helper:</strong><br>
            A B C D E F G H I J K L M N O P Q R S T U V W X Y Z<br>
            <small>Hint: Try shifting each letter backward or forward by the same amount!</small>
        </div>

        <button onclick="checkAnswer()">🚀 SUBMIT ANSWER</button>
        <button onclick="showHint()" style="background: linear-gradient(45deg, #ffaa44, #cc8822);">💡 GET HINT</button>

        <div class="flag" id="flag">
            🎉 MESSAGE DECODED PERFECTLY! 🎉<br><br>
            Flag: <span style="color: #ffff00;">{{FLAG}}</span><br><br>
            Next WiFi: <span style=\"color: #00ffff;\">{{NEXT_WIFI}}</span> | WiFi Password: <span style=\"color: #00ffff;\">{{WIFI_PASS}}</span><br><br>
            Clue: <span style=\"color: #00ff88;\">{{CLUE}}</span><br><br>
            <small>You discovered the Caesar Cipher with shift -3!</small>
        </div>
    </div>

    <script>
       /*new*/ 
function reportEvent(eventName) {
    fetch('/event', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: 'event=' + encodeURIComponent(eventName)
    }).catch(error => {
        console.error('Could not report event:', error);
    });
}
/*end of new*/
        // Encrypted: "WKH VHFUHW ZRUG LV FUBSWRJUDSKB" (shift +3)
        // Decrypted: "THE SECRET WORD IS CRYPTOGRAPHY"
        const correctAnswer = "THE SECRET WORD IS CRYPTOGRAPHY";
        const encryptedMessage = "WKH VHFUHW ZRUG LV FUBSWRJUDSKB";
        let hintsUsed = 0;

        function checkAnswer() {
            const userAnswer = document.getElementById('answerInput').value.toUpperCase().trim();

            if (userAnswer === correctAnswer) {
                /*new (Has to be put where game done)*/ reportEvent('Game completed');
                document.getElementById('feedback').textContent = '✅ PERFECT! You cracked the code!';
                document.getElementById('feedback').style.color = '#44ff44';
                document.getElementById('flag').style.display = 'block';
                document.getElementById('answerInput').disabled = true;

                // Celebration
                for (let i = 0; i < 30; i++) {
                    setTimeout(() => createConfetti(), i * 100);
                }
            } else {
                // Give helpful feedback
                let feedback = '❌ Incorrect! ';
                const similarity = calculateSimilarity(userAnswer, correctAnswer);

                if (similarity > 80) {
                    feedback += 'Very close! Check your spelling.';
                } else if (similarity > 50) {
                    feedback += 'You\'re on the right track!';
                } else if (similarity > 20) {
                    feedback += 'Some letters are correct. Keep trying!';
                } else {
                    feedback += 'Try a different shift amount.';
                }

                document.getElementById('feedback').textContent = feedback;
                document.getElementById('feedback').style.color = '#ff4444';

                // Shake animation
                const input = document.getElementById('answerInput');
                input.style.animation = 'shake 0.5s';
                setTimeout(() => {
                    input.style.animation = '';
                }, 500);
            }
        }

        function calculateSimilarity(str1, str2) {
            const longer = str1.length > str2.length ? str1 : str2;
            const shorter = str1.length > str2.length ? str2 : str1;
            const longerLength = longer.length;

            if (longerLength === 0) return 100;

            let matches = 0;
            for (let i = 0; i < shorter.length; i++) {
                if (shorter[i] === longer[i]) matches++;
            }

            return (matches / longerLength) * 100;
        }

        function showHint() {
            hintsUsed++;
            const hints = [
                '💡 Hint 1: Each letter is shifted by the SAME amount (Caesar Cipher)',
                '💡 Hint 2: Try shifting backward by 3 positions. W → T, K → H, H → E',
                '💡 Hint 3: The first word is "THE". Work from there!',
                '💡 Hint 4: The last word is "CRYPTOGRAPHY"'
            ];

            const hintIndex = Math.min(hintsUsed - 1, hints.length - 1);
            document.getElementById('feedback').textContent = hints[hintIndex];
            document.getElementById('feedback').style.color = '#ffaa44';
        }

        function createConfetti() {
            const confetti = document.createElement('div');
            confetti.textContent = ['🎉', '🎊', '⭐', '✨', '🔤'][Math.floor(Math.random() * 5)];
            confetti.style.position = 'fixed';
            confetti.style.left = Math.random() * window.innerWidth + 'px';
            confetti.style.top = '-50px';
            confetti.style.fontSize = '30px';
            confetti.style.zIndex = '1000';
            confetti.style.pointerEvents = 'none';
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

/*new*/
/*
// Route: Login page
void handleRoot() {
    logEvent("Login page requested");
  server.send_P(200, "text/html", login_html);
}

// Route: Handle login POST
void handleLogin() {
    logEvent("Login page requested");
    if (server.method() != HTTP_POST) {
    handleInvalidAccess(
        "Invalid method on /login",
        405,
        "Method Not Allowed"
    );
    return;
    }

  String teamId = server.arg("teamId");
  String pass = server.arg("password");

  int teamIdx = authenticateTeam(teamId, pass);

if (teamIdx >= 0) {
  String clientIP = server.client().remoteIP().toString();

  addSession(clientIP, teamIdx);
  resetInvalidAccessCount(clientIP);

  logEvent("Login successful");
  server.send(200, "text/plain", "OK");
} else {
  logEvent("Login failed");
  server.send(200, "text/plain", "FAIL");
}
}
//c:\Users\Admin\Downloads\ErrorHandling.ino
// Route: Game page (requires authentication)
void handleGame() {
  logEvent("Game page requested");
  String clientIP = server.client().remoteIP().toString();
  int teamIdx = getTeamIndex(clientIP);

if (teamIdx < 0) {
  handleInvalidAccess(
    "Unauthenticated access to /game",
    403,
    "Please log in first."
  );
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

  if (getTeamIndex(clientIP) < 0) {
    // Not logged in: just send them to the login page
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
    return;
  }

  logEvent("Logout requested");
  removeSession(clientIP);

  // Send the "logging out" page and make sure it is fully delivered
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", login_html);
  server.client().flush();

  Serial.println("[System] Restarting ESP32 after logout...");
  delay(1000);
  ESP.restart();
}
end of new*/

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
  /*new*/  server.on("/event", HTTP_POST, handleBrowserEventRoute); server.onNotFound(handleUnknownRoute);
}

void loop() {
  server.handleClient();
}
