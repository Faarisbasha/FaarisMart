// ==========================================
// FAARISMART REGISTER PAGE
// frontend/js/register.js
// ==========================================

const API_URL = "http://127.0.0.1:8080";

// -------------------- Elements --------------------

const form = document.getElementById("registerForm");
const usernameInput = document.getElementById("username");
const emailInput = document.getElementById("email");
const passwordInput = document.getElementById("password");
const confirmPasswordInput = document.getElementById("confirmPassword");
const togglePassword = document.getElementById("togglePassword");
const registerBtn = document.getElementById("registerBtn");

// -------------------- Show/Hide Password --------------------

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

        const data = await response.json();

        if (data.success) {

            alert("🎉 Account Created Successfully!");

            form.reset();

            window.location.href = "login.html";

        } else {

            alert(data.message || "Registration failed.");

        }

    } catch (error) {

        console.error(error);
        alert("Backend not connected. Please start FaarisAPI.exe.");

    } finally {

        registerBtn.disabled = false;
        registerBtn.innerHTML = "Create Account";

    }

});