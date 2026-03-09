import socket
import struct
import matplotlib.pyplot as plt
import errno

# 1. Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 12346
order_book = {}    # Price -> Current Volume
price_sides = {}   # Price -> Color ('green' or 'red')

# 2. Create Socket with Fixes
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False) 

# 3. Setup Graph Window
plt.ion()
fig, ax = plt.subplots(figsize=(10, 6))

print("--- HFT LIVE DEPTH DASHBOARD (Version 2.0) ---")

try:
    while True:
        try:
            # 4. Receive 16-byte TradePacket
            data, addr = sock.recvfrom(16)
            b_id, s_id, price, qty = struct.unpack('IIII', data)
            
            # 5. SMART LOGIC: Handle Trades vs. Resting Orders
            if s_id == 0: 
                # SIGNAL 0: A match happened. REDUCE volume in the book.
                order_book[price] = max(0, order_book.get(price, 0) - qty)
            elif s_id == 1:
                # SIGNAL 1: New BUY order. ADD volume and set Green.
                order_book[price] = order_book.get(price, 0) + qty
                price_sides[price] = 'green'
            elif s_id == 2:
                # SIGNAL 2: New SELL order. ADD volume and set Red.
                order_book[price] = order_book.get(price, 0) + qty
                price_sides[price] = 'red'
            
            # 6. REDRAW: Only if volume exists at this price level
            ax.clear()
            
            # Filter out price levels that have been fully matched (volume = 0)
            active_prices = sorted([p for p in order_book if order_book[p] > 0])
            volumes = [order_book[p] for p in active_prices]
            colors = [price_sides.get(p, 'blue') for p in active_prices]
            
            if active_prices:
                ax.bar(active_prices, volumes, color=colors)
                ax.set_xlim(min(active_prices) - 10, max(active_prices) + 10) 
                ax.set_ylim(0, max(volumes) * 1.2) 
            
            ax.set_title(f"Live Market Depth | Last Event @ ${price}")
            ax.set_xlabel("Price ($)")
            ax.set_ylabel("Total Resting Volume")
            
            plt.pause(0.001)

        except socket.error as e:
            if e.errno == errno.EWOULDBLOCK or e.errno == 10035:
                plt.pause(0.1)
            else:
                raise e

except KeyboardInterrupt:
    print("Dashboard Stopped.")
finally:
    sock.close()
