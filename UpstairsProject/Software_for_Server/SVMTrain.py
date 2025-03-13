import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from sklearn.metrics import accuracy_score
from joblib import dump, load

# 读取CSV文件
data = pd.read_csv('data.csv')

# 提取特征和标签
X = data.drop(columns=['label'])  # 特征
y = data['label']  # 标签

# 划分训练集和测试集
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 定义支持向量机分类器（使用二次核函数）
svm_classifier = SVC(kernel='poly', degree=2, random_state=42)

# 训练模型
svm_classifier.fit(X_train, y_train)

# 在测试集上进行预测
y_pred = svm_classifier.predict(X_test)

# 计算准确率
accuracy = accuracy_score(y_test, y_pred)
print("准确率:", accuracy)

# 保存模型
dump(svm_classifier, 'rf_model.joblib')
