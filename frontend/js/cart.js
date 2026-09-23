
// ==========================
// FAARISMART CART
// ==========================

let cart = JSON.parse(localStorage.getItem("cart")) || [];

cart = cart.map(item => ({
    ...item,
    quantity: item.quantity || 1
}));

function saveCart() {
    localStorage.setItem("cart", JSON.stringify(cart));
}

function renderCart() {

    const container = document.getElementById("cartContainer");

    if (!container) return;

    container.innerHTML = "";

    if (cart.length === 0) {

        container.innerHTML = `
        <div class="empty">
            <i class="fa-solid fa-cart-shopping"></i>
            <h2>Your Cart is Empty</h2>
            <p>Add some products to continue shopping.</p>
            <button onclick="window.location.href='index.html'">
                Continue Shopping
            </button>
        </div>`;

        return;
    }

    let total = 0;

    cart.forEach((item, index) => {

        total += Number(item.price) * item.quantity;

        container.innerHTML += `
        <div class="cart-item">

            <img src="images/${item.image}" alt="${item.name}">

            <div class="info">

                <h3>${item.name}</h3>

                <p class="price">
                    ₹${Number(item.price).toLocaleString("en-IN")}
                </p>

                <div class="qty">

                    <button onclick="changeQty(${index},-1)">−</button>

                    <span>${item.quantity}</span>

                    <button onclick="changeQty(${index},1)">+</button>

                </div>

            </div>

            <button class="remove" onclick="removeItem(${index})">

                <i class="fa-solid fa-trash"></i> Remove

            </button>

        </div>`;
    });

    container.innerHTML += `
    <div class="summary">

        <h2>Order Summary</h2>

        <div class="total">

            <span>Total</span>

            <span>₹${total.toLocaleString("en-IN")}</span>

        </div>

        <div class="actions">

            <button class="continue"
            onclick="window.location.href='index.html'">

                Continue Shopping

            </button>

            <button class="checkout" onclick="checkout()">

                Checkout

            </button>

        </div>

    </div>`;
}

function changeQty(index, change) {

    cart[index].quantity += change;

    if (cart[index].quantity <= 0) {
        cart.splice(index, 1);
    }

    saveCart();
    renderCart();
}

function removeItem(index) {

    cart.splice(index, 1);

    saveCart();

    renderCart();
}

function checkout() {

    if (cart.length === 0) {

        alert("Your cart is empty.");

        return;
    }

    window.location.href = "checkout.html";
}

renderCart();