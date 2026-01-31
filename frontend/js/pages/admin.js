import { Auth } from "../modules/auth.js";
import { Api } from "../modules/api.js";
import { CONFIG } from "../modules/config.js";

// --- Helper ---
const ACTION_LABELS = {
  "LOGIN": "Logged In",
  "CREATE": "Created Table", 
  "IMPORT": "Imported Data", 
  "DELETE-TABLE": "Deleted Table", 
  "INSERT": "Inserted Record",
  "UPDATE": "Updated Record", 
  "DELETE": "Deleted Record", 
  "CLEAR": "Cleared Table",    
  "REVERSE": "Reversed List"  
};
const ACTION_COLORS = {
  "LOGIN": "text-info",
  "CREATE": "text-success",
  "IMPORT": "text-primary",
  "DELETE-TABLE": "text-danger",
  "INSERT": "text-success",
  "UPDATE": "text-warning",
  "DELETE": "text-danger",
  "CLEAR": "text-danger",
  "REVERSE": "text-secondary"
};

// 1. Session Check
Auth.checkSession();

// 2. Event Listeners
document.addEventListener("DOMContentLoaded", () => {
  loadTableList();

  // Attach Logout
  const logoutBtn = document.getElementById("logoutBtn");
  if (logoutBtn) logoutBtn.addEventListener("click", Auth.logout);

  // Attach Refresh Logs
  const refreshBtn = document.getElementById("refreshLogsBtn");
  if (refreshBtn) refreshBtn.addEventListener("click", loadAuditLogs);
});

// --- VIEW SWITCHER ---
window.switchView = function (viewName, e) {
  // 1. Update Buttons
  document
    .querySelectorAll(".nav-link")
    .forEach((btn) => btn.classList.remove("active"));

  // 2. Add active class to the clicked button using 'e.currentTarget'
  if (e) {
    e.currentTarget.classList.add("active");
  }

  // 3. Show Section
  document
    .querySelectorAll(".view-section")
    .forEach((el) => el.classList.remove("active"));
  document.getElementById(`view-${viewName}`).classList.add("active");

  // 4. Load Data if needed
  if (viewName === "logs") loadAuditLogs();
  if (viewName === "tables") loadTableList();
};

// --- LOGIC: TABLES ---
async function loadTableList() {
  const grid = document.getElementById("tableGrid");
  if (!grid) return;
  grid.innerHTML = '<p class="text-white">Loading...</p>';

  try {
    const tables = await Api.getTables();

    grid.innerHTML = ""; // Clear loader

    // 1. Add "Create New" Card
    const addCard = `
            <div class="table-card add-card" data-bs-toggle="modal" data-bs-target="#createTableModal">
                <div class="plus-icon">+</div>
                <h3>Create Table</h3>
            </div>
        `;
    grid.innerHTML += addCard;

    // 2. Add Existing Tables
    if (tables && tables.length > 0) {
      tables.forEach((table) => {
        const card = `
                <div class="table-card" onclick="window.location.href='table_view.html?table_id=${table.internal_id}&name=${encodeURIComponent(table.name)}'">
                    <i class="fa-solid fa-database fa-2x mb-2 text-secondary"></i>
                    <h3>${table.name}</h3>
                </div>`;
        grid.innerHTML += card;
      });
    }
  } catch (error) {
    console.error(error);
    grid.innerHTML = '<p class="text-danger">Failed to load tables.</p>';
  }
}

window.createNewTable = async function () {
  const nameInput = document.getElementById("newTableName");
  const name = nameInput.value.trim();

  if (!name) return alert("Enter a name");

  try {
    // 1. Register table in Node
    const response = await Api.createTable(name);

    if (response.ok) {
      const data = await response.json();
      // 2. If success, go to the table view
      window.location.href = `table_view.html?table_id=${data.internal_id}&name=${encodeURIComponent(name)}`;
    } else {
      const err = await response.json();
      alert("Error: " + (err.message || "Could not create table"));
    }
  } catch (e) {
    console.error(e);
    alert("Network Error");
  }
};

// --- LOGIC: LOGS ---
async function loadAuditLogs() {
  const tbody = document.getElementById("logBody");
  tbody.innerHTML =
    '<tr><td colspan="3" class="text-center">Loading...</td></tr>';

  try {
    const logs = await Api.getHistory();

    logs.reverse();

    tbody.innerHTML = "";

    if (!logs || logs.length === 0) {
      tbody.innerHTML =
        '<tr><td colspan="3" class="text-center">No activity recorded.</td></tr>';
      return;
    }

    let rows = "";

    logs.forEach((log) => {
      const label = ACTION_LABELS[log.action] || log.action;
      let colorClass = ACTION_COLORS[log.action] || "text-secondary";

      rows += `
                <tr>
                    <td class="text-nowrap">${log.timestamp}</td>
                    <td class="fw-bold ${colorClass}">
                        ${label}
                    </td>
                    <td>
                        ${log.details || "-"}
                    </td>
                </tr>
            `;
    });

    tbody.innerHTML = rows;
  } catch (error) {
    console.error(error);
    tbody.innerHTML =
      '<tr><td colspan="3" class="text-danger text-center">Failed to load logs</td></tr>';
  }
}
