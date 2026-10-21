import { readFileSync, readdirSync } from 'node:fs'
import { basename, join } from 'node:path'
import type { DefaultTheme } from 'vitepress'

const ignoredDirectories = new Set(['.vitepress', 'node_modules', 'public'])

const sections = [
  {
    text: 'Getting started',
    pages: ['1-setup', '2-config', '3-optimize', '4-tools', '5-nginx', '6-test', '7-deploy'],
  },
  {
    text: 'Development',
    pages: ['coding_style', 'time_units', 'DataStreamGeometry'],
  },
  {
    text: 'Vision',
    pages: ['opencv', 'YOLO26detectONNX', 'YOLO26depthEstONNX'],
  },
  {
    text: 'Mapping and 3D',
    pages: ['GLIM', 'open3D', '3D/WebViewer3D', '3D/ImGUIviewer'],
    directory: '3D',
  },
  {
    text: 'Navigation and autopilot',
    pages: ['ArduPilot/AP_dev_setup', 'rtklib', 'unicore'],
    directory: 'ArduPilot',
  },
  {
    text: 'Other guides',
    pages: ['misc'],
    directory: '',
  },
  {
    text: 'Contribute',
    pages: ['contribute/docs', 'contribute/hosting'],
    directory: 'contribute',
  },
]

const titles: Record<string, string> = {
  '1-setup': 'Setup and installation',
  '2-config': 'Configuration',
  '3-optimize': 'System optimization',
  '4-tools': 'Tools',
  '5-nginx': 'Nginx and web access',
  '6-test': 'Testing and debugging',
  '7-deploy': 'Deployment and SD images',
  coding_style: 'C++ coding style',
  time_units: 'Timestamp units',
  DataStreamGeometry: 'DataObjects and geometry streams',
  opencv: 'OpenCV',
  YOLO26detectONNX: 'YOLO26 object detection',
  YOLO26depthEstONNX: 'YOLO26 depth estimation',
  GLIM: 'GLIM',
  open3D: 'Open3D',
  '3D/WebViewer3D': 'Browser 3D viewer',
  '3D/ImGUIviewer': 'Dear ImGui 3D viewer',
  'ArduPilot/AP_dev_setup': 'ArduPilot development',
  rtklib: 'RTKLIB',
  unicore: 'Unicore RTK receivers',
  misc: 'Miscellaneous notes',
  'contribute/docs': 'Writing documentation',
  'contribute/hosting': 'Hosting and domain setup',
}

function discoverMarkdown(directory: string, relative = ''): string[] {
  return readdirSync(join(directory, relative), { withFileTypes: true })
    .sort((left, right) => left.name.localeCompare(right.name, 'en'))
    .flatMap((entry) => {
      const path = relative ? `${relative}/${entry.name}` : entry.name
      if (entry.isDirectory()) {
        return entry.name.startsWith('.') || ignoredDirectories.has(entry.name)
          ? []
          : discoverMarkdown(directory, path)
      }
      return entry.isFile() && entry.name.endsWith('.md') && path !== 'index.md'
        ? [path.slice(0, -3)]
        : []
    })
}

function readableName(value: string): string {
  return value.replace(/[-_]/g, ' ').replace(/^\d+\s+/, '').replace(/^./, (letter) => letter.toUpperCase())
}

function pageTitle(directory: string, page: string): string {
  if (titles[page]) return titles[page]
  const markdown = readFileSync(join(directory, `${page}.md`), 'utf8')
  const heading = markdown.match(/^#\s+(.+?)\s*#*\s*$/m)?.[1]
  return heading
    ? heading.replace(/\[([^\]]+)\]\([^)]*\)/g, '$1').replace(/[`*_]/g, '')
    : readableName(basename(page))
}

export function createSidebar(directory: string): DefaultTheme.SidebarItem[] {
  const pages = discoverMarkdown(directory)
  const remaining = new Set(pages)
  const curatedPages = new Set(sections.flatMap((section) => section.pages))
  const item = (page: string): DefaultTheme.SidebarItem => ({
    text: pageTitle(directory, page),
    link: `/${page.endsWith('/index') ? page.slice(0, -5) : page}`,
  })
  const sidebar: DefaultTheme.SidebarItem[] = [{ text: 'Overview', link: '/' }]

  for (const section of sections) {
    const sectionPages = section.pages.filter((page) => remaining.has(page))
    if (section.directory !== undefined) {
      sectionPages.push(...pages.filter((page) =>
        remaining.has(page)
        && !curatedPages.has(page)
        && (section.directory === '' ? !page.includes('/') : page.startsWith(`${section.directory}/`)),
      ))
    }
    sectionPages.forEach((page) => remaining.delete(page))
    if (sectionPages.length) {
      sidebar.push({ text: section.text, collapsed: false, items: sectionPages.map(item) })
    }
  }

  // New folders become sections without requiring a navigation update.
  const extraSections = new Map<string, string[]>()
  for (const page of remaining) {
    const section = page.split('/')[0]
    extraSections.set(section, [...(extraSections.get(section) ?? []), page])
  }
  for (const [section, sectionPages] of extraSections) {
    sidebar.push({ text: readableName(section), collapsed: false, items: sectionPages.map(item) })
  }
  return sidebar
}
