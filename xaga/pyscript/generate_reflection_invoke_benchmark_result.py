import pandas as pd
import matplotlib.pyplot as plt

# 读取 benchmark 的 CSV 输出
df = pd.read_csv("reflection_invoke_benchmark.csv")

# 仅筛选包含 "invoke_method" 的 benchmark
df_method = df[df["name"].str.contains("invoke_method")].copy()

# 从 name 中提取库名和参数数量
def parse_info(name):
    parts = name.split("_")
    # 找到库名
    if "rainytoolkit" in parts:
        lib = "rainy-toolkit by rainy-juzixiao"
    elif "rttr" in parts:
        lib = "RTTR by rttr org"
    elif "udrefl" in parts:
        lib = "UDRefl by upba"
    elif "meta" in parts:
        lib = "meta library by skyjpack"
    else:
        lib = "unknown"
    # 提取参数数量
    for p in reversed(parts):
        if p.isdigit():
            return lib, int(p)
    return lib, 0

# 解析库名与参数数量
temp_method = df_method["name"].apply(lambda x: pd.Series(parse_info(x)))
df_method.loc[:, "lib"] = temp_method[0]
df_method.loc[:, "arg_count"] = temp_method[1]

# 创建图表
fig, ax1 = plt.subplots(figsize=(8, 6))  # 单个图表

# 绘制 invoke_method 的折线图
for lib, group in df_method.groupby("lib"):
    group_sorted = group.sort_values("arg_count")
    ax1.plot(group_sorted["arg_count"], group_sorted["cpu_time"], marker='o', label=lib)

ax1.set_xlabel("Number of Arguments")
ax1.set_ylabel("CPU Time (ns)")
ax1.set_title("Method Call Performance (invoke_method)")
ax1.legend()
ax1.grid(True)

plt.tight_layout()
plt.show()
