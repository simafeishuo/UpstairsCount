import asyncio
import database
import suanfa
from collections import defaultdict

# 全局参数
TIMEOUT = 5  # 超时时间
WINDOW_SIZE = 25  # 数据窗口大小
BATCH_SIZE = 5  # 数据批量大小
JING_NUMBER = 15  # 提醒阈值

# 用户数据存储，按用户独立管理
users_data = defaultdict(lambda: {"data": [], "stage": 0, "batch": []})


async def handle_client(reader, writer):
    """
    处理客户端连接的协程函数。
    """
    addr = writer.get_extra_info('peername')
    print(f"新连接：{addr}")
    ddd = 0
    try:
        while True:
            # 接收数据
            data = await asyncio.wait_for(reader.readline(), timeout=TIMEOUT)
            if not data:
                print(f"{addr} 断开连接")
                break

            data = data.decode().strip().split()
            if data[0] == 'start':
                print(data)
                username = data[1]
                database.reset_user_table(username)  # 删除并重建用户表
                users_data[username]["stage"] = 0  # 初始化阶段值
                writer.write(f"{username} 表已重置并重新创建\n".encode())
                await writer.drain()

            elif data[0] == 'end':
                print(data)
                username = data[1]
                await process_user_data(username)
                writer.write(f"{username} 传输完成\n".encode())
                await writer.drain()
                del users_data[username]
                break

            else:
                await process_data_chunk(data)
                if ddd > 10:
                    # print(data)
                    ddd = 0
                ddd = ddd + 1

    except asyncio.TimeoutError:
        print(f"{addr} 超时")
    except Exception as e:
        print(f"发生错误：{e}")
    finally:
        writer.close()


async def process_data_chunk(data_chunk):
    """
    处理从客户端接收到的一组数据。
    """
    for i in range(0, len(data_chunk), 9):
        username = data_chunk[i]
        data = list(map(int, data_chunk[i + 1:i + 9]))

        user = users_data[username]
        user["data"].append(data)

        if len(user["data"]) >= WINDOW_SIZE:
            # 提取窗口数据，计算阶段更新
            window_data = user["data"][-WINDOW_SIZE:]
            window_data1 = [row[:6] for row in window_data]
            # print(window_data1)
            new_stage = suanfa.main(window_data1)
            #print(window_data1)
            user["stage"] += new_stage

            # 添加数据到批量队列，包括阶段值
            user["batch"].append(data + [user["stage"]])

            # 清除已处理的窗口数据
            user["data"] = user["data"][WINDOW_SIZE:]

        # 检查是否需要批量写入
        if len(user["batch"]) >= BATCH_SIZE:
            await process_user_data(username)


async def process_user_data(username):
    """
    批量处理用户数据并写入数据库。
    """
    user = users_data[username]
    if user["batch"]:
        # 执行批量写入
        table_name = f"DATA_{username}"
        #print(user["batch"])
        database.into_database_batch(user["batch"], table_name)
        user["batch"].clear()  # 清空批量数据

        # 提醒逻辑
        reminder = user["stage"] % JING_NUMBER
        #if reminder == 0:
            #  print(f"提醒：用户 {username} 达到 {JING_NUMBER} 阶")

    # 打印当前步数和楼层数
   # print(f"用户 {username} 当前步数: {user['step']}, 楼层数: {user['floor']}")


async def main():
    """
    主函数，启动服务器。
    """
    server = await asyncio.start_server(handle_client, '0.0.0.0', 8082)
    addr = server.sockets[0].getsockname()
    print(f"服务器启动，监听 {addr}")

    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
