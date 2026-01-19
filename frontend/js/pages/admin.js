import { Auth } from '../modules/auth.js';
import { Api } from '../modules/api.js';
import { CONFIG } from '../modules/config.js';

// --- Helper ---
const ACTION_LABELS = {
    LOGIN: "Logged In",
    INSERT_EMPLOYEE: "Inserted Employee",
    UPDATE_EMPLOYEE: "Updated Employee",
    DELETE_EMPLOYEE: "Deleted Employee",
    BULK_INSERT: "Bulk Employee Import",
    CLEAR_TABLE: "Cleared an Employee Table",
    REVERSE_LIST: "Reversed Employee List"
};

const ACTION_COLORS = {
    LOGIN: "text-info",
    INSERT_EMPLOYEE: "text-success",
    UPDATE_EMPLOYEE: "text-warning",
    DELETE_EMPLOYEE: "text-danger",
    BULK_INSERT: "text-success",
    CLEAR_TABLE: "text-danger",
    REVERSE_LIST: "text-secondary"
};

const formatAction = (action) =>
    ACTION_LABELS[action] || action.replace(/_/g, " ");

const formatTime = (iso) =>
    new Date(iso).toLocaleString("en-IN", {
        dateStyle: "medium",
        timeStyle: "short"
    });


// 1. Session Check
Auth.checkSession();

// 2. Event Listeners
document.addEventListener('DOMContentLoaded', () => {
    loadTableList();
    
    // Attach Logout
    document.getElementById('logoutBtn').addEventListener('click', Auth.logout);

    // Attach Refresh Logs
    document.getElementById('refreshLogsBtn').addEventListener('click', loadAuditLogs);
});

// --- VIEW SWITCHER ---
window.switchView = function(viewName, e) {
    // 1. Update Buttons
    document.querySelectorAll('.nav-link').forEach(btn => btn.classList.remove('active'));

    // 2. Add active class to the clicked button using 'e.currentTarget'
    if(e){
        e.currentTarget.classList.add('active');
    }

    // 3. Show Section
    document.querySelectorAll('.view-section').forEach(el => el.classList.remove('active'));
    document.getElementById(`view-${viewName}`).classList.add('active');

    // 4. Load Data if needed
    if(viewName === 'logs') loadAuditLogs();
    if(viewName === 'tables') loadTableList();
}

// --- LOGIC: TABLES ---
async function loadTableList() {
    const grid = document.getElementById('tableGrid');
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
            tables.forEach(name => {
                const card = `
                <div class="table-card" onclick="window.location.href='table_view.html?table_id=${name}'">
                    <i class="fa-solid fa-database fa-2x mb-2 text-secondary"></i>
                    <h3>${name}</h3>
                    <small>Click to Manage</small>
                </div>`;
                grid.innerHTML += card;
            });
        }
    } catch (error) {
        console.error(error);
        grid.innerHTML = '<p class="text-danger">Failed to load tables.</p>';
    }
}

window.createNewTable = async function() {
    const nameInput = document.getElementById('newTableName');
    const name = nameInput.value.trim();
    
    if(!name) return alert("Enter a name");
    
    try {
        // 1. Register table in Node
        const response = await Api.createTable(name);
        
        if (response.ok) {
            // 2. If success, go to the table view
            window.location.href = `table_view.html?table_id=${name}`;
        } else {
            const err = await response.json();
            alert("Error: " + (err.message || "Could not create table"));
        }
    } catch (e) {
        console.error(e);
        alert("Network Error");
    }
}

// --- LOGIC: LOGS ---
async function loadAuditLogs() {
    const tbody = document.getElementById('logBody');
    tbody.innerHTML = '<tr><td colspan="3" class="text-center">Loading...</td></tr>';

    try {
        const response = await fetch(`${CONFIG.API_URL}/api/history`, {
            headers: { 'Authorization': `Bearer ${Auth.getToken()}` }
        });

        const logs = await response.json();
        tbody.innerHTML = "";

        if (!logs || logs.length === 0) {
            tbody.innerHTML = '<tr><td colspan="3" class="text-center">No activity recorded.</td></tr>';
            return;
        }

        logs.forEach(log => {
            const color = ACTION_COLORS[log.action] || "text-secondary";

            const row = `
                <tr>
                    <td>${formatTime(log.timestamp)}</td>
                    <td>${log.username}</td>
                    <td class="fw-bold ${color}">
                        ${formatAction(log.action)}
                    </td>
                </tr>
            `;

            tbody.innerHTML += row;
        });

    } catch (error) {
        console.error(error);
        tbody.innerHTML =
            '<tr><td colspan="3" class="text-danger text-center">Failed to load logs</td></tr>';
    }
}
