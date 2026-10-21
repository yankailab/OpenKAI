import { fileURLToPath } from 'node:url'
import { defineConfig } from 'vitepress'
import { createSidebar } from './sidebar'

const docsDirectory = fileURLToPath(new URL('..', import.meta.url))

export default defineConfig({
  title: 'OpenKAI Guide',
  description: 'Setup, development, vision, mapping, and hardware guides for OpenKAI.',
  lang: 'en',
  base: '/',
  cleanUrls: true,
  lastUpdated: true,
  srcExclude: ['public/**', '**/node_modules/**'],
  sitemap: { hostname: 'https://docs.openkai.tech' },
  head: [
    ['link', { rel: 'icon', type: 'image/png', href: '/favicon.png' }],
    ['meta', { name: 'theme-color', content: '#008bc8' }],
  ],
  themeConfig: {
    logo: { src: '/openkai-logo.png', alt: 'OpenKAI' },
    siteTitle: 'Guide',
    nav: [
      { text: 'Guide', link: '/1-setup', activeMatch: '^/(?!contribute(?:/|$)).+' },
      { text: 'Contribute', link: '/contribute/docs', activeMatch: '^/contribute/' },
    ],
    sidebar: createSidebar(docsDirectory),
    outline: { level: [2, 3], label: 'On this page' },
    search: { provider: 'local' },
    editLink: {
      pattern: 'https://github.com/yankailab/OpenKAI/edit/master/docs/:path',
      text: 'Edit this page on GitHub',
    },
    lastUpdated: { text: 'Last updated' },
    socialLinks: [{ icon: 'github', link: 'https://github.com/yankailab/OpenKAI' }],
    footer: {
      message: 'OpenKAI project documentation',
    },
  },
})
