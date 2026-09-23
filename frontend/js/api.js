
const API_URL = "http://127.0.0.1:8080";

async function apiGet(route){
    const res = await fetch(`${API_URL}${route}`);
    return await res.json();
}

async function apiPost(route,data){
    const res = await fetch(`${API_URL}${route}`,{
        method:"POST",
        headers:{
            "Content-Type":"application/json"
        },
        body:JSON.stringify(data)
    });

    return await res.json();
}