import asyncio
import time

import numpy as np

import kaerman_filter as kf


async def send_data(reader, writer, time1, user_name):
    with open("up_data1.txt", "r") as f:
        data = f.readlines()
    numbers = []
    for line in data:
        nums = line.strip().split()
        numbers.append(nums)
    arr = np.array(numbers)
    arr = arr.astype(float)
    arr = arr.transpose()
    data = []
    kll = 0
    for arr_row in arr:
        data22 = kf.KF(arr_row, 2488, 0.01, 100)
        data.append(data22)
        kll = kll + 1
    str1 = np.hstack(data)
    str1 = str1.astype(int)
    data = str1.tolist()
    start = f"start {user_name}\n"
    writer.write(start.encode())
    await writer.drain()
    time_sta=time.time()
    print("开始传输数据 {}".format(user_name))
    numbers = []
    for item in data:
        # item = [user_name,item]
        item = [i for sublist in item for i in (sublist if isinstance(sublist, list) else [sublist])]
        item = "{}  {}  {}  {}  {}  {}".format(item[0], item[1], item[2], item[3], item[4], item[5])
        item = '{} {} \n'.format(user_name, item)
        #print(item)
        writer.write(item.encode())
        await writer.drain()
        await asyncio.sleep(time1)  # 采用异步休眠
        data_get = await reader.read(500)
        data_get = data_get.decode().strip()
        #print(data_get)
        if data_get == "十五阶提醒":
            print(data_get)
    end = f"end {user_name}"
    writer.write(end.encode())
    await writer.drain()
    time_end=time.time()
    delta_time=time_end-time_sta
    print("数据传输完成 {}  用时：{}秒".format(user_name,delta_time))


async def main(time1, user_name):
    try:
        reader, writer = await asyncio.open_connection('127.0.0.1', 65430)
        await send_data(reader, writer, time1, user_name)
    except KeyboardInterrupt:
        print("客户端已关闭连接")
    finally:
        writer.close()
        await writer.wait_closed()


if __name__ == "__main__":
    time1 = 0.03
    user_name = "kol"
    asyncio.run(main(time1=time1, user_name=user_name))
