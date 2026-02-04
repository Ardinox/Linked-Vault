// frontend/js/pages/.js

import { Auth } from "../modules/auth.js";
import { Api } from "../modules/api.js";

// --- 1. IMMEDIATE SECURITY CHECK (Fail Fast) ---
Auth.checkSession();

// --- 2. INITIALIZATION ---
const params = new URLSearchParams(window.location.search);
const TABLE_ID = params.get("table_id");
const TABLE_NAME = params.get("name");

// If no Table ID, we can't do anything. Stop here.
if (!TABLE_ID) {
  alert("Access Denied: No Table ID provided.");
  window.location.href = "../views/admin.html";
  throw new Error("No Table ID");
}

// Update Title if element exists
const titleEl = document.getElementById("tableTitle");
if (titleEl && TABLE_NAME) titleEl.textContent = TABLE_NAME;

// State for Toggle View
let isRecursiveView = false;

// --- 3. PAGE LOAD HANDLER ---
document.addEventListener("DOMContentLoaded", () => {
  // Only fetch Navbar if we have a place to put it
  if (document.getElementById("navbar-container")) {
    loadNavbar();
  }

  // --- FLASH MESSAGE LOGIC ---
  if (params.get("action") === "updated") {
    const container =
      document.querySelector(".container-fluid") || document.body;
    const banner = document.createElement("div");
    banner.className = "alert alert-success text-center fw-bold";
    banner.innerText = "✅ Employee Updated Successfully";
    container.prepend(banner);
    setTimeout(() => banner.remove(), 3000);
    const newUrl = window.location.href.replace("&action=updated", "");
    window.history.replaceState({}, document.title, newUrl);
  }

  // Safety Net: Prevent Enter key from reloading the page
  const form = document.querySelector("form");
  if (form) form.addEventListener("submit", (e) => e.preventDefault());

  // A. ROUTER: Table View
  if (document.getElementById("tableBody")) {
    window.loadStandardTable();
  }

  // B. ROUTER: Add Employee Page
  const addBtn = document.getElementById("addBtn");
  if (addBtn) {
    restoreAddForm();

    // Select all inputs
    const inputs = document.querySelectorAll(
      "#id, #name, #age, #department, #salary",
    );

    inputs.forEach((input) => {
      // 1. Auto-save drafts
      input.addEventListener("input", () =>
        sessionStorage.setItem("add_" + input.id, input.value),
      );

      // 2. UX IMPROVEMENT: Hit Enter to Submit
      input.addEventListener("keydown", (e) => {
        if (e.key === "Enter") {
          e.preventDefault(); // Stop form submit
          addBtn.click(); // Trigger the Add Button
        }
      });
    });

    addBtn.addEventListener("click", addData);
  }

  // C. ROUTER: Update Employee Page
  const updateBtn = document.getElementById("updateBtn");
  if (updateBtn) {
    const idToUpdate = params.get("id");
    if (idToUpdate) loadEmployeeForUpdate(idToUpdate);

    // UX IMPROVEMENT: Hit Enter to Update
    const inputs = document.querySelectorAll(
      "#id, #name, #age, #department, #salary",
    );
    inputs.forEach((input) => {
      input.addEventListener("keydown", (e) => {
        if (e.key === "Enter") {
          e.preventDefault();
          updateBtn.click(); // Trigger the Update Button
        }
      });
    });

    updateBtn.addEventListener("click", performUpdate);
  }

  // D. ROUTER: Close/Clear Button
  const clearBtn = document.getElementById("clearBtn");
  if (clearBtn) {
    clearBtn.addEventListener("click", () => {
      clearAddForm();
      window.location.href = `table_view.html?table_id=${TABLE_ID}&name=${encodeURIComponent(TABLE_NAME || "")}`;
    });
  }

  // E. ROUTER: Search Bar
  const searchInput = document.getElementById("searchId");
  if (searchInput) {
    searchInput.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        e.preventDefault();
        window.searchData();
      }
    });
  }
});

// --- HELPER FUNCTIONS (Form Persistence) ---
function restoreAddForm() {
  ["id", "name", "age", "department", "salary"].forEach((field) => {
    const saved = sessionStorage.getItem("add_" + field);
    const el = document.getElementById(field);
    if (saved && el) el.value = saved;
  });
}

function clearAddForm() {
  ["id", "name", "age", "department", "salary"].forEach((field) => {
    sessionStorage.removeItem("add_" + field);
    const el = document.getElementById(field);
    if (el) el.value = "";
  });
  const pos = document.getElementById("pos");
  if (pos) pos.value = -1;
}

// --- NAVBAR LOADER ---
async function loadNavbar() {
  try {
    const response = await fetch("../components/navbar.html");
    if (!response.ok) return;
    const data = await response.text();
    const container = document.getElementById("navbar-container");

    // Inject and Highlight Active Link
    container.outerHTML = data;
    const currentFile = window.location.pathname.split("/").pop();
    document.querySelectorAll(".nav-link").forEach((link) => {
      const href = link.getAttribute("href");
      if (href && href !== "#") {
        const separator = href.includes("?") ? "&" : "?";
        link.setAttribute(
          "href",
          `${href}${separator}table_id=${TABLE_ID}&name=${encodeURIComponent(TABLE_NAME || "")}`,
        );
      }
      if (href && href.includes(currentFile)) {
        link.classList.add("active");
        link.style.fontWeight = "bold";
      }
    });
  } catch (error) {
    console.error("Navbar Error", error);
  }
}

// --- DATA OPERATIONS ---

// --- ADD DATA ---
async function addData() {
  const btn = document.getElementById("addBtn");

  // Collect Values
  const pos = document.getElementById("pos").value;
  const name = document.getElementById("name").value;
  const id = document.getElementById("id").value;
  const age = document.getElementById("age").value;
  const dept = document.getElementById("department").value;
  const salary = document.getElementById("salary").value;

  if (!id || !name || !age || !dept || !salary)
    return alert("Please fill in all fields.");

  btn.innerText = "Adding...";
  btn.disabled = true;

  const payload = {
    table_id: parseInt(TABLE_ID),
    position: parseInt(pos) || -1,
    data: {
      id: parseInt(id),
      name,
      age: parseInt(age),
      department: dept,
      salary: parseInt(salary),
    },
  };

  try {
    const response = await Api.insert(payload);
    // http.js handles 401 checks automatically
    if (response.ok) {
      alert(`Success! Employee ${name} added.`);
      clearAddForm();
    } else {
      const err = await response.json();
      alert("Error: " + (err.message || "Insert Failed"));
    }
  } catch (error) {
    console.error(error);
    alert("Network Error");
  } finally {
    btn.innerText = "Add";
    btn.disabled = false;
  }
}

// --- UPDATE DATA ---
async function loadEmployeeForUpdate(id) {
  try {
    // 1. Fetch data (Now returns an Array: [ { ... } ])
    const results = await Api.search(id, TABLE_ID);

    // 2. Check if the array is empty
    if (!results || results.length === 0) {
      alert("Employee not found");
      window.location.href = `table_view.html?table_id=${TABLE_ID}&name=${encodeURIComponent(TABLE_NAME || "")}`;
      return;
    }

    // 3. Extract the first item (The actual employee object)
    const data = results[0];
    // Auto-fill form
    document.getElementById("originalId").value = data.id;
    document.getElementById("id").value = data.id;
    document.getElementById("name").value = data.name;
    document.getElementById("age").value = data.age;
    document.getElementById("department").value = data.department;
    document.getElementById("salary").value = data.salary;
    document.getElementById("pos").value = params.get("pos") || -1;
  } catch (error) {
    console.error(error);
  }
}

async function performUpdate() {
  const btn = document.getElementById("updateBtn");
  if (!confirm("Overwrite this record?")) return;

  btn.innerText = "Processing...";
  btn.disabled = true;

  // Minimized Payload Construction
  const payload = {
    table_id: parseInt(TABLE_ID),
    original_id: parseInt(document.getElementById("originalId").value),
    position: parseInt(document.getElementById("pos").value),
    data: {
      id: parseInt(document.getElementById("id").value),
      name: document.getElementById("name").value,
      age: parseInt(document.getElementById("age").value),
      department: document.getElementById("department").value,
      salary: parseInt(document.getElementById("salary").value),
    },
  };

  try {
    const response = await Api.update(payload);
    if (response.ok) {
      window.location.replace(
        `table_view.html?table_id=${TABLE_ID}&name=${encodeURIComponent(TABLE_NAME || "")}&action=updated`,
      );
    } else {
      const err = await response.json();
      alert("Update Failed: " + (err.message || "Unknown error"));
      btn.innerText = "Update";
      btn.disabled = false;
    }
  } catch (error) {
    console.error(error);
    alert("Network Error");
    btn.innerText = "Update";
    btn.disabled = false;
  }
}

// --- TABLE FUNCTIONS ---
window.loadStandardTable = async function () {
  try {
    // 1. Fetch & Parse (Handled by Api + http.js)
    const data = await Api.getAll(TABLE_ID);
    // 2. Render
    renderTable(data);
  } catch (error) {
    console.error("Load Table Error:", error);
  }
};

window.searchData = async function () {
  const query = document.getElementById("searchId").value.trim();
  if (!query) return alert("Please enter a Name, ID or Department");
  try {
    const data = await Api.search(query, TABLE_ID);
    renderTable(data);
  } catch (error) {
    console.error(error);
  }
};

window.deleteEmp = async function (id) {
  if (!confirm("Delete ID " + id + "?")) return;
  try {
    const response = await Api.delete(id, TABLE_ID);
    // If recursive view is active, reload it. Otherwise, reload standard table.
    if (response.ok)
      isRecursiveView ? window.recursiveReverse() : window.loadStandardTable();
  } catch (error) {
    console.error(error);
  }
};

window.linkedReverse = async function () {
  isRecursiveView = false; // Reset toggle
  try {
    const response = await Api.reverse(TABLE_ID);
    if (response.ok) window.loadStandardTable();
  } catch (error) {
    console.error(error);
  }
};

window.recursiveReverse = async function () {
  isRecursiveView = !isRecursiveView; // Toggle State

  // Toggle UI Arrows
  document
    .getElementById("arrow_up")
    ?.classList.toggle("d-none", !isRecursiveView);
  document
    .getElementById("arrow_down")
    ?.classList.toggle("d-none", isRecursiveView);

  if (!isRecursiveView) return window.loadStandardTable();

  try {
    const data = await Api.recursiveReverse(TABLE_ID);
    renderTable(data);
  } catch (error) {
    isRecursiveView = false;
    window.loadStandardTable();
  }
};

window.downloadTable = async function () {
  try {
    // Manual Fetch required for Blobs (bypassing http.js JSON parsing)
    const response = await fetch(Api.getDownloadUrl(TABLE_ID), {
      headers: { Authorization: `Bearer ${Auth.getToken()}` },
    });

    // Manual Security Check required here
    if (response.status === 401) {
      Auth.logout();
      return;
    }
    if (!response.ok) return alert("Download Failed");

    // Create Download Link
    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `table_${TABLE_ID}.csv`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    window.URL.revokeObjectURL(url);
  } catch (e) {
    console.error(e);
  }
};

window.uploadCsv = () => document.getElementById("hiddenFileInput").click();

window.send_file_to_backend = async function () {
  const file = document.getElementById("hiddenFileInput").files[0];
  if (!file) return;

  const reader = new FileReader();
  reader.onload = async function (e) {
    try {
      const response = await Api.uploadCsv(TABLE_ID, e.target.result);
      if (response.ok) {
        const res = await response.json();
        alert(`Added: ${res.added}, Skipped: ${res.skipped}`);
        window.loadStandardTable();
      } else alert("Upload Failed");
    } catch (err) {
      console.error(err);
    }
  };
  reader.readAsText(file);
};

window.deleteData = async function () {
  if (
    !confirm(
      "⚠️ WARNING: This will PERMANENTLY delete this table and all its contents.",
    )
  )
    return;
  try {
    const response = await Api.clearTable(TABLE_ID);
    if (response.ok) {
      alert("Table deleted successfully.");
      window.location.href = "../views/admin.html";
    } else {
      const err = await response.json();
      alert("Failed to delete table: " + (err.message || "Unknown error"));
    }
  } catch (e) {
    console.error(e);
    alert("Network Error");
  }
};

function renderTable(data) {
  const tbody = document.getElementById("tableBody");
  if (!tbody) return;
  tbody.innerHTML = "";

  if (!data || data.length === 0) {
    tbody.innerHTML =
      "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }

  const list = Array.isArray(data) ? data : [data];
  list.forEach((emp, index) => {
    // Dynamic SN calculation based on View Mode
    let sn = isRecursiveView ? list.length - index : index + 1;

    const row = `<tr>
            <td>${sn}</td>
            <td>${emp.name}</td>
            <td>${emp.id}</td>
            <td>${emp.age}</td>
            <td>${emp.department}</td>
            <td>$${emp.salary}</td>
            <td>
                <button class="btn btn-danger btn-sm" onclick="deleteEmp(${emp.id})">Delete</button>
                <a class="btn btn-dark btn-sm" href="update_emp.html?id=${emp.id}&pos=${index}&table_id=${TABLE_ID}&name=${encodeURIComponent(TABLE_NAME || "")}">Update</a>
            </td>
        </tr>`;
    tbody.innerHTML += row;
  });
}
