# #suanfa

import database
import numpy as np
import pandas as pd
from scipy.stats import kurtosis, skew
import joblib
from concurrent.futures import ThreadPoolExecutor
import warnings

# 忽略特定类型的警告
warnings.filterwarnings("ignore", category=RuntimeWarning)
warnings.filterwarnings("ignore", category=UserWarning)

# 使用线程池进行并发优化
executor = ThreadPoolExecutor(max_workers=6)

# 加载模型
model = joblib.load('rf_model.joblib')


def normalize_data(data, L=-1.0, H=4.0):
    """
    数据归一化处理
    """
    max_val = np.max(data, axis=0)
    min_val = np.min(data, axis=0)
    return L + (H - L) * (data - min_val) / (max_val - min_val)


def extract_features(data):
    """
    提取特征，包括均值、标准差、峰度、偏度、最大值、最小值、相关性等。
    """
    features = []
    features.append(np.mean(data, axis=0))  # 均值
    features.append(np.std(data, axis=0))  # 标准差
    features.append(np.sqrt(np.mean(data ** 2, axis=0)))  # 均方根
    features.append(kurtosis(data, axis=0))  # 峰度
    features.append(skew(data, axis=0))  # 偏度
    features.append(np.mean(np.sort(data, axis=0)[-5:], axis=0))  # 最大值
    features.append(np.mean(np.sort(data, axis=0)[:5], axis=0))  # 最小值

    # 计算相关性
    correlation_matrix = np.corrcoef(data, rowvar=False)
    correlation_features = [
        correlation_matrix[0, 1], correlation_matrix[0, 2], correlation_matrix[1, 2],
        correlation_matrix[3, 4], correlation_matrix[3, 5], correlation_matrix[4, 5]
    ]
    features.append(correlation_features)

    # 展平所有特征
    return np.concatenate(features, axis=0)


def process_window(data_window):
    """
    处理数据窗口并预测阶段值。
    """
    # 特征提取
    features = extract_features(data_window)
    features[np.isnan(features)] = 10  # 替换 NaN 为默认值
    features = features.reshape(1, -1)  # 调整形状为模型输入格式
    # 模型预测
    prediction = model.predict(features)
    return prediction[0]


def main(user_data):
    """
    算法主函数，接收用户数据，返回阶段更新值。
    """
    # 数据归一化
    normalized_data = normalize_data(np.array(user_data).astype(float))

    # 使用最后 25 条数据进行窗口分析
    data_window = normalized_data[-25:, :]

    # 提交任务至线程池，异步处理特征提取和模型预测
    future = executor.submit(process_window, data_window)
    return future.result()
