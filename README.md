![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Frontend](https://img.shields.io/badge/Frontend-HTML5_%2F_JS-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![Architecture](https://img.shields.io/badge/Architecture-REST_API-success?style=for-the-badge&logo=serverless&logoColor=white)
![Build](https://img.shields.io/badge/Build-Make-orange?style=for-the-badge)

# LinkedVault

LinkedVault is a robust and fast Employee Management System built with a custom C backend and a modern, responsive HTML/JS frontend. It utilizes a Linked List data structure in memory to manage employee records efficiently.

---

## 🚀 Key Features

### 🛠 Core Engineering
* **Custom Data Structures:** Implements a dynamic Singly Linked List (`struct employee *next`) for $O(1)$ insertions and flexible memory usage.
* **Thread Safety:** Utilizes **POSIX Mutex Locks** (`pthread_mutex_t`) to ensure data integrity during concurrent read/write operations (e.g., prevents Race Conditions when multiple users insert/delete simultaneously).
* **Memory Management:** Custom logic to `malloc` and `free` nodes dynamically, ensuring zero memory leaks during lifecycle operations.

### ⚡ Functionality
* **CRUD Operations:** Create, Read, Update, and Delete employee records via REST API.
* **Advanced Algorithms:**
    * **Linked Reverse:** Physically reverses the pointers of the linked list in memory.
    * **Recursive View:** Uses recursion to generate a reversed JSON view without modifying the actual heap memory.
* **Data Persistence:**
    * **Bulk Import:** High-speed parsing of CSV files to populate the list.
    * **Export:** Streams current memory state to a downloadable `.csv` file.

---

## 📂 Project Structure

```text
LinkedVault/
├── backend/                # The C Server Logic
│   ├── include/            # Header files (.h)
│   ├── src/                # Source code (.c)
│   ├── vendor/             # Dependencies (Mongoose, cJSON)
│   ├── Makefile            # Build configuration
│   └── LinkedVault.exe     # Compiled Executable (after build)
│
├── frontend/               # The Web Interface
│   ├── Assets/             # Images and logos
│   ├── Dashboard/          # App Logic (add, update, table view)
│   │   ├── dashboard.js    # Main Frontend Controller
│   │   ├── styles.css      # Dashboard specific styles
│   │   └── *.html          # Dashboard pages
│   ├── global.css          # Landing page styles
│   └── index.html          # Landing Page
│
└── README.md               # Project Documentation
```

## 🛠️ Prerequisites

Before setting up the project, ensure your environment meets the following requirements:

### System & Tools
* **C Compiler (GCC):**
    * **Linux/macOS:** Standard `gcc` installation.
    * **Windows:** [MinGW-w64](https://www.mingw-w64.org/).
        > ⚠️ **Critical for Windows:** You **must** ensure your compiler supports POSIX threads. If installing MinGW, select **`posix`** for the "Threads" setting (do not use `win32`).
* **Build Automation:**
    * **Windows:** `mingw32-make` (included with MinGW).
    * **Linux/macOS:** GNU `make`.
* **Web Browser:** Modern standards-compliant browser (Chrome, Firefox, Edge, Safari).
* **(Optional) Python 3.x:** Required only if you intend to host the frontend for local network (LAN) access.

---

## ⚡ Quick Start Guide

### 1. Build and Run the Backend
The backend is a standalone C application that acts as the API server.

1.  Open your terminal or command prompt.
2.  Navigate to the backend source directory:
    ```bash
    cd backend
    ```
3.  Compile the project using the makefile:
    ```bash
    # For Windows
    mingw32-make clean
    mingw32-make

    # For Linux/Mac
    make clean
    make
    ```
4.  Start the server:
    ```bash
    .\LinkedVault.exe
    ```
    > **Success:** You should see the message: `Server running on port 8000...`

### 2. Launch the Dashboard
You can run the user interface in two modes:

#### Option A: Local Mode (Standalone)
* Navigate to the `frontend/` folder in your file explorer.
* Double-click `index.html` to open it in your default browser.

#### Option B: Network Mode (LAN Access)
To access the dashboard from mobile devices or other computers on your Wi-Fi:

1.  Find your computer's local IP address (Run `ipconfig` on Windows or `ifconfig` on Linux).
2.  Edit `frontend/Dashboard/dashboard.js` and update the API configuration:
    ```javascript
    const API_URL = "http://YOUR_LOCAL_IP:8000";
    ```
3.  Serve the frontend using Python:
    ```bash
    cd frontend
    python -m http.server 5500
    ```
4.  On your mobile device, browse to: `http://YOUR_LOCAL_IP:5500`

---

## 🔌 API Endpoints

| Method | Endpoint            | Description                      |
| :----- | :------------------ | :------------------------------- |
| POST   | `/insert`           | Add a new employee               |
| GET    | `/show`             | Get all employees (JSON)         |
| GET    | `/search?id=X`      | Find employee by ID              |
| DELETE | `/delete?id=X`      | Remove an employee               |
| PUT    | `/linkedreverse`    | Reverse the backend linked list  |
| GET    | `/recursivereverse` | Get reversed view (JSON)         |
| POST   | `/upload_csv`       | Import data from CSV             |
| GET    | `/download_table`   | Download data as CSV             |
| DELETE | `/clear_table`      | Delete all records               |


## 📊 Performance Benchmarks

The system was stress-tested using **Postman Performance Runner** to evaluate stability, concurrency handling, and memory efficiency.

**Test Environment:**
* **Load Profile:** Fixed load with concurrent Virtual Users (VUs).
* **Duration:** Continuous execution of mixed CRUD operations (Insert, Read, Delete, Export).
* **Hardware:** Local execution on standard x64 architecture.

**Key Metrics:**

| Metric | Result | Analysis |
| :--- | :--- | :--- |
| **Average Latency** | **9 ms** | The C backend and in-memory Linked List provide near-instantaneous response times, significantly outperforming typical interpreted language backends. |
| **Max Response Time** | **88 ms** | Worst-case scenario remains under 100ms, ensuring a fluid user experience even during complex operations like CSV parsing. |
| **Throughput** | **~10 req/s** | Consistent throughput observed with no degradation over time, indicating successful memory management. |
| **Server Stability** | **100%** | **Zero 500 Internal Server Errors** or crashes observed during concurrent load. |

> **Note:** The server correctly handles race conditions. For example, if User A deletes a node while User B tries to update it, the server utilizes **Mutex locks** to process requests sequentially, returning a valid **404 Not Found** rather than crashing..

---

## ❓ Troubleshooting

Common issues and solutions when building or running the project.

### Compiler & Build Issues

**1. Error:** `undefined reference to 'pthread_create'` or `cannot find -lpthread`
* **Context:** This occurs on Windows when using a MinGW compiler configured with the `win32` threading model instead of `posix`.
* **Solution:**
    * Reinstall MinGW-w64.
    * During installation, ensure you select **`posix`** in the **Threads** dropdown menu (default is often `win32`).
    * Alternatively, use **MSYS2** to install the correct toolchain:
      ```bash
      pacman -S mingw-w64-x86_64-gcc
      ```

**2. Error:** `'make' is not recognized as an internal or external command`
* **Context:** Windows does not have a native `make` tool.
* **Solution:**
    * Use `mingw32-make` instead (comes with MinGW).
    * Add your MinGW `bin` folder (e.g., `C:\MinGW\bin`) to your System **PATH** environment variable.

### Runtime Issues

**3. "Port 8000 is already in use"**
* **Context:** The server was not closed properly or another application is using the port.
* **Solution:**
    * **Windows:** Open Task Manager and kill `LinkedVault.exe`.
    * **Linux/Mac:** Run `lsof -i :8000` to find the PID, then `kill -9 [PID]`.