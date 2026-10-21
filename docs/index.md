---
layout: home
markdownStyles: false
title: OpenKAI Guide
titleTemplate: false
hero:
  name: OpenKAI
  text: Project documentation
  tagline: Setup, configuration, development, and integration guides.
  image:
    src: /openkai-logo.png
    alt: OpenKAI
  actions:
    - theme: brand
      text: Get started
      link: /1-setup
    - theme: alt
      text: View source
      link: https://github.com/yankailab/OpenKAI
features:
  - title: Set up OpenKAI
    details: Install dependencies, configure modules, and prepare your development environment.
    link: /1-setup
    linkText: Setup and installation
  - title: Develop and integrate
    details: Follow the C++ conventions and understand DataObjects, geometry streams, and timestamp units.
    link: /DataStreamGeometry
    linkText: DataObjects and interfaces
  - title: Vision and mapping
    details: Read the OpenCV, YOLO26, GLIM, and 3D viewer integration guides.
    link: /opencv
    linkText: Vision setup
---

<div class="vp-doc openkai-home-content">

## Explore the guides

| Topic | Guides |
| --- | --- |
| Getting started | [Installation](./1-setup.md) · [Configuration](./2-config.md) · [Optimization](./3-optimize.md) |
| Tools and operation | [Tools](./4-tools.md) · [Nginx](./5-nginx.md) · [Testing](./6-test.md) · [Deployment](./7-deploy.md) |
| Development | [C++ style](./coding_style.md) · [Timestamp units](./time_units.md) · [DataObjects](./DataStreamGeometry.md) |
| Vision | [OpenCV](./opencv.md) · [YOLO26 detection](./YOLO26detectONNX.md) · [YOLO26 depth](./YOLO26depthEstONNX.md) |
| Mapping and 3D | [GLIM](./GLIM.md) · [Open3D](./open3D.md) · [Browser viewer](./3D/WebViewer3D.md) · [Dear ImGui viewer](./3D/ImGUIviewer.md) |
| Navigation | [ArduPilot](./ArduPilot/AP_dev_setup.md) · [RTKLIB](./rtklib.md) · [Unicore receivers](./unicore.md) |
| Contribute | [Write documentation](./contribute/docs.md) · [Hosting and domain setup](./contribute/hosting.md) |

The sidebar and local search are generated from Markdown files in the repository's `docs/` directory. Use **Edit this page on GitHub** on a guide to propose an update.

</div>
