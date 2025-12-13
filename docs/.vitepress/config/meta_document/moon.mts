import { DefaultTheme } from "vitepress";

export const moon_hpp_document_item: DefaultTheme.SidebarItem = {
    text: 'moon',
    collapsed: true,
    items: [
        {
            text: 'moon序言',
            link: '/moon/',
        },
        {
            text: '&lt;tuple_like.hpp&gt;',
            collapsed: true,
            items: [
                {
                    text: '&lt;tuple_like.hpp&gt;',
                    link: '/moon/tuple_like'
                }
            ]
        },
        {
            text: '&lt;enumeration.hpp&gt;',
            link: '/moon/enumeration'
        }
    ]
};