const form = document.getElementById("registerForm");

const password = document.getElementById("password");
const confirmPassword = document.getElementById("confirmPassword");

const toggle = document.getElementById("togglePassword");

toggle.onclick = () => {

    const icon = toggle.querySelector("i");

    if(password.type === "password"){

        password.type = "text";
        icon.classList.replace("fa-eye","fa-eye-slash");

    }else{

        password.type = "password";
        icon.classList.replace("fa-eye-slash","fa-eye");

    }

};

form.addEventListener("submit", async (e)=>{

    e.preventDefault();

    const username = document.getElementById("username").value.trim();
    const email = document.getElementById("email").value.trim();
    const passwordValue = password.value.trim();

    if(passwordValue !== confirmPassword.value.trim()){

        alert("Passwords do not match.");
        return;

    }

    try{

        const res = await fetch("http://127.0.0.1:8080/register",{

            method:"POST",

            headers:{
                "Content-Type":"application/json"
            },

            body:JSON.stringify({
                username,
                email,
                password:passwordValue
            })

        });

        const data = await res.json();

        if(data.success){

            alert("Account Created Successfully!");
            window.location.href="login.html";

        }else{

            alert(data.message);

        }

    }catch{

        alert("Backend not connected.");

    }

});