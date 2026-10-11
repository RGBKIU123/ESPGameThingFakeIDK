#include <WiFi.h>
#include <WebServer.h>
#include "teams.h"

// Access Point credentials
const char* ssid = "GOAL";
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
<title>Team Login - Challenge 15</title>
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

// Game page HTML - Final Boss Quiz Challenge (copied from node14)
const char PROGMEM game_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Challenge 28: Final Boss Quiz</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: 'Courier New', monospace;
      background: linear-gradient(135deg, #0a0a0a 0%, #1a0a2e 100%);
      color: #00ff88;
      min-height: 100vh;
      padding: 20px;
      overflow-x: hidden;
    }

    .container {
      max-width: 800px;
      margin: 0 auto;
      background: rgba(0, 0, 0, 0.8);
      border: 2px solid #00ff88;
      border-radius: 15px;
      padding: 30px;
      box-shadow: 0 0 30px rgba(0, 255, 136, 0.3);
    }

    h1 {
      text-align: center;
      color: #ff00ff;
      font-size: 2.5em;
      margin-bottom: 20px;
      text-shadow: 0 0 20px #ff00ff;
      animation: glow 2s ease-in-out infinite alternate;
    }

    @keyframes glow {
      from { text-shadow: 0 0 10px #ff00ff, 0 0 20px #ff00ff; }
      to { text-shadow: 0 0 20px #ff00ff, 0 0 30px #ff00ff, 0 0 40px #ff00ff; }
    }

    .instructions {
      background: rgba(138, 43, 226, 0.2);
      border: 2px solid #8a2be2;
      border-radius: 10px;
      padding: 20px;
      margin-bottom: 25px;
    }

    .instructions h2 {
      color: #8a2be2;
      margin-bottom: 10px;
    }

    .instructions p {
      color: #00ffff;
      line-height: 1.6;
      margin-bottom: 10px;
    }

    .progress-container {
      background: rgba(0, 0, 0, 0.6);
      border: 2px solid #00ff88;
      border-radius: 10px;
      padding: 20px;
      margin-bottom: 20px;
    }

    .progress-label {
      color: #00ffff;
      font-size: 1.1em;
      margin-bottom: 10px;
      text-align: center;
    }

    .progress-bar {
      width: 100%;
      height: 30px;
      background: rgba(0, 0, 0, 0.8);
      border: 2px solid #8a2be2;
      border-radius: 15px;
      overflow: hidden;
      position: relative;
    }

    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #8a2be2 0%, #ff00ff 50%, #00ffff 100%);
      width: 0%;
      transition: width 0.5s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-weight: bold;
      box-shadow: 0 0 20px rgba(138, 43, 226, 0.6);
    }

    .stats {
      display: flex;
      justify-content: space-around;
      margin-bottom: 20px;
      flex-wrap: wrap;
      gap: 15px;
    }

    .stat-box {
      background: rgba(255, 0, 255, 0.2);
      border: 2px solid #ff00ff;
      border-radius: 10px;
      padding: 15px 25px;
      text-align: center;
      min-width: 150px;
    }

    .stat-label {
      color: #ff00ff;
      font-size: 0.9em;
      margin-bottom: 5px;
    }

    .stat-value {
      color: #00ff88;
      font-size: 1.8em;
      font-weight: bold;
    }

    .quiz-container {
      background: rgba(0, 0, 0, 0.6);
      border: 2px solid #00ff88;
      border-radius: 10px;
      padding: 30px;
      margin-bottom: 20px;
      min-height: 350px;
    }

    .question {
      color: #00ffff;
      font-size: 1.4em;
      margin-bottom: 30px;
      line-height: 1.5;
    }

    .options {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }

    .option {
      background: rgba(138, 43, 226, 0.3);
      border: 2px solid #8a2be2;
      border-radius: 10px;
      padding: 20px;
      cursor: pointer;
      transition: all 0.3s;
      color: #00ff88;
      font-size: 1.1em;
    }

    .option:hover:not(.disabled) {
      transform: translateX(10px);
      background: rgba(138, 43, 226, 0.5);
      border-color: #ff00ff;
      box-shadow: 0 0 20px rgba(138, 43, 226, 0.6);
    }

    .option.disabled {
      cursor: not-allowed;
      opacity: 0.6;
    }

    .option.correct {
      background: rgba(0, 255, 0, 0.3);
      border-color: #00ff00;
      animation: correctPulse 0.5s;
    }

    @keyframes correctPulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.05); }
    }

    .option.incorrect {
      background: rgba(255, 0, 0, 0.3);
      border-color: #ff0000;
      animation: shake 0.5s;
    }

    @keyframes shake {
      0%, 100% { transform: translateX(0); }
      25% { transform: translateX(-10px); }
      75% { transform: translateX(10px); }
    }

    .buttons {
      display: flex;
      gap: 15px;
      justify-content: center;
      flex-wrap: wrap;
    }

    button {
      background: linear-gradient(135deg, #8a2be2 0%, #ff00ff 100%);
      color: white;
      border: none;
      padding: 15px 30px;
      font-size: 1.1em;
      border-radius: 10px;
      cursor: pointer;
      font-family: 'Courier New', monospace;
      transition: all 0.3s;
      box-shadow: 0 0 15px rgba(138, 43, 226, 0.5);
    }

    button:hover {
      transform: translateY(-3px);
      box-shadow: 0 0 25px rgba(138, 43, 226, 0.8);
    }

    button:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .hint-box {
      background: rgba(255, 215, 0, 0.1);
      border: 2px solid #ffd700;
      border-radius: 10px;
      padding: 15px;
      margin-top: 20px;
      display: none;
    }

    .hint-box.show {
      display: block;
      animation: fadeIn 0.5s;
    }

    @keyframes fadeIn {
      from { opacity: 0; }
      to { opacity: 1; }
    }

    .hint-box h3 {
      color: #ffd700;
      margin-bottom: 10px;
    }

    .hint-box p {
      color: #ffff00;
    }

    .victory {
      background: rgba(0, 255, 0, 0.2);
      border: 2px solid #00ff00;
      border-radius: 10px;
      padding: 30px;
      text-align: center;
      display: none;
      margin-top: 20px;
    }

    .victory.show {
      display: block;
      animation: victoryPulse 1s ease-in-out;
    }

    @keyframes victoryPulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.05); }
    }

    .victory h2 {
      color: #00ff00;
      font-size: 2.5em;
      margin-bottom: 20px;
    }

    .flag {
      background: rgba(0, 0, 0, 0.8);
      border: 2px solid #00ff88;
      border-radius: 10px;
      padding: 20px;
      font-size: 1.5em;
      color: #00ff88;
      margin: 20px 0;
      word-wrap: break-word;
      overflow-wrap: break-word;
    }

    .failure {
      background: rgba(255, 0, 0, 0.2);
      border: 2px solid #ff0000;
      border-radius: 10px;
      padding: 30px;
      text-align: center;
      display: none;
      margin-top: 20px;
    }

    .failure.show {
      display: block;
      animation: fadeIn 0.5s;
    }

    .failure h2 {
      color: #ff0000;
      font-size: 2em;
      margin-bottom: 20px;
    }

    .confetti {
      position: fixed;
      width: 10px;
      height: 10px;
      top: -10px;
      z-index: 9999;
      animation: confettiFall 3s linear;
    }

    @keyframes confettiFall {
      to {
        transform: translateY(100vh) rotate(360deg);
        opacity: 0;
      }
    }

    @media (max-width: 600px) {
      h1 {
        font-size: 1.8em;
      }

      .question {
        font-size: 1.2em;
      }

      .option {
        font-size: 1em;
        padding: 15px;
      }

      .stat-box {
        min-width: 120px;
        padding: 10px 15px;
      }
    }
  </style>
  <script>
    function setFlag(flag){
      const el = document.getElementById('flagBox');
      if (el) el.textContent = flag;
    }
  </script>
    
</head>
<body>
  <div class="container">
    <h1>FINAL BOSS CHALLENGE</h1>

    <div class="instructions">
      <h2>HOW TO PLAY:</h2>
      <p>Answer 10 multiple choice questions</p>
      <p>Questions are about computers, internet, and tech culture</p>
      <p>Get 7 or more correct to win</p>
      <p>Each correct answer gives you points!</p>
      <p>Complete the ultimate challenge!</p>
    </div>

    <div class="progress-container">
      <div class="progress-label">Question Progress</div>
      <div class="progress-bar">
        <div class="progress-fill" id="progressFill">0/10</div>
      </div>
    </div>

    <div class="stats">
      <div class="stat-box">
        <div class="stat-label">CORRECT</div>
        <div class="stat-value" id="correctCount">0</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">REMAINING</div>
        <div class="stat-value" id="remainingCount">10</div>
      </div>
      <div class="stat-box">
        <div class="stat-label">SCORE</div>
        <div class="stat-value" id="score">0</div>
      </div>
    </div>

    <div class="quiz-container" id="quizContainer">
      <div class="question" id="questionText">Press START QUIZ to begin the final challenge!</div>
      <div class="options" id="optionsContainer"></div>
    </div>

    <div class="buttons">
      <button onclick="startQuiz()" id="startBtn">START QUIZ</button>
      <button onclick="nextQuestion()" id="nextBtn" style="display: none;">NEXT QUESTION</button>
      <button onclick="toggleHint()" id="hintBtn">SHOW HINT</button>
    </div>

    <div class="hint-box" id="hintBox">
      <h3>HINT:</h3>
      <p>Don't worry if you don't know every answer! You only need 7 out of 10 correct. Think about what makes sense and use common knowledge. Some questions might seem tricky, but go with your gut feeling!</p>
    </div>

    <div class="victory" id="victoryBox">
      <h2>ULTIMATE CHAMPION!</h2>
      <p style="color: #00ffff; font-size: 1.3em; margin-bottom: 20px;">
        You've conquered the final challenge!
      </p>
      <p style="color: #ffff00; font-size: 1.1em; margin-bottom: 20px;">
        Score: <span id="finalScore">0</span>/10
      </p>
      <div class="flag" id="flagBox">{{FLAG}}</div>
      <p style="color: #00ff88; margin-top: 15px; font-size: 1.1em;">{{CLUE}}</p>
      <p style="color: #ff00ff; margin-top: 20px; font-size: 1.2em;">CONGRATULATIONS! YOU'VE COMPLETED THE TREASURE HUNT!</p>
    </div>

    <div class="failure" id="failureBox">
      <h2>SO CLOSE!</h2>
      <p style="color: #ff6666; font-size: 1.2em; margin-bottom: 20px;">
        You got <span id="failScore">0</span>/10 correct.
      </p>
      <p style="color: #ffff00; font-size: 1.1em;">
        You need 7 or more to win. Try again!
      </p>
    </div>
  </div>

  <script>
    const questions = [
      {
        question: "What does WWW stand for?",
        options: ["World Wide Web", "Worldwide Wireless Web", "Western World Web"],
        correct: 0
      },
      {
        question: "What color is commonly associated with hackers in movies?",
        options: ["Red", "Green", "Blue"],
        correct: 1
      },
      {
        question: "What is the most popular search engine?",
        options: ["Bing", "Yahoo", "Google"],
        correct: 2
      },
      {
        question: "What does USB stand for?",
        options: ["Universal Serial Bus", "United System Base", "Ultra Speed Battery"],
        correct: 0
      },
      {
        question: "Which of these is a web browser?",
        options: ["Microsoft Word", "Chrome", "Photoshop"],
        correct: 1
      },
      {
        question: "What is a computer 'virus'?",
        options: ["A hardware component", "Harmful software", "A type of cable"],
        correct: 1
      },
      {
        question: "What does 'WiFi' allow you to do?",
        options: ["Print faster", "Connect to internet wirelessly", "Charge your phone"],
        correct: 1
      },
      {
        question: "What is a 'password' used for?",
        options: ["To protect your accounts", "To speed up your computer", "To download files"],
        correct: 0
      },
      {
        question: "What does the 'cloud' in 'cloud storage' refer to?",
        options: ["Weather patterns", "Storing data on remote servers", "A physical storage box"],
        correct: 1
      },
      {
        question: "What is 'spam' in email?",
        options: ["Important messages", "Unwanted junk messages", "Messages from friends"],
        correct: 1
      }
    ];

    let currentQuestion = 0;
    let correctAnswers = 0;
    let score = 0;
    let quizActive = false;
    let answered = false;

    function startQuiz() {
      currentQuestion = 0;
      correctAnswers = 0;
      score = 0;
      quizActive = true;
      answered = false;

      document.getElementById('victoryBox').classList.remove('show');
      document.getElementById('failureBox').classList.remove('show');
      document.getElementById('startBtn').style.display = 'none';
      document.getElementById('nextBtn').style.display = 'none';

      updateStats();
      showQuestion();
    }

    function showQuestion() {
      if (currentQuestion >= questions.length) {
        endQuiz();
        return;
      }

      answered = false;
      const q = questions[currentQuestion];

      document.getElementById('questionText').textContent = `Q${currentQuestion + 1}: ${q.question}`;

      const optionsContainer = document.getElementById('optionsContainer');
      optionsContainer.innerHTML = '';

      q.options.forEach((option, index) => {
        const optionDiv = document.createElement('div');
        optionDiv.className = 'option';
        optionDiv.textContent = option;
        optionDiv.onclick = () => selectAnswer(index);
        optionsContainer.appendChild(optionDiv);
      });

      updateStats();
    }

    function selectAnswer(selectedIndex) {
      if (answered || !quizActive) return;

      answered = true;
      const q = questions[currentQuestion];
      const options = document.querySelectorAll('.option');

      options.forEach((option, index) => {
        option.classList.add('disabled');
        if (index === q.correct) {
          option.classList.add('correct');
        }
        if (index === selectedIndex && index !== q.correct) {
          option.classList.add('incorrect');
        }
      });

      if (selectedIndex === q.correct) {
        correctAnswers++;
        score += 100;
        updateStats();
      }

      setTimeout(() => {
        if (currentQuestion < questions.length - 1) {
          document.getElementById('nextBtn').style.display = 'inline-block';
        } else {
          setTimeout(endQuiz, 1500);
        }
      }, 1000);
    }

    function nextQuestion() {
      currentQuestion++;
      document.getElementById('nextBtn').style.display = 'none';
      showQuestion();
    }

    function updateStats() {
      document.getElementById('correctCount').textContent = correctAnswers;
      document.getElementById('remainingCount').textContent = questions.length - currentQuestion;
      document.getElementById('score').textContent = score;

      const progress = ((currentQuestion) / questions.length) * 100;
      const progressFill = document.getElementById('progressFill');
      progressFill.style.width = progress + '%';
      progressFill.textContent = `${currentQuestion}/${questions.length}`;
    }

    function endQuiz() {
      quizActive = false;

      const progressFill = document.getElementById('progressFill');
      progressFill.style.width = '100%';
      progressFill.textContent = '10/10';

      if (correctAnswers >= 7) {
        showVictory();
      } else {
        showFailure();
      }

      document.getElementById('startBtn').style.display = 'inline-block';
      document.getElementById('startBtn').textContent = 'TRY AGAIN';
      document.getElementById('nextBtn').style.display = 'none';
    }

    function showVictory() {
      document.getElementById('finalScore').textContent = correctAnswers;
      document.getElementById('victoryBox').classList.add('show');
      createConfetti();
    }

    function showFailure() {
      document.getElementById('failScore').textContent = correctAnswers;
      document.getElementById('failureBox').classList.add('show');
    }

    function createConfetti() {
      const colors = ['#ff00ff', '#00ffff', '#00ff88', '#ffd700', '#ff69b4'];
      for (let i = 0; i < 150; i++) {
        setTimeout(() => {
          const confetti = document.createElement('div');
          confetti.className = 'confetti';
          confetti.style.left = Math.random() * 100 + 'vw';
          confetti.style.background = colors[Math.floor(Math.random() * colors.length)];
          confetti.style.animationDelay = Math.random() * 2 + 's';
          document.body.appendChild(confetti);

          setTimeout(() => confetti.remove(), 3000);
        }, i * 20);
      }
    }

    function toggleHint() {
      const hintBox = document.getElementById('hintBox');
      hintBox.classList.toggle('show');
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
  for (int i = 0; i < sessionCount; i++) {
    if (authenticatedIPs[i] == ip) {
      teamIndexForIP[i] = teamIdx;
      return;
    }
  }
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
      for (int j = i; j < sessionCount - 1; j++) {
        authenticatedIPs[j] = authenticatedIPs[j + 1];
        teamIndexForIP[j] = teamIndexForIP[j + 1];
      }
      sessionCount--;
      return;
    }
  }
}

void handleRoot() { server.send_P(200, "text/html", login_html); }

void handleLogin() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "Method Not Allowed"); return; }
  String teamId = server.arg("teamId");
  String pass = server.arg("password");
  int teamIdx = authenticateTeam(teamId, pass);
  if (teamIdx >= 0) { String clientIP = server.client().remoteIP().toString(); addSession(clientIP, teamIdx); server.send(200, "text/plain", "OK"); }
  else { server.send(200, "text/plain", "FAIL"); }
}

void handleGame() {
  String clientIP = server.client().remoteIP().toString();
  int teamIdx = getTeamIndex(clientIP);
  if (teamIdx < 0) { server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); return; }
  const TeamData& team = teams[teamIdx];
  String html = String(game_html);
  html.replace("{{FLAG}}", team.flag);
  html.replace("{{NEXT_WIFI}}", team.nextWifi);
  html.replace("{{WIFI_PASS}}", team.nextWifiPassword);
  html.replace("{{CLUE}}", team.clue);
  server.send(200, "text/html", html);
}

void handleLogout() { String clientIP = server.client().remoteIP().toString(); removeSession(clientIP); server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); }

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

void loop() { server.handleClient(); }
