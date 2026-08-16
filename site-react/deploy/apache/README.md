# Apache configuration for the site

Copies of the vhost files that serve the site on `theater`, kept here so the
hostname and redirect rules are under version control. They are **copies, not the
source of truth** — Apache reads `/etc/apache2/sites-available/`, and certbot
edits those files in place when it renews or extends the certificate. After
changing anything on the server, copy it back here.

| File | Purpose |
|------|---------|
| `sptk.conf` | port 80 — sends everything to `https://xmq.sptk.net` |
| `sptk-le-ssl.conf` | port 443 — serves the site, redirects the other hostnames |

## Hostnames

The site answers to three names and canonicalises them to one:

- `sptk.net` and `www.sptk.net` → 301 → `https://xmq.sptk.net`
- `xmq.sptk.net` → served directly

XMQ is the product the site leads with, so its hostname is the canonical one;
the `<link rel="canonical">` tags, the sitemap and the JSON-LD all name it. The
redirects have to stay permanently: links to sptk.net go back to 2007, and
removing them would break every one of those.

Two details that are easy to get wrong:

- **The redirects belong here, not in `public/.htaccess`.** Rules in `.htaccess`
  are re-evaluated after each internal rewrite, so a host redirect placed there
  fires on a second pass — after `/xmq_about` has become `/xmq_about.html` — and
  lands visitors on an address no canonical tag points at.
- **`/.well-known/` is exempt from the port 80 redirect.** Certbot renews the
  certificate over plain http through that path; redirecting it would break
  unattended renewals silently, two months later.

## Applying a change

`sites-enabled/` holds symlinks, so edit `sites-available/` — editing through
the symlink with `sed -i` would replace the link with a regular file:

```bash
scp sptk.conf sptk-le-ssl.conf theater:/tmp/
ssh theater '
  sudo cp -a /etc/apache2/sites-available/sptk.conf{,.bak-$(date +%F-%H%M%S)}
  sudo cp /tmp/sptk.conf /etc/apache2/sites-available/sptk.conf
  sudo apache2ctl configtest && sudo systemctl reload apache2'
```

Always run `configtest` before reloading.

## Certificate

One Let's Encrypt certificate named `sptk.net` covers all three hostnames,
managed by certbot with the apache authenticator and renewed by `certbot.timer`.
To add a hostname, put it in `ServerAlias` in **both** files and reload first —
the ACME check fails if Apache does not yet answer to the new name:

```bash
sudo certbot --apache --expand -d sptk.net -d www.sptk.net -d xmq.sptk.net
sudo certbot renew --dry-run
```

Anything missing from `ServerAlias` falls through to the default vhost on port
80, which is a different site entirely (`000-default.conf`).
