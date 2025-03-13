import pymysql
import pandas as pd
import os


# 获取数据库连接
def get_db_connection():
    return pymysql.connect(
        host="localhost",
        user="root",
        password="mine12345",
        database="upstair",
        charset="utf8"
    )


# 批量插入数据
def into_database_batch(data_list, table_name):
    """
    批量插入数据到指定表中
    :param data_list: 待插入的数据列表，每条记录是一个元组
    :param table_name: 数据表名
    """
    if not data_list:
        print(f"批量写入数据为空，跳过。目标表：{table_name}")
        return

    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            sql = f"""
                INSERT INTO {table_name} (WX, WY, WZ, AX, AY, AZ, MCU_STAGE,MCU_FLOOR,STAGE)
                VALUES (%s, %s, %s, %s, %s, %s, %s,%s,%s)
            """
            cursor.executemany(sql, data_list)
        connection.commit()
        #print(f"成功写入 {len(data_list)} 条数据到表 {table_name}")
    except Exception as e:
        print(f"写入数据库失败：{e}")
    finally:
        connection.close()


# 单条数据插入
def into_database(item, username, stage):
    """
    插入单条传感器数据到用户表。
    :param item: 包含传感器数据的列表 [WX, WY, WZ, AX, AY, AZ]。
    :param username: 用户名，决定目标表。
    :param stage: 当前阶段值。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            sql = f"""
                INSERT INTO DATA_{username} (WX, WY, WZ, AX, AY, AZ, MCU_STAGE , MCU_FLOOR, STAGE)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s , %s)
            """
            print(sql, (item[0], item[1], item[2], item[3], item[4], item[5], item[6], item[7], stage))
            cursor.execute(sql, (item[0], item[1], item[2], item[3], item[4], item[5], item[6], item[7], stage))
        connection.commit()
        print(f"成功写入单条数据到表 DATA_{username}")
    except Exception as e:
        print(f"插入数据失败：{e}")
    finally:
        connection.close()


# 用户注册
def into_user(user_data):
    """
    将注册信息写入用户表，并为新用户创建数据表。
    :param user_data: 包含 'username', 'mima', 'age', 'sex', 'tel' 的字典。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            # 插入用户信息
            user_sql = """
                INSERT INTO user (USER_NAME, PASSWORD, AGE, SEX, TELEPHONE)
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(user_sql, (
                user_data["username"], user_data["mima"], user_data["age"],
                user_data["sex"], user_data["tel"]
            ))

            # 创建用户专属表
            table_sql = f"""
                CREATE TABLE DATA_{user_data['username']} (
                    ID INT AUTO_INCREMENT PRIMARY KEY,
                    WX INT, WY INT, WZ INT, AX INT, AY INT, AZ INT, STAGE INT,MCU_STAGE INT,MCU_FLOOR INT
                )
            """
            cursor.execute(table_sql)

            # 插入初始数据
            init_sql = f"""
                INSERT INTO DATA_{user_data['username']} (WX, WY, WZ, AX, AY, AZ, STAGE,MCU_STAGE,MCU_FLOOR)
                VALUES (64870, 165, 64573, 65098, 14138, 62298, 0,0,0)
            """
            cursor.execute(init_sql)

        connection.commit()
        print(f"用户 {user_data['username']} 注册成功")
    except Exception as e:
        print(f"注册用户失败：{e}")
    finally:
        connection.close()


# 验证用户
def verfy_user(credentials):
    """
    验证用户名和密码是否正确。
    :param credentials: 包含 'username' 和 'mima' 的字典。
    :return: 验证成功返回用户数据，否则返回 False。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            sql = """
                SELECT * FROM user WHERE USER_NAME=%s AND PASSWORD=%s
            """
            cursor.execute(sql, (credentials["username"], credentials["mima"]))
            result = cursor.fetchall()
            return result if result else False
    except Exception as e:
        print(f"验证用户失败：{e}")
        return False
    finally:
        connection.close()


# 获取用户阶段数据
def get_stage_data(username):
    """
    获取用户的最大阶段值。
    :param username: 用户名。
    :return: 最大阶段值。
    """
    result = ()
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            # 修正 SQL 查询语句，不设置别名
            sql = f"SELECT MAX(STAGE), MAX(MCU_STAGE), MAX(MCU_FLOOR) FROM DATA_{username}"
            cursor.execute(sql)  # 执行 SQL 查询
            result = cursor.fetchone()  # 获取结果作为一个元组
            print(result)  # 打印结果供调试

            # 返回结果按序输出
            return result if result else (0, 0, 0)  # 如果 result 为 None，则返回 (0, 0, 0)

    except Exception as e:
        print(f"获取阶段数据失败：{e}")
        return 0
    finally:
        connection.close()


# 数据导出
def export_data(username, include_header=True):
    """
    导出指定用户的数据表为 CSV 文件。
    :param username: 用户名。
    :param include_header: 是否包含列名。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            sql = f"SELECT * FROM DATA_{username}"
            cursor.execute(sql)
            rows = cursor.fetchall()

            # 获取列名
            column_names = [desc[0] for desc in cursor.description]

        # 转换为 Pandas DataFrame
        df = pd.DataFrame(rows, columns=column_names)

        # 保存为 CSV 文件
        filename = f"DATA_{username}.csv"
        df.to_csv(filename, index=False, header=include_header)
        print(f"数据导出成功：{filename}")
    except Exception as e:
        print(f"导出数据失败：{e}")
    finally:
        connection.close()


# 管理员获取所有用户数据
def admin_get_data():
    """
    获取所有用户的数据表内容。
    :return: 包含所有用户数据的字典。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            # 查询所有用户的表名
            cursor.execute("SHOW TABLES LIKE 'DATA_%'")
            tables = cursor.fetchall()

            # 获取所有用户数据
            all_data = {}
            for table in tables:
                table_name = table[0]
                cursor.execute(f"SELECT * FROM {table_name}")
                all_data[table_name] = cursor.fetchall()

        return all_data
    except Exception as e:
        print(f"管理员获取数据失败：{e}")
        return {}
    finally:
        connection.close()


def reset_user_table(username):
    """
    在收到 start 命令时删除指定用户的表，并重新创建一个新的表。
    :param username: 用户名，决定表名。
    """
    connection = get_db_connection()
    try:
        with connection.cursor() as cursor:
            # 删除现有表
            drop_table_sql = f"DROP TABLE IF EXISTS DATA_{username}"
            cursor.execute(drop_table_sql)
            print(f"表 DATA_{username} 已删除")

            # 创建新表
            create_table_sql = f"""
                CREATE TABLE DATA_{username} (
                    ID INT AUTO_INCREMENT PRIMARY KEY,
                    WX INT, WY INT, WZ INT, AX INT, AY INT, AZ INT, STAGE INT,MCU_STAGE INT,MCU_FLOOR INT
                )
            """
            cursor.execute(create_table_sql)
            # print(f"表 DATA_{username} 已重新创建")
            init_sql = f"""
                            INSERT INTO DATA_{username} (WX, WY, WZ, AX, AY, AZ, STAGE,MCU_STAGE,MCU_FLOOR)
                            VALUES (64870, 165, 64573, 65098, 14138, 62298, 0,0,0)
                        """
            cursor.execute(init_sql)

        connection.commit()
    except Exception as e:
        print(f"重置表失败：{e}")
    finally:
        connection.close()
