import pandas as pd
import matplotlib.pyplot as plt

# 读取 benchmark 的 CSV 输出
df = pd.read_csv("reflection_invoke_benchmark.csv")

# 筛选出包含 "invoke_method" 和 "invoke_variadic_method" 的 benchmark
df_method = df[df["name"].str.contains("invoke_method")].copy()
df_variadic = df[df["name"].str.contains("invoke_variadic_method")].copy()

# 从 name 中提取库名和参数数量
def parse_info(name):
    parts = name.split("_")
    # 找到库名（如 rainytoolkit / rttr / ubpa）
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
    # 提取最后的参数数量（例如：_6）如果没有则是 0
    for p in reversed(parts):
        if p.isdigit():
            return lib, int(p)
    return lib, 0

# 使用 .loc 方法进行赋值
temp_method = df_method["name"].apply(lambda x: pd.Series(parse_info(x)))
df_method.loc[:, "lib"] = temp_method[0]
df_method.loc[:, "arg_count"] = temp_method[1]

temp_variadic = df_variadic["name"].apply(lambda x: pd.Series(parse_info(x)))
df_variadic.loc[:, "lib"] = temp_variadic[0]
df_variadic.loc[:, "arg_count"] = temp_variadic[1]

# 创建并排的两个子图
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))  # 改为 1行2列

# 绘制 invoke_method 的折线图
for lib, group in df_method.groupby("lib"):
    group_sorted = group.sort_values("arg_count")
    ax1.plot(group_sorted["arg_count"], group_sorted["cpu_time"], marker='o', label=lib)

ax1.set_xlabel("Number of Arguments")
ax1.set_ylabel("CPU Time (ns)")
ax1.set_title("Method Call Performance (invoke_method)")
ax1.legend()
ax1.grid(True)

# 绘制 invoke_variadic_method 的折线图
for lib, group in df_variadic.groupby("lib"):
    group_sorted = group.sort_values("arg_count")
    ax2.plot(group_sorted["arg_count"], group_sorted["cpu_time"], marker='o', label=lib)

ax2.set_xlabel("Number of Arguments")
ax2.set_ylabel("CPU Time (ns)")
ax2.set_title("Method Call Performance (invoke_variadic_method)")
ax2.legend()
ax2.grid(True)

plt.tight_layout()
plt.show()