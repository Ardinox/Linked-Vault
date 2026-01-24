![Backend](https://img.shields.io/badge/Backend-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Middleware](https://img.shields.io/badge/Middleware-Node.js-339933?style=for-the-badge&logo=node.js&logoColor=white)
![Frontend](https://img.shields.io/badge/Frontend-HTML5_%2F_JS-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![Architecture](https://img.shields.io/badge/Architecture-3--Tier-success?style=for-the-badge&logo=serverless&logoColor=white)

# LinkedVault

**LinkedVault** is a secure, high-performance Employee Management System that bridges low-level C systems programming with modern web architecture. It features a custom **Linked List-based in-memory database**, binary file persistence, and a secure Node.js API gateway, all fully containerized with Docker.

## 🏗 Architecture

The system follows a strictly layered **Client-Gateway-Server** pattern designed for security and performance:

- **Frontend (Presentation Layer):**
  - Built with **Vanilla JS (ES6 Modules)** for a lightweight, dependency-free experience.
  - Handles client-side state and renders dynamic tables from JSON data.
  - Communicates exclusively with the Gateway, never directly with the C backend.

- **Gateway (Logic & Security Layer):**
  - A **Node.js/Express** middleware that acts as the single entry point.
  - **JWT Guard:** Decodes tokens and validates user sessions before processing requests.
  - **Context Injection:** Automatically injects the user's `owner_id` into payloads, ensuring strict data isolation at the protocol level.

- **Backend (High-Performance Engine):**
  - Written in **C11**, optimizing for manual memory management and raw speed.
  - **In-Memory Database:** Loads data into a **Singly Linked List** heap structure for $O(1)$ access times.
  - **Binary Storage:** Serializes memory structures directly to disk (`.bin`), avoiding the overhead of SQL parsing or JSON conversion during I/O.

## 🚀 Key Features

### 🛠 Core Engineering (C Backend)

- **Custom Data Structures:** Implements dynamic Singly Linked Lists (`struct employee *next`) for $O(1)$ insertions.
- **Thread Safety:** Uses **POSIX Mutex Locks** to prevent Race Conditions during concurrent read/write.
- **Binary Persistence:** Saves/Loads data directly to/from binary files (`.bin`), which is significantly faster than text-based formats.
- **Audit Logging:** Automatically tracks every `CREATE`, `UPDATE`, and `DELETE` action with timestamps.

### 🛡️ Security & Web (Node Gateway)

- **JWT Authentication:** Secure session management; users must log in to access data.
- **Role-Based Access (RBAC):** Users can only access tables they explicitly own (Strict 403 enforcement).
- **Dockerized:** Fully isolated environments ensuring the app runs exactly the same on any machine.

## 📂 Project Structure

```text
LinkedVault/
├── backend/            # The C Engine
│   ├── bin/            # Database storage (mapped to host)
│   ├── include/        # Header files (.h)
│   ├── src/            # Source code (.c)
│   ├── vendor/         # External libs (mongoose, cJSON)
│   └── Dockerfile      # Multi-stage build (GCC -> Debian Slim)
│
├── gateway/            # The Node.js Security Layer
│   ├── server.js       # Auth logic & Proxy
│   ├── package.json    # Project dependencies
│   ├── .env            # Environment variables
│   └── Dockerfile      # Node Alpine image
│
├── frontend/           # The User Interface
│   ├── Assets/         # Static images and icons
│   ├── views/          # HTML partials and views
│   ├── js/             # Main JavaScript logic
│   │   ├── modules/    # Reusable API & Auth scripts
│   │   └── pages/      # Dashboard & Page controllers
│   ├── index.html      # Main entry point
│   └── Dockerfile      # Nginx image
│
└── docker-compose.yml  # Container Orchestration
```

## 🛠️ Prerequisites

Docker Desktop (or Docker Engine + Compose)

## ⚡ Quick Start Guide

### 1. Build and Run

Open your terminal in the project root and run:

   ```bash
   docker-compose up --build
   ```

### 2. Access the App

Once the logs say "Server running...", open your browser:

👉 `http://localhost:3000`

### 3. Usage Flow

**Register:** Create a new account on the login screen.

**Dashboard:** Click "Create Table" to start a new Employee Database.

**Manage Data:** Click "View" on your table to Add, Delete, or Search for employees.

**Audit Logs:** Check the "History" tab to see your actions recorded in real-time.

---

## 🔌 API Endpoints

| Method | Endpoint             | Description                                     |
| :----- | :------------------- | :---------------------------------------------- |
| POST   | `/auth/register`     | Create a new user account                       |
| POST   | `/auth/login`        | Login and receive JWT token                     |
| POST   | `/my_tables`         | Create a new table                              |
| POST   | `/list_tables`       | Get list of tables owned by user                |
| DELETE | `/delete_table`      | Permanently delete a table                      |
| POST   | `/insert`            | Add a new employee record                       |
| GET    | `/show`              | Get all employees (JSON)                        |
| GET    | `/search`            | Find employee by ID                             |
| PUT    | `/update`            | Update an existing employee                     |
| DELETE | `/delete`            | Remove an employee                              |
| PUT    | `/linkedreverse`     | Reverse the backend linked list                 |
| GET    | `/recursivereverse`  | Get reversed view (JSON) **(Recursive)**        |
| POST   | `/upload_csv`        | Import data from CSV string                     |
| GET    | `/download_table`    | Download data as CSV file                       |
| GET    | `/api/history`       | View audit logs of user actions                 |

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
