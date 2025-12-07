import { DefaultTheme } from "vitepress";
import { core_hpp_document_item } from "./core.mts";

export const core_toolkit_index: DefaultTheme.SidebarItem[] = [
    {
        text: '核心库文档',
        base: '/md/core/',
        collapsed: true,
        items: [
            core_hpp_document_item
        ]
    }
]