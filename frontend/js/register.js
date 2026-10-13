// ==========================================
// FAARISMART REGISTER PAGE
// frontend/js/register.js
// ==========================================

const getEndpoint = () => (typeof getApiBaseUrl === "function" ? getApiBaseUrl() : (window.API_URL || "http://127.0.0.1:8080"));

// -------------------- Elements --------------------

const form = document.getElementById("registerForm");
const usernameInput = document.getElementById("username");
const emailInput = document.getElementById("email");
const passwordInput = document.getElementById("password");
const confirmPasswordInput = document.getElementById("confirmPassword");
const togglePassword = document.getElementById("togglePassword");
const registerBtn = document.getElementById("registerBtn");

// -------------------- Show/Hide Password --------------------

if (togglePassword) {
    togglePassword.addEventListener("click", () => {
        const icon = togglePassword.querySelector("i");

        if (passwordInput.type === "password") {
            passwordInput.type = "text";
            confirmPasswordInput.type = "text";
            icon.classList.replace("fa-eye", "fa-eye-slash");
        } else {
            passwordInput.type = "password";
            confirmPasswordInput.type = "password";
            icon.classList.replace("fa-eye-slash", "fa-eye");
        }
    });
}

// -------------------- Email Validation --------------------

function isValidEmail(email) {
    const pattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return pattern.test(email);
}

// -------------------- Register User --------------------

form.addEventListener("submit", async (e) => {

    e.preventDefault();

    const username = usernameInput.value.trim();
    const email = emailInput.value.trim();
    const password = passwordInput.value.trim();
    const confirmPassword = confirmPasswordInput.value.trim();

    if (!username || !email || !password || !confirmPassword) {
        alert("Please fill all fields.");
        return;
    }

    if (!isValidEmail(email)) {
        alert("Please enter a valid email address.");
        return;
    }

    if (password.length < 6) {
        alert("Password must contain at least 6 characters.");
        return;
    }

    if (password !== confirmPassword) {
        alert("Passwords do not match.");
        return;
    }

    registerBtn.disabled = true;
    registerBtn.innerHTML =
        '<i class="fa-solid fa-spinner fa-spin"></i> Creating...';

    try {
        const API_URL = getEndpoint();
        const response = await fetch(`${API_URL}/register`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                username,
                email,
                password
            })
        });

        let data = {};
        try {
            data = await response.json();
        } catch (jsonErr) {
            data = {};
        }

        if (response.ok && data.success) {

            alert("🎉 Account Created Successfully!");

            form.reset();

            window.location.href = "login.html";

        } else {

            alert(data.message || "Registration failed. Please check your details.");

        }

    } catch (error) {

        console.error(error);
        alert("Unable to reach backend API. Please check your network connection.");

    } finally {

        registerBtn.disabled = false;
        registerBtn.innerHTML = "Create Account";

    }

});