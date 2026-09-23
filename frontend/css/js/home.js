const form=document.getElementById("loginForm");

const password=document.getElementById("password");

const toggle=document.getElementById("togglePassword");

toggle.onclick=()=>{

const icon=toggle.querySelector("i");

if(password.type==="password"){

password.type="text";

icon.classList.replace("fa-eye","fa-eye-slash");

}else{

password.type="password";

icon.classList.replace("fa-eye-slash","fa-eye");

}

};

form.addEventListener("submit",async(e)=>{

e.preventDefault();

const username=document.getElementById("username").value.trim();

const passwordValue=password.value.trim();

try{

const res=await fetch("http://127.0.0.1:8080/login",{

method:"POST",

headers:{

"Content-Type":"application/json"

},

body:JSON.stringify({

username,
password:passwordValue

})

});

const data=await res.json();

if(data.success){

localStorage.setItem("userId",data.user_id);

localStorage.setItem("username",data.username);

window.location.href="index.html";

}else{

alert(data.message);

}

}catch{

alert("Backend not connected.");

}

});