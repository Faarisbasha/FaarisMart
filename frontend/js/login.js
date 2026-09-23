// ==========================================
// FAARISMART LOGIN
// ==========================================

const API = "http://127.0.0.1:8080";

// Elements
const form = document.getElementById("loginForm");
const usernameInput = document.getElementById("username");
const passwordInput = document.getElementById("password");
const togglePassword = document.getElementById("togglePassword");
const loginBtn = document.getElementById("loginBtn");
const rememberMe = document.getElementById("rememberMe");

// ------------------------------------------
// Auto Login
// ------------------------------------------

window.addEventListener("load", () => {

    const savedUser = localStorage.getItem("faarisUser");

    if (savedUser) {
        window.location.href = "index.html";
    }

});

// ------------------------------------------
// Show / Hide Password
// ------------------------------------------

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

// ------------------------------------------
// Login
// ------------------------------------------

form.addEventListener("submit", async (e) => {

    e.preventDefault();

    const username = usernameInput.value.trim();
    const password = passwordInput.value.trim();

    if (!username || !password) {

        alert("Please enter username and password.");
        return;

    }

    loginBtn.disabled = true;
    loginBtn.innerHTML =
        '<i class="fa-solid fa-spinner fa-spin"></i> Signing In...';

    try {

        const response = await fetch(API + "/login", {

            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({
                username,
                password
            })

        });

        const data = await response.json();

        console.log("Login Response:", data);

        if (data.success) {

            if (rememberMe.checked) {

                localStorage.setItem("faarisUser", data.username);
                localStorage.setItem("userId", data.user_id);

            } else {

                sessionStorage.setItem("faarisUser", data.username);
                sessionStorage.setItem("userId", data.user_id);

            }

            window.location.href = "index.html";

        } else {

            alert(data.message || "Invalid Username or Password");

        }

    } catch (error) {

        console.error(error);

        alert("Backend not connected. Please start FaarisAPI.exe.");

    } finally {

        loginBtn.disabled = false;
        loginBtn.innerHTML = "Login";

    }

});