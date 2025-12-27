![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Frontend](https://img.shields.io/badge/Frontend-HTML5_%2F_JS-E34F26?style=for-the-badge&logo=html5&logoColor=white)

![Bootstrap](https://img.shields.io/badge/Style-Bootstrap_5-7952B3?style=for-the-badge&logo=bootstrap&logoColor=white)
![Library](https://img.shields.io/badge/Library-Mongoose_Web_Server-green?style=for-the-badge)
![JSON](https://img.shields.io/badge/Data-cJSON-orange?style=for-the-badge)

![Build](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Under_Development-red?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)

# LinkedVault

LinkedVault is a robust and fast Employee Management System built with a custom C backend and a modern, responsive HTML/JS frontend. It utilizes a Linked List data structure in memory to manage employee records efficiently

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

## ⚡ How to Run

### Start the Backend

1. Open your terminal in `backend` folder.

2. Compile the project:

   - **Windows:**
     ```bash
     mingw32-make
     ```
   - **Linux / Mac:**
     ```bash
     make
     ```

3. Start the server:
   - **Windows:**
     ```bash
     .\LinkedVault.exe
     ```
   - **Linux / Mac:**
     ```bash
     ./LinkedVault.exe
     ```


### Launch the Frontend

1. Open `frontend/index.html` (The entry point).

2. start live server.

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


