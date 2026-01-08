# `<tuple_like.hpp>` 公共traits

## member_count {#member_count}

<DeclarationTable
    header="rainy/meta/moon/type_like.hpp"
    :content="`
(1) (since C++17)
template <typename Ty>
struct member_count;
`
"
/>

<DescriptSection>

获取聚合类的成员个数。

</DescriptSection>

<ParameterSection isTypeParameter>

`Ty`: 要获取的对应类型

</ParameterSection>

<AttentionSection level="as_reminder">

如果没有定义任何注册宏或是该类并非聚合类的时候，则无法获取

</AttentionSection>

<RemarkSection>

默认尝试从聚合类中获取大小，如果特化了reflectet_for_type，即`is_reflectet_for_type_valid<type_traits::cv_modify::remove_cvref_t<Ty>>`表达式结果为true时，将返回该特化指定的大小，另外，RAINY_REFLECT_TUPLE_LIKE和RAINY_PRIVATE_REFLECT_TUPLE_LIKE定义的注册也同样适用。

</RemarkSection>

<InnerMemberDefine
    :titles="['成员', '描述']"
    :data="[
        ['value', '一个静态变量，traits通过该成员传递数据']
    ]"
/>

<!-- <DeclarationTable
    header="rainy/meta/moon/type_like.hpp"

/> -->