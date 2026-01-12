![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Frontend](https://img.shields.io/badge/Frontend-HTML5_%2F_JS-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![Architecture](https://img.shields.io/badge/Architecture-REST_API-success?style=for-the-badge&logo=serverless&logoColor=white)
![Build](https://img.shields.io/badge/Build-Make-orange?style=for-the-badge)

# LinkedVault

LinkedVault is a robust and fast Employee Management System built with a custom C backend and a modern, responsive HTML/JS frontend. It acts as a practical demonstration of using a **Singly Linked List** in a real-world server application to manage employee records efficiently in memory.

---

## 🚀 Key Features

### 🛠 Core Engineering

- **Custom Data Structures:** Implements a dynamic Singly Linked List (`struct employee *next`) for $O(1)$ insertions and flexible memory usage.
- **Thread Safety:** Utilizes **POSIX Mutex Locks** (`pthread_mutex_t`) to ensure data integrity during concurrent read/write operations, effectively preventing Race Conditions.
- **Memory Management:** Custom logic to `malloc` and `free` nodes dynamically, ensuring zero memory leaks during lifecycle operations.
- **Persistence:** Saves and retrieves the in-memory linked list to/from binary files on disk.

### ⚡ Functionality

- **CRUD Operations:** Create, Read, Update, and Delete employee records via a RESTful API.
- **Advanced Algorithms:**
  - **Linked Reverse:** Physically reverses the pointers of the linked list in memory.
  - **Recursive View:** Uses recursion to generate a reversed JSON view without modifying the actual heap memory.
- **Data Persistence:**
  - **Bulk Import:** High-speed parsing of CSV files to populate the list.
  - **Export:** Streams current memory state to a downloadable `.csv` file.

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

- **C Compiler (GCC):**
  - **Linux/macOS:** Standard `gcc` installation.
  - **Windows:** [MinGW-w64](https://www.mingw-w64.org/).
    > ⚠️ **Critical for Windows:** You **must** ensure your compiler supports POSIX threads. If installing MinGW, select **`posix`** for the "Threads" setting (do not use `win32`). If you encounter errors like `undefined reference to pthread_create`, you have the wrong MinGW version.
- **Build Automation:**
  - **Windows:** `mingw32-make` (included with MinGW).
  - **Linux/macOS:** GNU `make`.
- **Web Browser:** Modern standards-compliant browser (Chrome, Firefox, Edge, Safari).

---

## ⚡ Quick Start Guide

### 1. Build and Run the Backend

The backend is a standalone C application that acts as the API server.

1.  Open your terminal or command prompt.
2.  Navigate to the backend directory:
    ```bash
    cd backend
    ```
3.  Compile the project:

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

You can run the user interface effortlessly:

1.  Navigate to the `frontend/` folder.
2.  Simply double-click `index.html` to open it in your browser.
3.  Ensure the backend is running; the dashboard will connect to `http://localhost:8000` automatically.

---

## 🔌 API Endpoints

| Method | Endpoint            | Description                                     |
| :----- | :------------------ | :---------------------------------------------- |
| POST   | `/insert`           | Add a new employee                              |
| GET    | `/show`             | Get all employees (JSON)                        |
| GET    | `/search?id=X`      | Find employee by ID                             |
| DELETE | `/delete?id=X`      | Remove an employee                              |
| PUT    | `/linkedreverse`    | Reverse the backend linked list                 |
| GET    | `/recursivereverse` | Get reversed view (JSON) **(See Known Issues)** |
| POST   | `/upload_csv`       | Import data from CSV                            |
| GET    | `/download_table`   | Download data as CSV                            |
| DELETE | `/clear_table`      | Delete all records                              |

---

## 👥 Contributors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/Ardinox">
        <img src="https://github.com/Ardinox.png" width="100px;" alt=""/><br />
        <sub><b>Ajoy</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/ranadip9339">
        <img src="https://github.com/ranadip9339.png" width="100px;" alt=""/><br />
        <sub><b>Ronadeep</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Anuska1211">
        <img src="https://github.com/Anuska1211.png" width="100px;" alt=""/><br />
        <sub><b>Anuska</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/arkak4204-coder">
        <img src="https://github.com/arkak4204-coder.png" width="100px;" alt=""/><br />
        <sub><b>Arka</b></sub>
      </a>
    </td>
    
  </tr>
</table>
