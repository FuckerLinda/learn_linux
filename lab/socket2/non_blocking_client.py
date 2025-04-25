# non_blocking_client.py
import socket
import select
import sys

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.setblocking(False)

try:
    client.connect(("localhost", 12345))
except BlockingIOError:
    pass  # 连接进行中

# 使用select等待连接完成
timeout = 5  # 超时时间
while True:
    _, writable, _ = select.select([], [client], [], timeout)
    if not writable:
        print("连接超时")
        sys.exit(1)
    # 检查是否连接成功
    err = client.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
    if err != 0:
        print(f"连接失败: {errno.errorcode[err]}")
        sys.exit(1)
    break

client.send(b"Hello from client!")
data = client.recv(1024)
print(f"收到回复: {data.decode()}")
client.close()
