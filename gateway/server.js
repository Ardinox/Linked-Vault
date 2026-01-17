require('dotenv').config();

const express = require("express");
const fs = require("fs");
const cors = require("cors");
const path = require("path");
const jwt = require("jsonwebtoken");
const { createProxyMiddleware } = require("http-proxy-middleware");

const app = express();
const port = process.env.PORT || 4000;
const SECRET_KEY = process.env.SECRET_KEY || "fallback_secret_key_for_dev";
const backendUrl = process.env.BACKEND_URL || "http://localhost:8000";

// --- MIDDLEWARE CONFIGURATION ---
app.use(cors());

// 1. Create a Specific JSON Parser Middleware
// We will apply this ONLY to the Node.js routes that need it.
// This prevents the proxy stream from being consumed for C Backend requests.
const jsonParser = express.json();

// --- 2. SPY LOGGER MIDDLEWARE ---
app.use((req, res, next) => {
  const monitoredMethods = ["POST", "PUT", "DELETE"];

  if (monitoredMethods.includes(req.method)) {
    const logDir = path.join(__dirname, "logs");
    const logFile = path.join(logDir, "admin_history.json");

    if (!fs.existsSync(logDir)) {
      fs.mkdirSync(logDir);
    }

    const logEntry = {
      timestamp: new Date().toISOString(),
      method: req.method,
      endpoint: req.originalUrl,
    };

    let history = [];
    try {
      if (fs.existsSync(logFile)) {
        const data = fs.readFileSync(logFile);
        history = JSON.parse(data);
      }
    } catch (err) {
      console.error("Error reading log file:", err);
    }

    history.push(logEntry);
    fs.writeFileSync(logFile, JSON.stringify(history, null, 2));

    console.log(`[LOGGED] ${req.method} request to ${req.originalUrl}`);
  }
  next();
});

// --- Auth Route (Uses JSON Parser) ---
app.post("/login", jsonParser, (req, res) => {
  const { username, password } = req.body;
  if (username === "admin" && password === "password") {
    const token = jwt.sign({ username: username, role: "admin" }, SECRET_KEY, {
      expiresIn: "1h",
    });
    return res.json({ status: "success", token: token });
  }
  return res
    .status(401)
    .json({ status: "error", message: "Invalid credentials" });
});

// --- AUTH MIDDLEWARE ---
const authenticateToken = (req, res, next) => {
  if (req.path === "/login") return next();

  const authHeader = req.headers["authorization"];
  const token = authHeader && authHeader.split(" ")[1];

  if (!token) {
    return res
      .status(401)
      .json({ message: "Access Denied: No Token Provided" });
  }

  jwt.verify(token, SECRET_KEY, (err, user) => {
    if (err) {
      return res.status(403).json({ message: "Access Denied: Invalid Token" });
    }
    req.user = user;
    next();
  });
};

app.use(authenticateToken);

// --- ADMIN API ENDPOINTS ---
app.get("/api/history", (req, res) => {
  const logFile = path.join(__dirname, "logs", "admin_history.json");

  if (!fs.existsSync(logFile)) {
    return res.json([]);
  }
  try {
    const data = fs.readFileSync(logFile);
    const history = JSON.parse(data);
    res.json(history.reverse());
  } catch (err) {
    console.error("Error reading logs:", err);
    res.status(500).json({ error: "Could not read history logs" });
  }
});

// --- USER TABLE MANAGEMENT (Uses JSON Parser) ---

app.get('/my_tables', (req, res) => {
    const username = req.user.username; 
    const tableFile = path.join(__dirname, 'logs', 'user_tables.json');

    if (!fs.existsSync(tableFile)) return res.json([]);

    try {
        const data = JSON.parse(fs.readFileSync(tableFile));
        const userTables = data[username] || [];
        res.json(userTables);
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to fetch tables" });
    }
});

// Apply jsonParser here because we need to read req.body.tableName
app.post('/my_tables', jsonParser, (req, res) => {
    const username = req.user.username;
    const { tableName } = req.body;

    if (!tableName) return res.status(400).json({ message: "Table Name required" });

    const logDir = path.join(__dirname, 'logs');
    const tableFile = path.join(logDir, 'user_tables.json');

    if (!fs.existsSync(logDir)) fs.mkdirSync(logDir);

    let db = {};
    if (fs.existsSync(tableFile)) {
        try {
            db = JSON.parse(fs.readFileSync(tableFile));
        } catch(e) {}
    }

    if (!db[username]) db[username] = [];

    if (db[username].includes(tableName)) {
        return res.status(409).json({ message: "Table already exists" });
    }

    db[username].push(tableName);
    fs.writeFileSync(tableFile, JSON.stringify(db, null, 2));

    res.json({ message: "Table Created", tableName });
});

// --- PROXY MIDDLEWARE ---
// No JSON Parser here! The raw stream flows directly to C.
app.use(
  "/",
  createProxyMiddleware({
    target: backendUrl,
    changeOrigin: true,
    onError: (err, req, res) => {
      console.error("Proxy Error:", err);
      res.status(500).send("Proxy Error: Could not reach C Backend.");
    },
  })
);

app.listen(port, () => {
  console.log(`Gatekeeper running on port ${port}`);
});