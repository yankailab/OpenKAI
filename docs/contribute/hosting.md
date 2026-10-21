# Hosting and domain setup

The documentation site's intended public address is **https://docs.openkai.tech**. VitePress builds static pages, GitHub Actions uploads the build, and GitHub Pages serves the result. The separate **www.openkai.tech** portal uses the Onamae rental server.

## GitHub Pages configuration

In [OpenKAI's Pages settings](https://github.com/yankailab/OpenKAI/settings/pages), use:

| Setting | Value |
| --- | --- |
| Build and deployment source | GitHub Actions |
| Custom domain | `docs.openkai.tech` |
| Enforce HTTPS | Enabled once GitHub has issued the certificate |

For a fresh setup, remove an obsolete `github-pages` environment under **Settings → Environments** before the first deployment. The workflow creates the environment again when it deploys. Reapply any desired protection rules to the new environment, allowing deployments from `master`.

The workflow is `.github/workflows/docs.yml`. Its build job installs the locked dependencies with `npm ci`, runs VitePress, and uploads `docs/.vitepress/dist/`. A separate deployment job publishes successful builds from `master` to the `github-pages` environment. Pull requests build the site without publishing it.

The workflow needs `contents: read` to read the repository. Only its deployment job receives `pages: write` and `id-token: write`. The `github-pages` environment should allow deployments from `master`.

`docs/public/CNAME` records `docs.openkai.tech` and is copied into the artifact. For an Actions-based Pages site, this file alone does **not** configure the custom domain: the custom domain must also be saved in the repository's Pages settings. Keep both values aligned.

The site uses `/` as its base because it is served at a custom domain, and `.vitepress/config.ts` uses `https://docs.openkai.tech` as the sitemap hostname.

## Onamae DNS

Manage the record in the DNS service authoritative for `openkai.tech`. When the domain uses Onamae rental-server nameservers `ns-rs1.gmoserver.jp` and `ns-rs2.gmoserver.jp`, edit the DNS zone from the rental-server control panel. A record entered into a separate, inactive DNS service will not affect the public domain.

Add this record:

| Type | Host name | Destination |
| --- | --- | --- |
| CNAME | `docs` | `yankailab.github.io` |

Some consoles show the host as `docs.openkai.tech`; the result should be the same full DNS name. Use the console's default TTL. The destination is a hostname: do not include `https://`, `/OpenKAI`, or any other path. DNS tools may display it with a trailing dot.

In the [Onamae rental-server control panel](https://cp.onamae.ne.jp/), open
**ドメイン → openkai.tech → DNS → +DNSレコードを追加**. Enter the record above,
then confirm and add it. See [Onamae's DNS record guide](https://www.onamae-server.com/guide/rs/p/25).

If another record already exists specifically at `docs.openkai.tech`, replace the conflicting record when adding this CNAME. Keep the domain's nameservers and the existing root, `www`, mail, and other records unchanged. The `docs` CNAME routes only the documentation subdomain to GitHub Pages.

For domain ownership verification, GitHub may provide a unique TXT record under the account or organization's Pages settings. Copy the exact host and value GitHub supplies into this same authoritative DNS zone. Do not invent a verification value.

See [GitHub's custom subdomain instructions](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site/managing-a-custom-domain-for-your-github-pages-site#configuring-a-subdomain) and [domain verification guide](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site/verifying-your-custom-domain-for-github-pages).

## Verify a deployment

1. Confirm the latest **Deploy documentation** workflow on `master` completed successfully.
2. Check that Pages settings report the custom-domain DNS check as successful.
3. Allow GitHub to finish issuing its HTTPS certificate, then enable **Enforce HTTPS**.
4. Open `https://docs.openkai.tech`, follow a guide link, and try local search.
5. Open `https://www.openkai.tech` separately to confirm the portal still resolves to its own host.

Useful command-line checks:

```bash
dig +short NS openkai.tech
dig +short CNAME docs.openkai.tech
curl -I https://docs.openkai.tech
```

The CNAME lookup should return `yankailab.github.io.`. DNS caches and certificate issuance can take time after a record changes. If GitHub cannot validate DNS, first check which nameservers are authoritative and whether the `docs` CNAME is visible there.

## Maintain the site

Documentation changes deploy from `master`; generated files do not need to be committed and a `gh-pages` branch is unnecessary. To roll back a documentation change, revert its source commit and let the workflow build it again.

For dependency updates, change the pinned VitePress version in `docs/package.json`, regenerate `docs/package-lock.json`, and run a production build before merging. Keep the workflow's Node version consistent with the supported version documented in `docs/package.json`.

When changing the public hostname, update the GitHub Pages custom domain, DNS, `docs/public/CNAME`, and the sitemap hostname together.
