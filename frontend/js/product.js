const API = "http://127.0.0.1:8080";

// Get product ID from URL
const params = new URLSearchParams(window.location.search);
const productId = Number(params.get("id"));

let product = null;
let quantity = 1;

// Load product from PostgreSQL
async function loadProduct() {

    try {

        const res = await fetch(API + "/products");

        if (!res.ok) throw new Error("Failed to fetch products");

        const products = await res.json();

        product = products.find(p => Number(p.id) === productId);

        if (!product) {
            alert("Product not found.");
            window.location.href = "index.html";
            return;
        }

        // Fill page
        document.getElementById("productImage").src =
            "images/" + product.image;

        document.getElementById("productImage").alt =
            product.name;

        document.getElementById("productName").textContent =
            product.name;

        document.getElementById("productPrice").textContent =
            "₹" + Number(product.price).toLocaleString("en-IN");

        document.getElementById("productDesc").textContent =
            product.description;

        // Rating
        const ratingElement = document.querySelector(".rating");

        if (ratingElement) {
            ratingElement.innerHTML =
                `⭐ ${product.rating} / 5`;
        }

        // Stock
        const stockElement = document.getElementById("stockText");

        if (stockElement) {

            if (product.stock > 0) {

                stockElement.innerHTML =
                    `✅ In Stock (${product.stock} left)`;

                stockElement.style.color = "#16a34a";

            } else {

                stockElement.innerHTML =
                    "❌ Out of Stock";

                stockElement.style.color = "#dc2626";
            }
        }

    } catch (err) {

        console.error(err);

        alert("Backend not connected.");
    }
}

// Change quantity
function changeQty(change) {

    quantity += change;

    if (quantity < 1)
        quantity = 1;

    if (product && quantity > product.stock)
        quantity = product.stock;

    document.getElementById("qty").textContent = quantity;
}

// Add product to cart
function addToCart() {

    if (!product) return;

    let cart = JSON.parse(localStorage.getItem("cart")) || [];

    const existing = cart.find(item => item.id === product.id);

    if (existing) {

        existing.quantity += quantity;

    } else {

        cart.push({
            ...product,
            quantity
        });
    }

    localStorage.setItem("cart", JSON.stringify(cart));

    alert(`${quantity} × ${product.name} added to cart!`);
}

// Buy now
function buyNow() {

    addToCart();

    window.location.href = "checkout.html";
}

// Start
loadProduct();