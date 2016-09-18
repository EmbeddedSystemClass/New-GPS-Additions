var map = L.map('map').setView([33.7674, -117.5008], 16);
L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 18,
}).addTo(map);

var cf = L.circle([33.7674, -117.5008], 1, {
    color: 'blue',
    fillColor: 'blue',
    fillOpacity: 1
}).addTo(map);

var accuracy = L.circle([33.7674, -117.5008], 0, {
    color: 'red',
    fillColor: '#f03',
    fillOpacity: 0.5
}).addTo(map);

if(typeof MainWindow != 'undefined') {
    var onMapMove = function() { MainWindow.onMapMove(map.getCenter().lat, map.getCenter().lng) };
    map.on('move', onMapMove);
    onMapMove();
}
