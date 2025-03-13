import socket
import csv
import os

# 配置服务器参数
HOST = '0.0.0.0'  # 监听所有网络接口
PORT = 8083   # 监听端口

# 创建一个 CSV 文件
csv_file = "received_data.csv"
header = ["Name", "Field1", "Field2", "Field3", "Field4", "Field5", "Field6"]

# 初始化 CSV 文件，写入头部
if not os.path.exists(csv_file):
    with open(csv_file, mode='w', newline='', buffering=1) as file:
        writer = csv.writer(file)
        writer.writerow(header)

# 创建 TCP/IP 套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)  # 禁用 TCP 缓存
server_socket.bind((HOST, PORT))
server_socket.listen(5)

print(f"Server listening on {HOST}:{PORT}")

try:
    while True:
        # 等待客户端连接
        client_socket, client_address = server_socket.accept()
        print(f"Connection established with {client_address}")

        with client_socket, open(csv_file, mode='a', newline='', buffering=1) as file:
            writer = csv.writer(file)

            while True:
                # 接收数据
                data = client_socket.recv(1024).decode('utf-8').strip()
                if not data:
                    break
                # print(data)
                # 检查特殊命令 "+++"
                if data[0] == '+++':
                    print("Received shutdown command '+++'")
                    break

                # 分割多条数据
                lines = data.split("\n")
                for line in lines:
                    line = line.strip()
                    if not line:
                        continue

                    print(f"Processing line: {line}")
                    if line[0] == "+++":
                        break
                    # 数据解析
                    fields = line.split()
                    if len(fields) == 7:  # 确保数据格式正确
                        writer.writerow(fields)
                    else:
                        print(f"Invalid data format: {line}")

        print(f"Connection with {client_address} closed.")

except KeyboardInterrupt:
    print("Server shutting down...")
finally:
    server_socket.close()
