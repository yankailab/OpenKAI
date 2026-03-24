# Install Nginx for web access and webSocket control

```bash
sudo apt-get update
sudo apt-get install -y nginx
systemctl status nginx --no-pager
```

Create your web folder and index.html

```bash
sudo mkdir -p /var/www/my_site
sudo tee /var/www/my_site/index.html > /dev/null <<'EOF'
<!doctype html>
<html>
  <head><meta charset="utf-8"><title>Hello</title></head>
  <body><h1>Nginx is serving this page ✅</h1></body>
</html>
EOF

sudo chown -R www-data:www-data /var/www/my_site
sudo chmod -R 755 /var/www/my_site
```

Configure an Nginx site for that folder

Create a site config:

```bash
sudo nano /etc/nginx/sites-available/my_site
```

Paste this:
```Nginx
server {
    listen 80;
    listen [::]:80;

    server_name _;

    root /var/www/my_site;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }
}
```

Enable it and reload Nginx:

```bash
sudo ln -s /etc/nginx/sites-available/my_site /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

(Optional) Disable the default site if you want:

```bash
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t
sudo systemctl reload nginx
```

Allow HTTP through the Ubuntu firewall (important)

If UFW is enabled:

```bash
sudo ufw status
```

If it says “active”, allow Nginx:

```bash
sudo ufw allow 'Nginx Full'
```

(Or minimum HTTP only:)

```bash
sudo ufw allow 80/tcp
```
