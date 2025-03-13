### readme.md

# 上楼检测软件

本项目是一个基于异步通信和机器学习的上楼检测系统，结合了实时数据采集、数据处理、数据库交互、模型预测和 Web 前端展示等功能。分软件部分和硬件部分。
**版权声明：上传的初始版本已申请著作权保护，未经允许不得运用于商业目的**

## 功能概述

- **异步通信**：通过 `asyncio` 构建异步 TCP 服务器，实现高并发客户端的连接和数据处理。
- **传感器数据处理**：支持从客户端接收传感器数据，进行数据归一化、特征提取，并通过机器学习模型进行预测。
- **机器学习预测**：使用随机森林模型进行数据阶段预测，通过 `scikit-learn` 进行训练和模型存储。
- **数据库管理**：使用 `pymysql` 操作 MySQL 数据库，支持用户注册、登录、数据存储等。
- **Web 后台管理**：提供基于 `Flask` 的前端 Web 应用，支持用户登录、注册、查看数据。

## 技术栈
### 软件部分
- **Python 版本**：3.8+
- **主要库**：
  - `asyncio`：异步网络通信与高并发处理
  - `pandas`：数据处理和导出
  - `numpy`：数值计算与数据处理
  - `scipy`：统计计算（峰度、偏度等）
  - `scikit-learn`：机器学习训练和模型预测
  - `joblib`：保存和加载机器学习模型
  - `Flask`：Web 后端框架
  - `pymysql`：数据库交互
### 硬件部分
- 基于STM32F407平台的开发板所支持
- 使用keil 5.32
- 基于正点原子MPU6050六轴传感器示例改造而来
- 使用ESP8266 WIFI芯片传输数据


## 安装与运行
### 软件部分
#### 1. 克隆项目

```bash
git clone https://github.com/simafeishuo/UpstairsCount.git
cd UpstairsProject
```

#### 2. 安装依赖库

确保你已经安装了 Python 3.8 及以上版本，然后使用以下命令安装项目的所有依赖：
推荐新建一个合适的conda环境
```bash
conda create -n upstairs python=3.8
conda activate upstairs
pip install -r requirements.txt
conda deactivate
```
#### 3. 数据库配置

在 MySQL 中创建名为 `upstair` 的数据库，并执行以下 SQL 脚本初始化数据库：

```sql
CREATE DATABASE upstair;
USE upstair;

CREATE TABLE user (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    USER_NAME VARCHAR(255),
    PASSWORD VARCHAR(255),
    AGE INT,
    SEX VARCHAR(10),
    TELEPHONE VARCHAR(20)
);
```

#### 4. 启动 Web 服务

启动 Flask Web 应用，运行以下命令：

```bash
python qianduan.py
```

Flask Web 应用将在 `http://127.0.0.1:5000` 启动，提供用户登录、注册和数据导出等功能。

#### 5. 启动 TCP 服务器

启动服务器代码，监听客户端的连接并处理传感器数据：

```bash
python server.py
```

服务器将监听 `8082` 端口，等待客户端的数据连接。

#### 6. 启动客户端

启动客户端程序，模拟传感器数据的发送：

```bash
python new1_cilent1.py
```

客户端会向服务器发送数据，进行实时数据传输和处理。

### 硬件部分
使用南邮提供的STM32F407开发板或正点原子的STM32F407开发板套件
本项目仅提供使用WIFI连接的版本
其他协议请自行查询更换

#### 1. 打开STM32文件中的USER文件夹中的MPU6050.uvprojx

#### 2. 打开main.c并配置
``` keil
# define WIFI_SSID "your_SSID"
# define WIFI_PASSWORD "your_password"
# define SERVER_IP "your_server_IP"
# define SERVER_PORT your_server_port
```
#### 重新烧录文件

### 模型调试
本项目提供接受原始传感器数据并进行自动标注的代码
需使用matlab2023a及以上版本使用

```
调试模型需用到的文件
Software_for_Server
|——shuju_jieshou.py
|——tiaoshi.m
```

以下为推荐的调试流程，请使用已经配置好网络连接的开发板操作
#### 1 运行shuju_jieshou.py

#### 2 携带开发板爬楼梯、平地走路等运动

请保证数据的量足够多，推荐至少爬楼梯一个小时

#### 3 运行tiaoshi.m文件，使用matlab的分类学习工具箱训练data_woo数据表

注意，干扰数据需手动将标签置0
如果自动标注的数量差距过大，请添加或减少滤波器

#### 4 选择效果较好的模型，选用对应的scikit-learn代码训练并保存相应的模型文件


## 项目结构
```
UpstairsProject
|————Software_for_Server # 软件部分
|————Software_for_STM32  # 硬件部分
```
```
软件部分
Software_for_Server/
│
├── database.py         # 数据库操作模块，包含用户管理、数据插入、导出等功能
├── server.py           # 异步 TCP 服务器模块，处理客户端连接和数据传输
├── suanfa.py           # 数据处理与特征提取模块
├── xunlain_wangluo.py  # 随机森林模型训练与预测模块
|—— kaerman_filter      # 卡尔曼滤波
├── new1_cilent1.py     # 模拟客户端数据传输模块
├── qianduan.py         # Flask Web 后台管理模块
├── up_data1.txt        # 客户端读取的传感器数据文件
├── rf_model.joblib     # 训练好的随机森林模型
|—— requirements.txt    # 项目依赖库列表
|—— templates           # 网页模版
|—— static              # 未知
```
```
硬件部分
Software_for_STM32
|——CORE
|——FWLIB
|——HARDWARE
|——OBJ
|——SYSTEM
|——USER
|——USMART
|——keilkill.bat
```
```
调试部分
Software_for_Server
|——shuju_jieshou.py
|——tiaoshi.m
```

## 贡献

欢迎任何形式的贡献，包括提交问题、修复 bug 或添加新功能。如果你有兴趣参与，请通过拉取请求 (Pull Requests) 提交你的代码。

## 许可

本项目遵循 MIT 许可证，详细信息请查看 LICENSE 文件。

---

此项目设计用于使用六轴运动传感器检测用户上楼姿态，适用于需要大规模数据传输和快速预测的应用场景。