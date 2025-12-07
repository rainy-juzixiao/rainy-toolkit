def generate_macros():
    # 前20个明确列出
    macros = []
    macros.append("// 基础定义（1-20）")
    for i in range(1, 21):
        macros.append(f"#define RAINY_INITIALIZER_LIST_{i} {{{', {}'.format('' if i == 1 else '')}" + (", {}" * (i-1)) + "}")
    
    macros.append("\n// 递归定义（21-256）")
    for i in range(21, 257):
        macros.append(f"#define RAINY_INITIALIZER_LIST_{i} RAINY_INITIALIZER_LIST_{i-1} ,{{}}")
    
    return "\n".join(macros)

if __name__ == "__main__":
    print("#define RAINY_INITIALIZER_LIST(N) RAINY_INITIALIZER_LIST_##N")
    print()
    print(generate_macros())