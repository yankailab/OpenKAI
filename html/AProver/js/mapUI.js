// Consecutive taps add boundary points; pinch and the zoom buttons handle zoom.
var map = L.map('map', { doubleClickZoom: false }).setView([36.7793686,138.4152144], 16);

// Panel expansion and responsive layout changes can resize the map without
// resizing the browser window.
const mapResizeObserver = new ResizeObserver(() => map.invalidateSize({ pan: false }));
mapResizeObserver.observe(document.getElementById('map'));

tlGoogleHybrid = L.tileLayer('http://{s}.google.com/vt/lyrs=s,h&x={x}&y={y}&z={z}',{
        maxZoom: 20,
        subdomains:['mt0','mt1','mt2','mt3']
});

// L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
//         maxZoom: 19,
//         attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
//     }).addTo(map);

/*
tlGoogleStreets = L.tileLayer('http://{s}.google.com/vt/lyrs=m&x={x}&y={y}&z={z}',{
        maxZoom: 20,
        subdomains:['mt0','mt1','mt2','mt3']
});

tlGoogleSat = L.tileLayer('http://{s}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}',{
        maxZoom: 20,
        subdomains:['mt0','mt1','mt2','mt3']
});

tlGoogleTerrain = L.tileLayer('http://{s}.google.com/vt/lyrs=p&x={x}&y={y}&z={z}',{
        maxZoom: 20,
        subdomains:['mt0','mt1','mt2','mt3']
});
*/

tlGoogleHybrid.addTo(map);

// L.marker([36.7795232705419, 138.52919832429615]).addTo(map)
//     .bindPopup('Home')
//     .openPopup();



