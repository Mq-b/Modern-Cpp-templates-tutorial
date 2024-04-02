import { defineConfig } from 'vitepress';
import { repo_name, repo_url } from './theme/params';
import footnote_plugin from 'markdown-it-footnote';

const tutorial_path = '/md/第一部分-基础知识/';
const homework_path = '/homework/08折叠表达式作业/';

// https://vitepress.dev/reference/site-config
export default defineConfig({
  title: "现代 C++ 模板教程",
  head: [['link', { rel: 'icon', href: repo_name + '/icon.webp' }]],
  rewrites: { 'README.md': 'index.md' },
  base: repo_name + '/',
  themeConfig: {
    // https://vitepress.dev/reference/default-theme-config
    sidebar: [
      {
        items: [
          { text: '首页', link: '/' },
          { text: '阅读须知', link: '/md/README' },
          { text: '函数模板', link: tutorial_path + '01函数模板' },
          { text: '类模板', link: tutorial_path + '02类模板' },
          { text: '变量模板', link: tutorial_path + '03变量模板' },
          { text: '模板全特化', link: tutorial_path + '04模板全特化' },
          { text: '模板偏特化', link: tutorial_path + '05模板偏特化' },
          { text: '模板显式实例化解决模板分文件问题', link: tutorial_path + '06模板显式实例化解决模板分文件问题' },
          { text: '显式实例化解决模板导出静态动态库', link: tutorial_path + '07显式实例化解决模板导出静态动态库' },
          { text: '折叠表达式', link: tutorial_path + '08折叠表达式' },
          { text: '待决名', link: tutorial_path + '09待决名' },
          { text: '了解与利用SFINAE', link: tutorial_path + '10了解与利用SFINAE' },
          { text: '约束与概念', link: tutorial_path + '11约束与概念' },
          {
            text: '作业提交展示',
            collapsed: true,
            items: [
              { text: 'mq卢瑟', link: homework_path + 'mq卢瑟' },
              { text: 'roseveknif', link: homework_path + 'roseveknif' },
              { text: 'mq日', link: homework_path + 'mq日' },
              { text: 'saidfljwnzjasf', link: homework_path + 'saidfljwnzjasf' },
              { text: 'ooolize', link: homework_path + 'ooolize' },
            ]
          },
          { text: '总结', link: tutorial_path + '12总结' },
        ]
      },
    ],
    logo: '/icon.webp',
    editLink: {
      pattern: repo_url + 'blob/main/:path',
      text: '在 GitHub 上编辑此页面'
    },
    socialLinks: [{ icon: 'github', link: repo_url }],

    search: { provider: 'local' },
    outline: { label: '页面导航' },
    returnToTopLabel: '回到顶端',
  },
  markdown: {
    config(md) {
      md.use(footnote_plugin);
    }
  },

  cleanUrls: true,
  ignoreDeadLinks: true,
})
