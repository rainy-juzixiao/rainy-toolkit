# `<tuple_like.hpp>` 函数

## get_member_names {#get_member_names}

<DeclarationTable
    header="rainy/meta/moon/type_like.hpp"
    :content="`
(1) (since C++17)
template <typename Ty>
constexpr rain_fn get_member_names() noexcept -> auto;`
"
/>

<DescriptSection>

尝试获取指定类型中所有成员的名称。

</DescriptSection>

<ParameterSection isTypeParameter>

`Ty`: 要获取的对应类型

</ParameterSection>

<ReturnValueSection>

返回指定类型中所有成员的名称。

</ReturnValueSection>

<AttentionSection>

get_member_names以及其依赖的函数在IDE中可能会导致错误的结果在constexpr求值中，但实际编译期求值会得到正确的结果。

因此，如果对IDE中的结果有洁癖，请尽可能避免获取字符串在编译期，除非，你定义了注册，这样才能确保IDE生成正确的结果。

</AttentionSection>
