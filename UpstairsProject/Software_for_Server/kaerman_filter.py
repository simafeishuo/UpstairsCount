import numpy as np
import matplotlib.pyplot as plt


def KF(Z, over, Q, R):
    # 定义超参数
    over = over
    Q = Q  # 4e-4
    R = R  # 0.25
    # 定义尺寸函数
    cc = [over, 1]
    # 定义迭代的初始参数
    X_bar = np.zeros(cc)
    Xbar = np.zeros(cc)
    K = np.zeros(cc)
    P_ = np.zeros(cc)
    P = np.zeros(cc)
    P[0] = 1
    Xbar[0] = Z[0]
    # 循环迭代
    for n in range(1, over):
        # 时间更新
        X_bar[n] = Xbar[n - 1]
        P_[n] = P[n - 1] + Q
        # 状态更新
        K[n] = P_[n] / (P_[n] + R)
        Xbar[n] = X_bar[n] + K[n] * (Z[n] - X_bar[n])
        P[n] = (1 - K[n]) * P_[n]
    return Xbar


# 生成模拟测量值
#over = 50
#Z = (24 + np.sqrt(100) * np.random.normal(0, 1, size=over)).reshape(over, 1)
#print(Z)
#X = KF(Z, 50, 0.01, 100)

#print(type(X))
#print(Z)
#x = np.arange(over)
#fig, ax = plt.subplots()
#ax.plot(x, X, label='estimate')
#ax.plot(x, Z, label='measure')
#ax.legend()


