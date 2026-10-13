/* ==========================================
   FAARISMART LOGIN
========================================== */

// Use centralized API endpoint
const getEndpoint = () => (typeof getApiBaseUrl === "function" ? getApiBaseUrl() : (window.API || "http://127.0.0.1:8080"));

// Elements
const form = document.getElementById("loginForm");
const usernameInput = document.getElementById("username");
const passwordInput = document.getElementById("password");
const togglePassword = document.getElementById("togglePassword");
const loginBtn = document.getElementById("loginBtn");
const rememberMe = document.getElementById("rememberMe");

// ==========================================
// AUTO LOGIN
// ==========================================

window.addEventListener("load", () => {
    const savedUser =
        localStorage.getItem("faarisUser") ||
        sessionStorage.getItem("faarisUser");

    if (savedUser) {
        window.location.replace("index.html");
    }
});

// ==========================================
// SHOW / HIDE PASSWORD
// ==========================================

if (togglePassword) {
    togglePassword.addEventListener("click", () => {
        const icon = togglePassword.querySelector("i");

        if (passwordInput.type === "password") {
            passwordInput.type = "text";
            icon.classList.replace("fa-eye", "fa-eye-slash");
        } else {
            passwordInput.type = "password";
            icon.classList.replace("fa-eye-slash", "fa-eye");
        }
    });
}

// ==========================================
// LOGIN
// ==========================================

form.addEventListener("submit", async (e) => {
    e.preventDefault();

    const username = usernameInput.value.trim();
    const password = passwordInput.value;

    if (!username || !password) {
        alert("Please enter username and password.");
        return;
    }

    loginBtn.disabled = true;
    loginBtn.innerHTML =
        '<i class="fa-solid fa-spinner fa-spin"></i> Signing In...';

    try {
        const API = getEndpoint();
        const response = await fetch(`${API}/login`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                username: username,
                password: password
            })
        });

        let data = {};
        try {
            data = await response.json();
        } catch (jsonErr) {
            data = {};
        }

        console.log("Login Response:", data);

        if (response.ok && data.success === true) {
            // Clear any previous login session
            localStorage.removeItem("faarisUser");
            localStorage.removeItem("userId");
            sessionStorage.removeItem("faarisUser");
            sessionStorage.removeItem("userId");

            // Save login based on Remember Me
            const storage = rememberMe.checked
                ? localStorage
                : sessionStorage;

            storage.setItem("faarisUser", data.username || username);

            if (data.user_id !== undefined && data.user_id !== null) {
                storage.setItem("userId", data.user_id);
            }

            // Go to homepage after successful login
            window.location.href = "index.html";
        } else {
            // Display user-friendly error message from server
            alert(data.message || "Invalid username or password.");
        }

    } catch (error) {
        console.error("Login Error:", error);

        alert(
            "Unable to reach the backend API. Please verify the server is running and check your internet connection."
        );

    } finally {
        loginBtn.disabled = false;
        loginBtn.innerHTML = "Login";
    }
});