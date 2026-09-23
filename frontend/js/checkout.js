const API = "http://127.0.0.1:8080";

let cart = JSON.parse(localStorage.getItem("cart")) || [];

const GST_RATE = 0.18;
const DELIVERY = 49;

function renderCheckout() {

    const orderBox = document.getElementById("orderSummary");

    if (!orderBox) return;

    if (cart.length === 0) {
        alert("Your cart is empty.");
        window.location.href = "cart.html";
        return;
    }

    let subtotal = 0;
    orderBox.innerHTML = "";

    cart.forEach(item => {

        const itemTotal = Number(item.price) * Number(item.quantity);

        subtotal += itemTotal;

        orderBox.innerHTML += `
        <div class="order-item">
            <span>${item.name} × ${item.quantity}</span>
            <span>₹${itemTotal.toLocaleString("en-IN")}</span>
        </div>`;
    });

    const gst = Math.round(subtotal * GST_RATE);
    const grandTotal = subtotal + gst + DELIVERY;

    orderBox.innerHTML += `
    <hr>

    <div class="order-item">
        <strong>Subtotal</strong>
        <strong>₹${subtotal.toLocaleString("en-IN")}</strong>
    </div>

    <div class="order-item">
        <span>GST (18%)</span>
        <span>₹${gst.toLocaleString("en-IN")}</span>
    </div>

    <div class="order-item">
        <span>Delivery</span>
        <span>₹${DELIVERY}</span>
    </div>

    <hr>

    <div class="order-item total">
        <strong>Total</strong>
        <strong>₹${grandTotal.toLocaleString("en-IN")}</strong>
    </div>`;
}

async function placeOrder() {

    const name = document.getElementById("name").value.trim();
    const phone = document.getElementById("phone").value.trim();
    const address = document.getElementById("address").value.trim();
    const payment = document.querySelector("input[name='payment']:checked");

    if (!name || !phone || !address) {
        alert("Please fill all delivery details.");
        return;
    }

    if (!payment) {
        alert("Please select a payment method.");
        return;
    }

    let subtotal = 0;
    cart.forEach(item => subtotal += Number(item.price) * Number(item.quantity));

    const gst = Math.round(subtotal * GST_RATE);
    const grandTotal = subtotal + gst + DELIVERY;

    const orderData = {
        user_id: localStorage.getItem("userId") || 1,
        customer_name: name,
        phone: phone,
        address: address,
        payment_method: payment.value,
        total: grandTotal
    };

    try {

        const res = await fetch(API + "/orders", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(orderData)
        });

        const data = await res.json();

        if (data.success) {

            localStorage.removeItem("cart");

            alert("Order placed successfully! Order ID: " + data.order_id);

            window.location.href = "orders.html";

        } else {

            alert(data.message || "Order failed.");

        }

    } catch (err) {

        console.error(err);

        alert("Backend not connected.");

    }

}

renderCheckout();