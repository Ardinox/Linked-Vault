require('dotenv').config();

const express = require("express");
const cors = require("cors");
const jwt = require("jsonwebtoken");
const axios = require("axios"); // used to talk to C
const bcrypt = require("bcryptjs"); // used to hash passwords

const app = express();
const port = process.env.PORT || 4000;
const SECRET_KEY = process.env.SECRET_KEY || "super_secret_key_change_me";
const C_BACKEND = process.env.BACKEND_URL || "http://localhost:8000";

// --- MIDDLEWARE ---
app.use(cors());
app.use(express.json()); // PARSE EVERYTHING (We are no longer streaming blindly)

// --- HELPER: Centralized C-Backend Caller ---
// This function adds the 'owner_id' automatically to every request
async function callC(method, endpoint, payload = {}, params = {}, tokenData = null) {
    try {
        // 1. Inject owner_id if user is logged in
        if (tokenData) {
            payload.owner_id = tokenData.id; // For POST/PUT bodies
            params.owner_id = tokenData.id;  // For GET/DELETE query strings
        }

        // 2. Make the Request
        const response = await axios({
            method: method,
            url: `${C_BACKEND}${endpoint}`,
            data: payload,
            params: params
        });

        return { status: response.status, data: response.data };
    } catch (error) {
        // Handle C Server Errors
        if (error.response) {
            return { status: error.response.status, data: error.response.data };
        }
        console.error("C Backend Error:", error.message);
        return { status: 500, data: { error: "C Backend Unreachable" } };
    }
}

// ==========================
// 1. AUTHENTICATION ROUTES
// ==========================

// REGISTER
app.post("/register", async (req, res) => {
    const { username, password } = req.body;
    if (!username || !password) return res.status(400).json({ error: "Missing fields" });

    // 1. Hash the password before sending to C (Security Best Practice)
    const hash = await bcrypt.hash(password, 10);

    // 2. Call C: /auth/register
    const result = await callC("POST", "/auth/register", { username, hash });
    res.status(result.status).json(result.data);
});

// LOGIN
app.post("/login", async (req, res) => {
    const { username, password } = req.body;
    if (!username || !password) return res.status(400).json({ error: "Missing fields" });

    // 1. Call C: /auth/get_user (POST)
    // C returns: { "id": 1, "hash": "$2a$10$..." }
    const result = await callC("POST", "/auth/get_user", { username });

    if (result.status !== 200) {
        return res.status(401).json({ error: "User not found" });
    }

    // 2. Verify Password using Node.js (C just stores the string)
    const validPass = await bcrypt.compare(password, result.data.hash);
    if (!validPass) {
        return res.status(401).json({ error: "Invalid Credentials" });
    }

    // 3. Generate Token (Embed the C-Backend ID!)
    const token = jwt.sign(
        { id: result.data.id, username: username }, 
        SECRET_KEY, 
        { expiresIn: "2h" }
    );

    res.json({ status: "success", token, userId: result.data.id });
});

// ==========================
// 2. AUTH MIDDLEWARE (Gatekeeper)
// ==========================
const authenticateToken = (req, res, next) => {
    const authHeader = req.headers["authorization"];
    const token = authHeader && authHeader.split(" ")[1];

    if (!token) return res.status(401).json({ error: "No Token" });

    jwt.verify(token, SECRET_KEY, (err, user) => {
        if (err) return res.status(403).json({ error: "Invalid Token" });
        req.user = user; // Contains { id, username }
        next();
    });
};

app.use(authenticateToken); // Protect all routes below this line

// ==========================
// 3. TABLE MANAGEMENT
// ==========================

// CREATE TABLE
app.post("/my_tables", async (req, res) => {
    // req.body has { name: "My Project" }
    // We inject { owner_id: req.user.id }
    const result = await callC("POST", "/meta/create_table", req.body, {}, req.user);
    res.status(result.status).json(result.data);
});

// LIST TABLES
app.post("/list_tables", async (req, res) => {
    // C expects POST for listing now (from your C code)
    const result = await callC("POST", "/meta/list_tables", {}, {}, req.user);
    res.status(result.status).json(result.data);
});

// DROP TABLE (Soft Delete)
app.delete("/delete_table", async (req, res) => {
    // Expects ?table_id=1001
    const result = await callC("DELETE", "/delete_table", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

// ==========================
// 4. DATA OPERATIONS (CRUD)
// ==========================

// INSERT
app.post("/insert", async (req, res) => {
    // Body: { table_id: 1001, data: {...} }
    // Node injects owner_id automatically
    const result = await callC("POST", "/insert", req.body, {}, req.user);
    res.status(result.status).json(result.data);
});

// SHOW ALL (GET)
app.get("/show", async (req, res) => {
    // Query: ?table_id=1001
    const result = await callC("GET", "/show", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

// SEARCH (GET)
app.get("/search", async (req, res) => {
    // Query: ?table_id=1001&id=5
    const result = await callC("GET", "/search", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

// DELETE ROW (DELETE)
app.delete("/delete", async (req, res) => {
    // Query: ?table_id=1001&id=5
    const result = await callC("DELETE", "/delete", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

// UPDATE (PUT)
app.put("/update", async (req, res) => {
    // Body: { table_id, original_id, data }
    const result = await callC("PUT", "/update", req.body, {}, req.user);
    res.status(result.status).json(result.data);
});

// ==========================
// 5. UTILITIES (Reverse, Export)
// ==========================

app.put("/linkedreverse", async (req, res) => {
    const result = await callC("PUT", "/linkedreverse", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

app.get("/recursivereverse", async (req, res) => {
    const result = await callC("GET", "/recursivereverse", {}, req.query, req.user);
    res.status(result.status).json(result.data);
});

// DOWNLOAD CSV (Special Case: Stream handling)
app.get("/download_table", async (req, res) => {
    try {
        const response = await axios({
            method: "GET",
            url: `${C_BACKEND}/download_table`,
            params: { ...req.query, owner_id: req.user.id },
            responseType: 'stream' // Important for file downloads
        });
        
        // Pipe the file stream directly to the client
        res.setHeader('Content-Type', 'text/csv');
        res.setHeader('Content-Disposition', response.headers['content-disposition']);
        response.data.pipe(res);
    } catch (error) {
        res.status(500).json({ error: "Download failed" });
    }
});

app.listen(port, () => {
    console.log(`Gatekeeper running on port ${port}`);
});