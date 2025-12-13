import { DefaultTheme } from "vitepress";
import { moon_hpp_document_item } from "./moon.mts";

export const meta_index: DefaultTheme.SidebarItem[] = [
    {
        text: 'meta库文档',
        base: '/md/meta/',
        collapsed: true,
        items: [
            moon_hpp_document_item
        ]
    }
]