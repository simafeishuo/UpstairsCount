clear; clc; close all;

% 读取CSV文件并提取数据
data = csvread('received_data.csv', 1, 1);  % 从第二行第二列开始读取
da_len = length(data);

% 对数据进行平滑和滤波处理
for j = 1:6
    data(:, j) = smooth(data(:, j));
    data(:, j) = smooth(data(:, j));
    data(:, j) = medfilt1(data(:, j), 10); % 使用中值滤波
end

% 数据归一化
L = -1;
H = 4;
for m = 1:da_len
    max_val = max(data(:,:));   % 找到数据中的最大值
    min_val = min(data(:,:));   % 找到数据中的最小值
    new_data(m, :) = L + (H - L) * (data(m, :) - min_val) / (max_val - min_val); % 数据归一化
end

% 计算峰值和谷值
eval = [];
for ggg = 1:1
    close all;
    y = new_data(1:end, ggg);
    len = length(data);
    x = 1:len;
    
    % 绘制数据
    plot(x, y);
    figure;
    
    % 找到数据中的峰值和谷值
    [peaks, peak_locs] = findpeaks(y);
    [lows, low_locs] = findpeaks(-y);
    lows = -lows;
    
    % 绘制峰值和谷值
    plot(x, y);
    hold on;
    plot(x(peak_locs), peaks, 'ro', 'MarkerFaceColor', 'r');
    plot(x(low_locs), lows, 'ro', 'MarkerFaceColor', 'g');
    hold off;

    % 显示结果
    num_peaks = length(peaks);
    size = min(length(lows), length(peaks));
    peak_locs = peak_locs(1:size);
    low_locs = low_locs(1:size);
    fprintf('数据中的峰值数量为: %d\n', num_peaks);
    
    % 计算峰值和谷值之间的差值并存储
    cha = abs(data(peak_locs, ggg) - data(low_locs, ggg));
    eval(ggg) = mean(cha);
end

% 数据分析窗口长度
window_len = 25;
data = new_data;  % 使用new_data进行分析
zuobiao = peak_locs;  % 使用生成的peak_locs
leng = length(data);

% 初始化各类窗口统计结果
aver_window = [];
stand_window = [];
count = 1;
rmse_window = [];
fengdu_window = [];
piandu_window = [];
corr_window = [];
max_window = [];
min_window = [];
label_window = [];

% 按照窗口分析数据
for i = 1:ceil(window_len):leng
    low = max(1, i - floor(window_len / 2));
    upp = min(leng, i + floor(window_len / 2));
    fprintf("%d %d\n", low, upp);
    
    % 提取窗口数据
    data_window = data(low:upp, :);
    
    % 计算窗口统计特征
    aver_window(count, :) = mean(data_window);
    stand_window(count, :) = std(data_window);
    rmse_window(count, :) = rms(data_window);
    fengdu_window(count, :) = kurtosis(data_window);
    piandu_window(count, :) = skewness(data_window);
    max_window(count, :) = mean(maxk(data_window, 5));
    min_window(count, :) = mean(mink(data_window, 5));
    tes = corr(data_window);
    corr_window(count, :) = [tes(1, 2), tes(1, 3), tes(2, 3), tes(4, 5), tes(4, 6), tes(5, 6)];
    
    % 判断是否包含峰值位置
    for j = 1:length(zuobiao)
        if zuobiao(j) >= low && zuobiao(j) <= upp
            label_window(count) = 1;  % 标签为1，表示包含峰值
            break
        else
            if j == length(zuobiao)
                label_window(count) = 0;  % 标签为0，表示不包含峰值
            end
        end
    end
    count = count + 1;
end

% 转置标签矩阵
label_window = label_window';
% 汇总所有窗口特征
total_window = [aver_window, stand_window, rmse_window, max_window, min_window, fengdu_window, piandu_window, corr_window];
data_woo = [total_window, label_window];

% 输出为CSV文件
column_names = {'gx_mean', 'gy_mean', 'gz_mean', 'ax_mean', 'ay_mean', 'az_mean', ...
                'gx_std', 'gy_std', 'gz_std', 'ax_std', 'ay_std', 'az_std', ...
                'gx_rmse', 'gy_rmse', 'gz_rmse', 'ax_rmse', 'ay_rmse', 'az_rmse', ...
                'gx_max', 'gy_max', 'gz_max', 'ax_max', 'ay_max', 'az_max', ...
                'gx_min', 'gy_min', 'gz_min', 'ax_min', 'ay_min', 'az_min', ...
                'gx_fengdu', 'gy_fengdu', 'gz_fengdu', 'ax_fengdu', 'ay_fengdu', 'az_fengdu', ...
                'gx_piandu', 'gy_piandu', 'gz_piandu', 'ax_piandu', 'ay_piandu', 'az_piandu', ...
                'gx_gy_coor', 'gx_gz_corr', 'gy_gz_corr', 'ax_ay_corr', 'ax_az_corr', 'ay_az_corr', 'label'};

% 将数据保存为CSV文件
writetable(array2table(data_woo, 'VariableNames', column_names), 'data.csv');

% 输出标签为1的窗口数量
sum(label_window)
