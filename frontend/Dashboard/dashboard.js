const params = new URLSearchParams(window.location.search);
const TABLE_ID = params.get("table_id");

// 2. PROTECTION: Kick out if missing
if (!TABLE_ID) {
  alert("Access Denied: No Table ID provided.");
  // Go up one level (..) to get out of /Dashboard and back to index.html
  window.location.href = "../index.html";

  // Throw an error to stop the rest of this script from running
  throw new Error("Halted: No Table ID");
}

const API_URL = "http://localhost:8000";

// --- GLOBAL STATE ---
let isRecursiveView = false;

// --- GLOBAL EVENT LISTENER (The Router) ---
document.addEventListener("DOMContentLoaded", () => {
  // "Close" button on Add Page
  const closeBtn = document.getElementById("closeBtn");
  if (closeBtn) {
    closeBtn.setAttribute("href", `table_view.html?table_id=${TABLE_ID}`);
  }

  // "Close" button on Update Page
  const closeUpdateBtn = document.getElementById("closeUpdateBtn");
  if (closeUpdateBtn) {
    closeUpdateBtn.setAttribute("href", `table_view.html?table_id=${TABLE_ID}`);
  }

  // If we are on Home Page (Table exists)
  if (document.getElementById("tableBody")) {
    loadStandardTable();
  }

  // If we are on Add Page
  const addBtn = document.getElementById("addBtn");
  if (addBtn) {
    addBtn.type = "button";
    addBtn.addEventListener("click", (e) => addData(e));
  }

  // If we are on Update Page
  const updateBtn = document.getElementById("updateBtn");
  if (updateBtn) {
    updateBtn.type = "button";
    const urlParams = new URLSearchParams(window.location.search);
    const idToUpdate = urlParams.get("id");

    if (idToUpdate) {
      loadEmployeeForUpdate(idToUpdate);
    } else {
      alert("No ID provided!");
      window.location.href = `table_view.html?table_id=${TABLE_ID}`;
    }

    updateBtn.addEventListener("click", (e) => performUpdate(e));
  }
});

// --- Navbar Integration ---
fetch("navbar.html")
  .then((response) => response.text())
  .then((data) => {
    let navbar = document.getElementById("navbar-container");

    navbar.outerHTML = data;

    const currentPath = window.location.pathname.split("/").pop();

    const navLinks = document.querySelectorAll(".nav-link");

    navLinks.forEach((link) => {
      const originalHref = link.getAttribute("href");
      if (originalHref && originalHref !== "#") {
        const separator = originalHref.includes("?") ? "&" : "?";
        link.setAttribute(
          "href",
          `${originalHref}${separator}table_id=${TABLE_ID}`
        );
      }

      if (originalHref === currentPath) {
        link.classList.add("active");
      }
    });
  });

// --- HELPER: Status Messages ---
function showStatus(message, type) {
  const el =
    document.getElementById("status") || document.getElementById("statusMsg");
  if (el) {
    el.style.display = "block";
    el.className =
      type === "success" ? "alert alert-success" : "alert alert-danger";
    el.innerText = message;
  } else {
    console.log(type + ": " + message);
  }
}

// --- HELPER: Input Field Clearing ---
function clearAllField() {
  const fields = ["pos", "name", "id", "age", "department", "salary"];
  fields.forEach((f) => {
    const el = document.getElementById(f);
    if (el) el.value = f === "pos" ? -1 : "";
  });
}

// --- CORE: RENDER TABLE (UI Logic) ---
function renderTable(data) {
  const tbody = document.getElementById("tableBody");
  tbody.innerHTML = "";

  // Handle empty or undefined data
  if (!data || data.length === 0) {
    tbody.innerHTML =
      "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }
  if (Array.isArray(data) && data.length === 0) {
    tbody.innerHTML =
      "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }
  if (!Array.isArray(data) && data.id === undefined) {
    tbody.innerHTML =
      "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }

  // Handle API returning a single object instead of array (for Search)
  const dataArray = Array.isArray(data) ? data : [data];

  dataArray.forEach((emp, index) => {
    // LOGIC: If Recursive View, count down. If Normal, count up.
    let serialNumber = isRecursiveView ? dataArray.length - index : index + 1;

    const row = `<tr>
            <td>${serialNumber}</td>
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

// --- 1. ADD DATA ---
async function addData(e) {
  if (e) e.preventDefault();

  const addBtn = document.getElementById("addBtn");

  const pos = document.getElementById("pos").value;
  const name = document.getElementById("name").value;
  const id = document.getElementById("id").value;
  const age = document.getElementById("age").value;
  const dept = document.getElementById("department").value;
  const salary = document.getElementById("salary").value;

  if (!id || !name || !age || !dept || !salary) {
    showStatus("All fields must be filled.", "error");
    return;
  }

  if (addBtn) {
    addBtn.disabled = true;
    addBtn.innerText = "Adding...";
  }

  const payload = {
    table_id: TABLE_ID,
    position: parseInt(pos) || -1,
    data: {
      id: parseInt(id),
      name: name,
      age: parseInt(age) || 0,
      department: dept,
      salary: parseInt(salary) || 0,
    },
  };

  try {
    const response = await fetch(`${API_URL}/insert`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });
    const result = await response.json();

    if (response.ok) {
      clearAllField();
      showStatus(`Success! Added Employee ID ${result.id}`, "success");
    } else {
      showStatus("Error: " + (result.message || "Unknown error"), "error");
    }
  } catch (error) {
    console.error("Connection Error:", error);
    showStatus("Failed to connect to server.", "error");
  } finally {
    if (addBtn) {
      addBtn.disabled = false;
      addBtn.innerText = "Add";
    }
  }
}

// --- 2. LOAD STANDARD TABLE (Fetch /show) ---
async function loadStandardTable() {
  try {
    const response = await fetch(`${API_URL}/show?table_id=${TABLE_ID}`, {
      method: "GET",
    });
    if (!response.ok) {
      const errData = await response.json();
      console.error("Server Error:", errData);
      alert(`Failed to load table: ${errData.message || response.statusText}`);
      return;
    }
    const data = await response.json();
    renderTable(data);
  } catch (error) {
    console.error("Error loading table:", error);
  }
}

// --- 3. DELETE ---
window.deleteEmp = async function (id) {
  if (id === undefined || id === null) return;
  if (!confirm("Delete ID " + id + "?")) return;

  try {
    const response = await fetch(
      `${API_URL}/delete?id=${id}&table_id=${TABLE_ID}`,
      {
        method: "DELETE",
        headers: { "Content-Type": "application/json" },
      }
    );

    if (response.ok) {
      // Refresh the table based on current view
      if (isRecursiveView) recursiveReverse();
      else loadStandardTable();
    } else {
      const result = await response.json();
      alert("Failed: " + (result.message || "Unknown Error"));
    }
  } catch (error) {
    console.error("Delete Error:", error);
  }
};

// --- 4. LOAD DATA FOR UPDATE FORM ---
async function loadEmployeeForUpdate(id) {
  try {
    const response = await fetch(
      `${API_URL}/search?id=${id}&table_id=${TABLE_ID}`,
      {
        method: "GET",
        headers: { "Content-Type": "application/json" },
      }
    );
    if (response.status === 404) {
      alert("Record not found. Returning to table view.");
      window.location.replace(`table_view.html?table_id=${TABLE_ID}`);
      return;
    }

    const data = await response.json();

    if (response.ok) {
      document.getElementById("originalId").value = data.id;
      document.getElementById("id").value = data.id;
      document.getElementById("name").value = data.name;
      document.getElementById("age").value = data.age;
      document.getElementById("department").value = data.department;
      document.getElementById("salary").value = data.salary;

      const urlParams = new URLSearchParams(window.location.search);
      const currentPos = urlParams.get("pos");
      document.getElementById("pos").value =
        currentPos !== null ? currentPos : -1;
    } else {
      showStatus("Error loading data: " + data.message, "error");
    }
  } catch (error) {
    console.error("Load Error:", error);
    showStatus("Load Error (Check Console)", "error");
  }
}

// --- 5. PERFORM UPDATE (Delete Old -> Insert New) ---
async function performUpdate(e) {
  if (e) {
    e.preventDefault();
    e.stopPropagation();
  }

  const updateBtn = document.getElementById("updateBtn");
  let isRedirecting = false;

  const originalId = document.getElementById("originalId").value;
  const newPos = document.getElementById("pos").value;
  const newId = document.getElementById("id").value;
  const newName = document.getElementById("name").value;
  const newAge = document.getElementById("age").value;
  const newDept = document.getElementById("department").value;
  const newSalary = document.getElementById("salary").value;

  // Safety Checks
  if (!newId || !newName) {
    showStatus("ID and Name required.", "error");
    return;
  }

  if (!originalId) {
    showStatus("Original ID missing. Cannot update.", "error");
    return;
  }

  if (!confirm("Overwrite this employee record?")) return;

  if (updateBtn) {
    updateBtn.disabled = true;
    updateBtn.innerText = "Updating...";
  }

  try {
    // Step 1: Delete Old
    const delRes = await fetch(
      `${API_URL}/delete?id=${originalId}&table_id=${TABLE_ID}`,
      {
        method: "DELETE",
        headers: { "Content-Type": "application/json" },
      }
    );

    if (!delRes.ok) {
      const err = await delRes.json();
      showStatus("Update Failed (Delete Step): " + err.message, "error");
      if (updateBtn) {
        updateBtn.disabled = false;
        updateBtn.innerText = "Update";
      }
      return;
    }

    // Step 2: Insert New
    const payload = {
      table_id: TABLE_ID,
      position: parseInt(newPos),
      data: {
        id: parseInt(newId),
        name: newName,
        age: parseInt(newAge),
        department: newDept,
        salary: parseInt(newSalary),
      },
    };

    const insRes = await fetch(`${API_URL}/insert`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    });

    if (insRes.ok) {
      isRedirecting = true;
      showStatus("Update Successful!");
      window.location.replace(`table_view.html?table_id=${TABLE_ID}`);
    } else {
      const res = await insRes.json();
      showStatus("Update Failed (Insert Step): " + res.message, "error");
    }
  } catch (error) {
    console.error("Update Error:", error);
    showStatus("Network Error.", "error");
  } finally {
    if (!isRedirecting && updateBtn) {
      updateBtn.disabled = false;
      updateBtn.innerHTML = "Update";
    }
  }
}

// --- 6. SEARCH EMPLOYEE ---
async function searchData() {
  const id = document.getElementById("searchId");
  const idValue = id.value.trim();
  const tbody = document.getElementById("tableBody");

  if (idValue.length === 0) {
    alert("Search field is Empty");
    return;
  }
  try {
    const response = await fetch(
      `${API_URL}/search?id=${idValue}&table_id=${TABLE_ID}`,
      {
        method: "GET",
        headers: { "Content-Type": "application/json" },
      }
    );

    if (response.status === 404) {
      tbody.innerHTML =
        "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
      return;
    }

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();

    // REUSE RENDER LOGIC
    renderTable(data);
  } catch (error) {
    console.error("Error: ", error);
    alert("Something went wrong while fetching data.");
  }
}

// --- 7. LINKED REVERSE (Iterative / Physical Reverse) ---
async function linkedReverse() {
  isRecursiveView = false; // Reset flag, because this physically changes the list

  try {
    const response = await fetch(
      `${API_URL}/linkedreverse?table_id=${TABLE_ID}`,
      {
        method: "PUT",
      }
    );

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    loadStandardTable();
  } catch (error) {
    console.error("Error: ", error);
    alert("Something went wrong!");
  }
}

// --- 8. RECURSIVE REVERSE  ---
async function recursiveReverse() {
  // 1. Toggle the Global Flag
  isRecursiveView = !isRecursiveView;

  // 2. Handle Icons (Toggle Visibility)
  const up = document.getElementById("arrow_up");
  const down = document.getElementById("arrow_down");

  // Make sure elements exist before trying to toggle classes
  if (up && down) {
    if (isRecursiveView) {
      // Recursive Mode is ON: Show the icon to go back (Up), hide the start icon (Down)
      up.classList.remove("d-none");
      down.classList.add("d-none");
    } else {
      // Recursive Mode is OFF: Show the Down icon, hide the Up icon
      up.classList.add("d-none");
      down.classList.remove("d-none");
    }
  }

  // 3. DATA FETCHING LOGIC

  // CASE A: User turned Recursion OFF (Going back to normal)
  if (!isRecursiveView) {
    loadStandardTable(); // Fetch /show
    return;
  }

  // CASE B: User turned Recursion ON
  try {
    const response = await fetch(
      `${API_URL}/recursivereverse?table_id=${TABLE_ID}`,
      {
        method: "GET",
      }
    );

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    renderTable(data); // Render using the countdown S.NO logic
  } catch (error) {
    console.error("Error: ", error);
    alert("Something went wrong with the recursive fetch!");

    // Fallback: If error, reset flag and view
    isRecursiveView = false;
    loadStandardTable();
  }
}

// --- 9. CSV upload ---
function uploadCsv() {
  document.getElementById("hiddenFileInput").click();
}

async function send_file_to_backend() {
  const fileInput = document.getElementById("hiddenFileInput");
  const file = fileInput.files[0];

  if (!file) {
    return;
  }
  if (file.type !== "text/csv" && !file.name.endsWith(".csv")) {
    alert("Please upload a valid .csv file");
    return;
  }

  const reader = new FileReader();

  reader.onload = async function (e) {
    const csvContent = e.target.result;
    try {
      const response = await fetch(
        `${API_URL}/upload_csv?table_id=${TABLE_ID}`,
        {
          method: "POST",
          headers: { "Content-Type": "text/plain" },
          body: csvContent,
        }
      );

      const result = await response.json();
      if (response.ok) {
        alert(
          `Upload successfull!\nAdded: ${result.added}\nSkipped: ${result.skipped}`
        );
      } else {
        alert("Upload failed!");
      }
    } catch (error) {
      console.error("Error: ", error);
      alert("Something went wrong!");
    }
    if (isRecursiveView) {
      recursiveReverse();
    } else {
      loadStandardTable();
    }
    fileInput.value = "";
  };
  reader.readAsText(file);
}

// --- 10. Table download ---
function downloadTable() {
  // Redirect to the backend export route
  window.location.href = `${API_URL}/download_table?table_id=${TABLE_ID}`;
}

// --- 11. Delete Table ---
async function deleteData() {
  if (!confirm("Are you sure you want to wipe out entire table?")) return;
  try {
    const response = await fetch(
      `${API_URL}/clear_table?table_id=${TABLE_ID}`,
      {
        method: "DELETE",
      }
    );
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    loadStandardTable();
  } catch (error) {
    console.error("Error: ", error);
    alert("Something went wrong with the recursive fetch!");
  }
}

// ==========================================
// KEYBOARD SHORTCUTS (Enter & Escape)
// ==========================================
document.addEventListener("keydown", function (event) {
  // --- HANDLE "ENTER" KEY ---
  if (event.key === "Enter") {
    // 1. Search Logic (Table View)
    // Only search if the user is currently typing in the Search Box
    const searchInput = document.getElementById("searchId");
    if (searchInput && document.activeElement === searchInput) {
      event.preventDefault(); // Stop default form submit behavior
      searchData();
      return;
    }

    // 2. Add Logic (Add Page)
    // Tries to find the button. Ensure your HTML button has id="addBtn"
    const addBtn = document.getElementById("addBtn");
    if (addBtn) {
      event.preventDefault();
      addBtn.click(); // Trigger the click
      return;
    }

    // 3. Update Logic (Update Page)
    const updateBtn = document.getElementById("updateBtn");
    if (updateBtn) {
      event.preventDefault();
      updateBtn.click(); // Trigger the click
      return;
    }
  }

  // --- HANDLE "ESCAPE" KEY ---
  if (event.key === "Escape") {
    // 1. Close Button (Add Page)
    const closeBtn = document.getElementById("closeBtn");
    if (closeBtn) {
      closeBtn.click(); // Triggers the link navigation
      return;
    }

    // 2. Close Button (Update Page)
    const closeUpdateBtn = document.getElementById("closeUpdateBtn");
    if (closeUpdateBtn) {
      closeUpdateBtn.click(); // Triggers the link navigation
      return;
    }
  }
});
