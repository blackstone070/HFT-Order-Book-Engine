import socket
import struct
import matplotlib.pyplot as plt
import errno

# 1. Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 12345
order_book = {} 

# 2. Create Socket with Fixes
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False) 

# 3. Setup Graph Window
plt.ion()
fig, ax = plt.subplots(figsize=(10, 6))

print("--- HFT Dashboard Live (Waiting for C++ Simulation...) ---")

try:
    while True:
        try:
            # 4. Receive data (Non-blocking)
            data, addr = sock.recvfrom(16)
            b_id, s_id, price, qty = struct.unpack('IIII', data)
            
            # 5. Update our internal Order Book
            # We add volume at the specific price level
            order_book[price] = order_book.get(price, 0) + qty
            
            # 6. Redraw the Depth Chart
            ax.clear()
            prices = sorted(order_book.keys())
            volumes = [order_book[p] for p in prices]
            
            # Bids (Green) vs Asks (Red)
            # Assuming 105 is the 'mid' price where colors split
            colors = ['green' if p < 105 else 'red' for p in prices]
            
            ax.bar(prices, volumes, color=colors)
            
            # Force the X-Axis view to see the full market range
            ax.set_xlim(85, 125) 
            ax.set_ylim(0, 100) # Since we removed 10k, let's zoom the volume axis
            
            ax.set_title(f"Market Depth - Last Trade: ${price} | Qty: {qty}")
            ax.set_xlabel("Price ($)")
            ax.set_ylabel("Total Resting Volume")
            
            plt.pause(0.001)

        except socket.error as e:
            # If no data is coming, just keep the window responsive
            if e.errno == errno.EWOULDBLOCK or e.errno == 10035:
                plt.pause(0.1)
            else:
                raise e

except KeyboardInterrupt:
    print("Dashboard Stopped.")
finally:
    sock.close()
