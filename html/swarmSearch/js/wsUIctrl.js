function $(selector) { return document.querySelector(selector);}

window.onload = function()
{
    wsInit();

  var strEOJ = "EOJ";
  
  $('#btnStandby').onclick = function(e)
  {
      var cmd = {
        cmd:'setState',
        s:'standby',
        id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };
  
  $('#btnTakeoff').onclick = function(e)
  {
      var cmd = {
          cmd:'setState',
          s:'takeoff',
          id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnAuto').onclick = function(e)
  {
      var cmd = {
          cmd:'setState',
          s:'auto',
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnRTL').onclick = function(e)
  {
      var cmd = {
          cmd:'setState',
          s:'rtl',
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

};

function cmdHandler(jCmd)
{
    if(jCmd.cmd == "hb")
    {
      $('#state').innerHTML = jCmd.s.toUpperCase();
    }
    else if(jCmd.cmd == "ndUpdate")
    {
        ndUpdate(jCmd);
    }
    else if(jCmd.cmd == "ndDelete")
    {
        ndDelete(jCmd);
    }
    else if(jCmd.cmd == "ndClearAll")
    {
        for(i=0; i<gvNodeMarkers.length; i++)
        {
            map.removeLayer(gvNodeMarkers[i].mk);
            gvNodeMarkers[i].mk = null;
        }

        gvNodeMarkers.splice(0);
    }
    else if(jCmd.cmd == "gcSet")
    {
    }
    else if(jCmd.cmd == "gcUpdate")
    {
    }
    else if(jCmd.cmd == "gcClear")
    {
    }
}

gvNodeMarkers = [];

function ndUpdate(jCmd)
{
    var i = getNodeIdx(jCmd.id);
    if(i < 0)
        i = ndAdd(jCmd);

    gvNodeMarkers[i].mk.setLatLng([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)]).update();
    gvNodeMarkers[i].mk.getPopup().setContent("ID:"+JSON.stringify(jCmd.id)
                                            + "</br>" + "LatLng: " + jCmd.lat + ", " + jCmd.lng
                                            + "</br>" + "Alt: " + jCmd.alt
                                            + "</br>" + "Hdg: " + jCmd.hdg
                                            + "</br>" + "Spd: " + jCmd.spd
                                            + "</br>" + "Mode: " + jCmd.mode
                                            + "</br>" + "Batt: " + jCmd.batt + "%"
                                             );
    gvNodeMarkers[i].mk.getPopup().update();
}

function ndAdd(jCmd)
{
    if(jCmd.id >= 400)
    {
        gvNodeMarkers.push(
            {
                id: jCmd.id,
                mk: new L.marker([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)], {icon: L.spriteIcon('yellow')}).addTo(map).bindPopup('ID:'+JSON.stringify(jCmd.id))
            }
        );    
    }
    else if(jCmd.id >= 300)
    {
        gvNodeMarkers.push(
            {
                id: jCmd.id,
                mk: new L.marker([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)], {icon: L.spriteIcon('red')}).addTo(map).bindPopup('ID:'+JSON.stringify(jCmd.id))
            }
        );    
    }
    else
    {
        gvNodeMarkers.push(
            {
                id: jCmd.id,
                mk: new L.marker([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)]).addTo(map).bindPopup('ID:'+JSON.stringify(jCmd.id))
            }
        );    
    }

    return gvNodeMarkers.length-1;
}

function ndDelete(jCmd)
{
    var i = getNodeIdx(jCmd.id);
    if(i < 0)
        return;

    map.removeLayer(gvNodeMarkers[i].mk);
    gvNodeMarkers[i].mk = null;
    gvNodeMarkers.splice(i, 1);
}

function getNodeIdx(id)
{
    for(i=0; i<gvNodeMarkers.length; i++)
    {
        if(gvNodeMarkers[i].id == id)
            return i;
    }

    return -1;
}
