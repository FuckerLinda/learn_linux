# non_blocking_server.py
import socket
import time

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setblocking(False)  # 设置为非阻塞
server.bind(("localhost", 12345))
server.listen(5)

print("非阻塞服务器启动，等待连接...")

while True:
    try:
        client, addr = server.accept()
        print(f"收到来自 {addr} 的连接")
        client.send(b"Hello from non-blocking server!")
        client.close()
    except BlockingIOError:
        print("暂无新连接，处理其他任务...")
        time.sleep(2)  # 避免CPU占用过高
