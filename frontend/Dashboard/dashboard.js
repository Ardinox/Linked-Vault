const API_URL = "http://localhost:8000";

// --- GLOBAL STATE ---
let isRecursiveView = false;

// --- GLOBAL EVENT LISTENER (The Router) ---
document.addEventListener("DOMContentLoaded", () => {
  // 1. If we are on Home Page (Table exists)
  if (document.getElementById("tableBody")) {
    loadStandardTable();
  }

  // 2. If we are on Add Page
  const addBtn = document.getElementById("addBtn");
  if (addBtn) {
    addBtn.addEventListener("click", addData);
  }

  // 3. If we are on Update Page
  const updateBtn = document.getElementById("updateBtn");
  if (updateBtn) {
    const urlParams = new URLSearchParams(window.location.search);
    const idToUpdate = urlParams.get("id");

    if (idToUpdate) {
      loadEmployeeForUpdate(idToUpdate);
    } else {
      alert("No ID provided!");
      window.location.href = "table_view.html";
    }

    updateBtn.addEventListener("click", performUpdate);
  }
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

  if (!data || data.length === 0) {
    tbody.innerHTML =
      "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }
  if (Array.isArray(data) && data.length === 0) {
    tbody.innerHTML = "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
    return;
  }
  if (!Array.isArray(data) && data.id === undefined) {
    tbody.innerHTML = "<tr><td colspan='7' class='text-center'>No Data Found</td></tr>";
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
              <a class="btn btn-dark btn-sm" href="update_emp.html?id=${emp.id}&pos=${index}">Update</a>
            </td>
          </tr>`;
    tbody.innerHTML += row;
  });
}

// --- 1. ADD DATA ---
async function addData() {
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

  const payload = {
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
  }
}

// --- 2. LOAD STANDARD TABLE (Fetch /show) ---
async function loadStandardTable() {
  try {
    const response = await fetch(`${API_URL}/show`);
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
    const response = await fetch(`${API_URL}/delete`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id: id }),
    });

    if (response.ok) {
      // Refresh the table based on current view
      if (isRecursiveView) recurssiveReverse();
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
    const response = await fetch(`${API_URL}/search`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id: parseInt(id) }),
    });

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
async function performUpdate() {
  const originalId = document.getElementById("originalId").value;
  const newPos = document.getElementById("pos").value;
  const newId = document.getElementById("id").value;
  const newName = document.getElementById("name").value;
  const newAge = document.getElementById("age").value;
  const newDept = document.getElementById("department").value;
  const newSalary = document.getElementById("salary").value;

  if (!newId || !newName) {
    showStatus("ID and Name required.", "error");
    return;
  }

  if (!confirm("Overwrite this employee record?")) return;

  try {
    // Step 1: Delete Old
    const delRes = await fetch(`${API_URL}/delete`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id: parseInt(originalId) }),
    });

    if (!delRes.ok) {
      const err = await delRes.json();
      showStatus("Update Failed (Delete Step): " + err.message, "error");
      return;
    }

    // Step 2: Insert New
    const payload = {
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
      alert("Update Successful!");
      window.location.href = "table_view.html";
    } else {
      const res = await insRes.json();
      showStatus("Update Failed (Insert Step): " + res.message, "error");
    }
  } catch (error) {
    console.error("Update Error:", error);
    showStatus("Network Error.", "error");
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
    const response = await fetch(`${API_URL}/search`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id: parseInt(idValue) }),
    });

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
    const response = await fetch(`${API_URL}/linkedreverse`, {
      method: "POST", // Typically POST for actions that change data
    });

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    loadStandardTable();
  } catch (error) {
    console.error("Error: ", error);
    alert("Something went wrong!");
  }
}

// --- 8. RECURSIVE REVERSE (Visual Reverse) ---
async function recurssiveReverse() {
  // 1. Toggle the Global Flag
  isRecursiveView = !isRecursiveView;

  // 2. Handle Icons (Toggle Visibility)
  const up = document.getElementById("arrow_up");
  const down = document.getElementById("arrow_down");
  
  // Make sure elements exist before trying to toggle classes
  if (up && down) {
    if (isRecursiveView) {
      // Recursive Mode is ON: Show the icon to go back (Up), hide the start icon (Down)
      up.classList.remove('d-none'); 
      down.classList.add('d-none');
    } else {
      // Recursive Mode is OFF: Show the Down icon, hide the Up icon
      up.classList.add('d-none');
      down.classList.remove('d-none');
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
    const response = await fetch(`${API_URL}/recursivereverse`); 
    
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