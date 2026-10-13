-- =====================================================================
-- FaarisMart Database Schema and Safe Initial Seed
-- Safe to execute repeatedly (Idempotent: CREATE TABLE IF NOT EXISTS / ON CONFLICT)
-- =====================================================================

-- 1. Users Table
CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(20) DEFAULT 'buyer',
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Backward compatibility: add generated 'id' column if missing
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.columns 
        WHERE table_name = 'users' AND column_name = 'id'
    ) THEN
        ALTER TABLE users ADD COLUMN id INT GENERATED ALWAYS AS (user_id) STORED;
    END IF;
END $$;

-- 2. Products Table
CREATE TABLE IF NOT EXISTS products (
    product_id SERIAL PRIMARY KEY,
    product_name VARCHAR(255) NOT NULL,
    price NUMERIC(10, 2) NOT NULL,
    image VARCHAR(255) DEFAULT 'placeholder.png',
    description TEXT,
    category VARCHAR(100),
    stock INT DEFAULT 0,
    rating NUMERIC(2, 1) DEFAULT 4.5,
    seller_username VARCHAR(50) DEFAULT 'admin',
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- 3. Orders Table
CREATE TABLE IF NOT EXISTS orders (
    order_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(user_id) ON DELETE SET NULL,
    customer_name VARCHAR(100),
    username VARCHAR(50),
    phone VARCHAR(30),
    address TEXT,
    payment_method VARCHAR(50),
    total NUMERIC(10, 2) DEFAULT 0,
    product_id INT REFERENCES products(product_id) ON DELETE SET NULL,
    quantity INT DEFAULT 1,
    price NUMERIC(10, 2) DEFAULT 0,
    order_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- 4. Cart Table
CREATE TABLE IF NOT EXISTS cart (
    cart_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    product_id INT NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    quantity INT NOT NULL DEFAULT 1,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(username, product_id)
);

-- 5. Reviews Table
CREATE TABLE IF NOT EXISTS reviews (
    review_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    product_id INT NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    rating INT CHECK (rating >= 1 AND rating <= 5),
    review_text TEXT,
    review_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_products_category ON products(category);
CREATE INDEX IF NOT EXISTS idx_orders_user_id ON orders(user_id);
CREATE INDEX IF NOT EXISTS idx_orders_order_date ON orders(order_date DESC);
CREATE INDEX IF NOT EXISTS idx_cart_username ON cart(username);
CREATE INDEX IF NOT EXISTS idx_reviews_product_id ON reviews(product_id);

-- Initial seed products (Safe: only inserts if product_id does not exist)
INSERT INTO products (product_id, product_name, price, image, description, category, stock, rating, seller_username)
VALUES
(1, 'ASUS TUF Gaming F15', 52990.00, 'asusutuf15.jpg', 'Intel Core i5 11th Gen, 16GB RAM, 512GB SSD, RTX 2050 Graphics, 144Hz FHD Display', 'Electronics', 15, 4.6, 'admin'),
(2, 'HP Victus Gaming Laptop', 62990.00, 'hpvictus.jpg', 'AMD Ryzen 5 5600H, 16GB RAM, 512GB SSD, AMD Radeon RX 6500M, 15.6 inch FHD', 'Electronics', 12, 4.5, 'admin'),
(3, 'Lenovo IdeaPad Slim 3', 38990.00, 'lenovo.jpg', 'Intel Core i3 12th Gen, 8GB RAM, 512GB SSD, Windows 11 Home, Thin & Light', 'Electronics', 20, 4.3, 'admin'),
(4, 'Apple iPhone 15 (128 GB)', 69900.00, 'iphone15.jpg', 'Dynamic Island, 48MP Main Camera, USB-C, A16 Bionic chip, All-Day Battery Life', 'Electronics', 25, 4.8, 'admin'),
(5, 'Samsung Galaxy S24 5G', 74999.00, 'galaxys24.jpg', 'Galaxy AI, 8GB RAM, 128GB Storage, 50MP Camera, Dynamic AMOLED 2X Display', 'Electronics', 18, 4.7, 'admin'),
(6, 'OnePlus 13 5G', 54999.00, 'oneplus13.jpg', 'Snapdragon 8 Elite, 16GB RAM, 256GB Storage, Hasselblad Camera, 100W SUPERVOOC', 'Electronics', 22, 4.7, 'admin'),
(7, 'Realme P1 Speed 5G', 17999.00, 'realmep1speed.jpg', 'Dimensity 7300 Energy 5G, 8GB RAM, 128GB ROM, 120Hz OLED, 45W Fast Charging', 'Electronics', 30, 4.4, 'admin'),
(8, 'Sony WH-CH520 Wireless Headphones', 3990.00, 'sonywhch520.jpg', 'Up to 50 Hours Battery Life, DSEE Upscaling, Multipoint Connection, Built-in Mic', 'Electronics', 40, 4.5, 'admin'),
(9, 'boAt Airdopes 141 ANC', 1499.00, 'boatearbuds.jpg', 'Active Noise Cancellation (up to 32dB), 42H Playtime, ENx Tech, ASAP Charge', 'Electronics', 50, 4.2, 'admin'),
(10, 'JBL Flip 6 Bluetooth Speaker', 9999.00, 'jblflip6.jpg', 'Powerful 2-Way Speaker System, IP67 Waterproof & Dustproof, 12 Hours Playtime', 'Electronics', 15, 4.7, 'admin'),
(11, 'Logitech G102 Lightsync Mouse', 1495.00, 'logitechg102.jpg', '8000 DPI Sensor, Customizable RGB Lighting, 6 Programmable Buttons', 'Electronics', 35, 4.6, 'admin'),
(12, 'HP Wireless Optical Mouse', 699.00, 'hpwirelessmouse..jpg', '2.4GHz Wireless Connectivity, 1600 DPI, Long Battery Life, Ergonomic Design', 'Electronics', 45, 4.3, 'admin'),
(13, 'Redgear Mechanical Keyboard', 2299.00, 'redgearkeyboard.jpg', 'Blue Mechanical Switches, Rainbow LED Backlit, Control Knob, Wrist Rest', 'Electronics', 20, 4.4, 'admin'),
(14, 'Xiaomi Smart Band 9', 2999.00, 'miband9.jpg', '1.62 inch AMOLED Display, 150+ Sports Modes, 21-Day Battery Life, Heart Rate & SpO2', 'Electronics', 28, 4.5, 'admin'),
(15, 'SanDisk Ultra 128GB MicroSD', 899.00, 'sandisk128gb.jpg', 'Up to 140MB/s Read Speed, Class 10, A1 App Performance, Full HD Video Support', 'Electronics', 60, 4.7, 'admin'),
(16, 'Nike Air Max SC Running Shoes', 5995.00, 'nikeshoes.jpg', 'Lightweight cushioning, breathable mesh and leather upper, heritage running look', 'Footwear', 14, 4.6, 'admin'),
(17, 'Adidas Ultraboost Light Shoes', 11999.00, 'adidasultraboost.jpg', 'Light BOOST midsole, Primeknit+ upper, Continental Rubber outsole for superior grip', 'Footwear', 10, 4.8, 'admin'),
(18, 'Puma Smash V2 Casual Sneakers', 2499.00, 'pumasneakers.jpg', 'Classic tennis-inspired silhouette, soft suede upper, durable rubber outsole', 'Footwear', 22, 4.3, 'admin'),
(19, 'Bata Formal Leather Shoes', 1999.00, 'batashoes.jpg', 'Genuine leather formal lace-up derby shoes, cushioned insole for all-day comfort', 'Footwear', 18, 4.2, 'admin'),
(20, 'Levis 511 Slim Fit Stretch Jeans', 2599.00, 'levisjeans.jpg', 'Slim through the thigh with a straight leg, premium stretch denim comfort', 'Fashion', 25, 4.5, 'admin'),
(21, 'Allen Solly Slim Fit Shirt', 1499.00, 'allensollyshirt.jpg', '100% Cotton, spread collar, long sleeves, versatile formal and casual wear', 'Fashion', 30, 4.4, 'admin'),
(22, 'Roadster Pure Cotton T-Shirt', 499.00, 'roadstertshirt.jpg', '100% Breathable cotton round neck casual t-shirt, relaxed regular fit', 'Fashion', 40, 4.2, 'admin'),
(23, 'Puma Fleece Pullover Hoodie', 2199.00, 'pumahoodie..jpg', 'Warm cotton-poly fleece, adjustable drawstring hood, kangaroo front pocket', 'Fashion', 16, 4.5, 'admin'),
(24, 'Fossil Leather Bifold Wallet', 2995.00, 'fossilwallet.jpg', '100% Genuine leather, RFID blocking technology, 8 card slots, 2 bill compartments', 'Accessories', 20, 4.6, 'admin'),
(25, 'Amazon Basics Laptop Sleeve', 699.00, 'amazonsleeve.jpg', 'Padded neoprene protective case, slim and compact, top zipper opening', 'Accessories', 35, 4.5, 'admin')
ON CONFLICT (product_id) DO NOTHING;

-- Reset sequence to highest product_id
SELECT setval('products_product_id_seq', COALESCE((SELECT MAX(product_id) FROM products), 1));
