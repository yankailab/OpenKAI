# Install Nginx for web access and webSocket control

```bash
sudo apt-get update
sudo apt-get install -y nginx
systemctl status nginx --no-pager
```

Create your web folder and index.html

```bash
sudo mkdir -p /var/www/OpenKAI

sudo tee /var/www/OpenKAI/index.html > /dev/null <<'EOF'
<!doctype html>
<html>
  <head><meta charset="utf-8"><title>OpenKAI web console setup</title></head>
  <body><h1>Nginx is serving this page ✅</h1></body>
</html>
EOF

sudo chown -R www-data:www-data /var/www/OpenKAI
sudo chmod -R 755 /var/www/OpenKAI
```

Configure an Nginx site for that folder
Create a site config:

```bash
sudo nano /etc/nginx/sites-available/OpenKAI
```

Paste this:
```Nginx
server {
    listen 80;
    listen [::]:80;

    server_name _;

    root /var/www/OpenKAI;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }
}
```

Enable it and reload Nginx:
```bash
sudo ln -s /etc/nginx/sites-available/OpenKAI /etc/nginx/sites-enabled/
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

Open from browser to confirm NginX is serving the html correctly.
Ctrl + F5 for refresh caching.
