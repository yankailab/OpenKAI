# Vendored three.js r185 (npm 0.185.0)

Source: https://github.com/mrdoob/three.js/tree/r185

Files downloaded from the pinned npm distribution at
`https://cdn.jsdelivr.net/npm/three@0.185.0/`:

- `build/three.module.min.js`
- `build/three.core.min.js`
- `examples/jsm/controls/OrbitControls.js`
- `LICENSE` (stored as `LICENSE.three`)

License: MIT, reproduced in `LICENSE.three`. The sole local change is the
OrbitControls import from `three` to `./three.module.min.js`, allowing relative
imports without a bundler or import map. Keep all files on the same release.

SHA-256 of the files shipped here:

```text
8b378ebe60e2fe500158cb0ac71cb5e8b7d92953c2abcc63a0eb90499653b5bc  LICENSE.three
845c827054bda45b84ae0e9e0db4e707f75d2182af9cd88787b2cac86aeef8ef  OrbitControls.js
0e9dd2793e01d0d9eb4f2ab00b4ffcdd4488275ebebee5c31fa8d347bc29f0bf  three.core.min.js
86bcee248b64f44bcfc23c331ae74619061957d59cab040171dcb6fb5900beb6  three.module.min.js
```
