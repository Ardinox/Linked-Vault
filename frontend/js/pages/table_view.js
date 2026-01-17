import { Auth } from "../modules/auth.js";
import { Api } from "../modules/api.js";

// --- 1. INITIALIZATION ---
const params = new URLSearchParams(window.location.search);
const TABLE_ID = params.get("table_id");

if (!TABLE_ID) {
  alert("Access Denied: No Table ID provided.");
  window.location.href = "../index.html";
  throw new Error("No Table ID");
}

let isRecursiveView = false;

// --- 2. PAGE LOAD HANDLER ---
document.addEventListener("DOMContentLoaded", () => {
  Auth.checkSession();
  loadNavbar();

  // --- FLASH MESSAGE LOGIC ---
  if (params.get("action") === "updated") {
    const container = document.querySelector(".container-fluid") || document.body;
    const banner = document.createElement("div");
    banner.className = "alert alert-success text-center fw-bold";
    banner.innerText = "✅ Employee Updated Successfully";
    container.prepend(banner);
    setTimeout(() => banner.remove(), 3000);
    const newUrl = window.location.href.replace("&action=updated", "");
    window.history.replaceState({}, document.title, newUrl);
  }

  // Prevent default form submission
  const form = document.querySelector("form");
  if (form) {
    form.addEventListener("submit", (e) => e.preventDefault());
  }

  // A. Logic for "Table View" Page
  if (document.getElementById("tableBody")) {
    window.loadStandardTable();
  }

  // B. Logic for "Add Employee" Page
  const addBtn = document.getElementById("addBtn");
  if (addBtn) {
    // 1. RESTORE DATA
    restoreAddForm();

    // 2. AUTO-SAVE DATA
    const inputs = document.querySelectorAll("#id, #name, #age, #department, #salary");
    inputs.forEach((input) => {
      input.addEventListener("input", () => {
        sessionStorage.setItem("add_" + input.id, input.value);
      });
    });

    // 3. Attach Add Listener
    addBtn.addEventListener("click", (e) => addData(e));
  }

  // C. Logic for "Close / Clear" Button (THE FIX)
  const clearBtn = document.getElementById("clearBtn");
  if (clearBtn) {
    clearBtn.addEventListener("click", () => {
        // 1. Wipe the storage and inputs
        clearAddForm();
        
        // 2. Redirect back to Table View
        window.location.href = `table_view.html?table_id=${TABLE_ID}`;
    });
  }

  // D. Logic for "Update Employee" Page
  const updateBtn = document.getElementById("updateBtn");
  if (updateBtn) {
    const idToUpdate = params.get("id");
    if (idToUpdate) {
      loadEmployeeForUpdate(idToUpdate);
    }
    updateBtn.addEventListener("click", (e) => performUpdate(e));
  }

  // Search Listener
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

// --- HELPER FUNCTIONS (Must be outside DOMContentLoaded to be accessible) ---

function restoreAddForm() {
    const fields = ["id", "name", "age", "department", "salary"];
    fields.forEach(field => {
        const saved = sessionStorage.getItem("add_" + field);
        const el = document.getElementById(field);
        if (saved && el) {
            el.value = saved;
        }
    });
}

function clearAddForm() {
    const fields = ["id", "name", "age", "department", "salary"];
    fields.forEach(field => {
        // Remove from Storage
        sessionStorage.removeItem("add_" + field);
        // Remove from UI
        const el = document.getElementById(field);
        if (el) el.value = "";
    });
    // Reset position helper
    const pos = document.getElementById("pos");
    if(pos) pos.value = -1;
}

// --- 3. NAVBAR LOADER ---
async function loadNavbar() {
  try {
    const response = await fetch("../components/navbar.html");
    const data = await response.text();
    const container = document.getElementById("navbar-container");
    if (container) {
      container.outerHTML = data;
      const navLinks = document.querySelectorAll(".nav-link");
      const currentFile = window.location.pathname.split("/").pop();
      navLinks.forEach((link) => {
        const href = link.getAttribute("href");
        if (href && href !== "#") {
          const separator = href.includes("?") ? "&" : "?";
          link.setAttribute("href", `${href}${separator}table_id=${TABLE_ID}`);
        }
        if (href && href.includes(currentFile)) {
            link.classList.add("active");
            link.style.fontWeight = "bold";
        }
      });
    }
  } catch (error) {
    console.error("Navbar Error", error);
  }
}

// --- 4. DATA OPERATIONS ---

// --- ADD DATA ---
async function addData(e) {
  if(e) { e.preventDefault(); e.stopPropagation(); }
  
  const btn = document.getElementById("addBtn");
  const pos = document.getElementById("pos").value;
  const name = document.getElementById("name").value;
  const id = document.getElementById("id").value;
  const age = document.getElementById("age").value;
  const dept = document.getElementById("department").value;
  const salary = document.getElementById("salary").value;

  if (!id || !name || !age || !dept || !salary) {
    return alert("Please fill in all fields.");
  }

  btn.innerText = "Adding...";
  btn.disabled = true;

  const payload = {
    table_id: TABLE_ID,
    position: parseInt(pos) || -1,
    data: {
      id: parseInt(id),
      name: name,
      age: parseInt(age),
      department: dept,
      salary: parseInt(salary),
    },
  };

  try {
    const response = await Api.insert(payload);
    
    if (response.status === 401) {
        alert("Session Expired");
        Auth.logout();
        return;
    }

    if (response.ok) {
      alert(`Success! Employee ${name} added.`);
      // Clear data on success
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
    const data = await Api.search(id, TABLE_ID);
    if (!data) {
      alert("Employee not found");
      window.location.href = `table_view.html?table_id=${TABLE_ID}`;
      return;
    }
    
    const elOriginalId = document.getElementById("originalId");
    const elId = document.getElementById("id");
    const elPos = document.getElementById("pos");

    if (elOriginalId && elId) {
        elOriginalId.value = data.id;
        elId.value = data.id;
        document.getElementById("name").value = data.name;
        document.getElementById("age").value = data.age;
        document.getElementById("department").value = data.department;
        document.getElementById("salary").value = data.salary;
        if (elPos) elPos.value = params.get("pos") || -1;
    }
  } catch (error) { console.error(error); }
}

async function performUpdate(e) {
  if (e) { e.preventDefault(); e.stopPropagation(); }

  const btn = document.getElementById("updateBtn");
  const originalId = document.getElementById("originalId").value;
  const newPos = document.getElementById("pos").value;
  const newId = document.getElementById("id").value;
  const newName = document.getElementById("name").value;
  const newAge = document.getElementById("age").value;
  const newDept = document.getElementById("department").value;
  const newSalary = document.getElementById("salary").value;

  if (!confirm("Overwrite this record?")) return;

  if (btn) { btn.innerText = "Processing..."; btn.disabled = true; }

  const payload = {
    table_id: TABLE_ID,
    original_id: parseInt(originalId),
    position: parseInt(newPos),
    data: {
      id: parseInt(newId),
      name: newName,
      age: parseInt(newAge),
      department: newDept,
      salary: parseInt(newSalary)
    }
  };

  try {
    const response = await Api.update(payload);

    if (response.status === 401) {
       alert("Session expired.");
       Auth.logout();
       return;
    }

    if (response.ok) {
        window.location.replace(`table_view.html?table_id=${TABLE_ID}&action=updated`);
    } else {
        const err = await response.json();
        alert("Update Failed: " + (err.message || "Unknown error"));
        if (btn) { btn.innerText = "Update"; btn.disabled = false; }
    }
  } catch (error) {
    console.error(error);
    alert("Network Error");
    if (btn) { btn.innerText = "Update"; btn.disabled = false; }
  }
}

// --- TABLE FUNCTIONS ---
window.loadStandardTable = async function() {
  try {
    const data = await Api.getAll(TABLE_ID);
    renderTable(data);
  } catch (error) { console.error(error); }
};

window.searchData = async function () {
  const id = document.getElementById("searchId").value.trim();
  if (!id) return alert("Enter ID");
  try {
    const data = await Api.search(id, TABLE_ID);
    renderTable(data ? [data] : []);
  } catch (error) { console.error(error); }
};

window.deleteEmp = async function (id) {
  if (!confirm("Delete ID " + id + "?")) return;
  try {
    const response = await Api.delete(id, TABLE_ID);
    if (response.ok) isRecursiveView ? window.recursiveReverse() : window.loadStandardTable();
  } catch (error) { console.error(error); }
};

window.linkedReverse = async function () {
  isRecursiveView = false;
  try {
    const response = await Api.reverse(TABLE_ID);
    if (response.ok) window.loadStandardTable();
  } catch (error) { console.error(error); }
};

window.recursiveReverse = async function () {
  isRecursiveView = !isRecursiveView;
  document.getElementById("arrow_up")?.classList.toggle("d-none", !isRecursiveView);
  document.getElementById("arrow_down")?.classList.toggle("d-none", isRecursiveView);
  if (!isRecursiveView) return window.loadStandardTable();
  try {
    const data = await Api.recursiveReverse(TABLE_ID);
    renderTable(data);
  } catch (error) { isRecursiveView = false; window.loadStandardTable(); }
};

window.downloadTable = async function () {
  try {
    const response = await fetch(Api.getDownloadUrl(TABLE_ID), {
      headers: { Authorization: `Bearer ${Auth.getToken()}` },
    });
    if (!response.ok) return alert("Download Failed");
    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `table_${TABLE_ID}.csv`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    window.URL.revokeObjectURL(url);
  } catch (e) { console.error(e); }
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
    } catch (e) { console.error(e); }
  };
  reader.readAsText(file);
};

window.deleteData = async function () {
  if (!confirm("Wipe entire table?")) return;
  try {
    const response = await Api.clearTable(TABLE_ID);
    if (response.ok) window.loadStandardTable();
  } catch (e) { console.error(e); }
};

function renderTable(data) {
  const tbody = document.getElementById("tableBody");
  if (!tbody) return;
  tbody.innerHTML = "";
  if (!data || data.length === 0) {
    tbody.innerHTML = "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }
  const list = Array.isArray(data) ? data : [data];
  list.forEach((emp, index) => {
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
                <a class="btn btn-dark btn-sm" href="update_emp.html?id=${emp.id}&pos=${index}&table_id=${TABLE_ID}">Update</a>
            </td>
        </tr>`;
    tbody.innerHTML += row;
  });
}