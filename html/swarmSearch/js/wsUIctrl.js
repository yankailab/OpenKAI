window.onload = function()
{
    function $(selector) { return document.querySelector(selector);}

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
    if(jCmd.cmd == "setState")
    {
      $('#state').innerHTML = JSON.stringify(jCmd.s);
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
    nIdx = getNodeIdx(jCmd.id);

    if(nIdx < 0)
        nIdx = ndAdd(jCmd);

    gvNodeMarkers[nIdx].mk.setLatLng([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)]).update();
    gvNodeMarkers[nIdx].mk.getPopup().setContent("123");
    gvNodeMarkers[nIdx].mk.getPopup().update();
}

function ndAdd(jCmd)
{
    gvNodeMarkers.push(
        {
            id: jCmd.id,
            mk: new L.marker([JSON.parse(jCmd.lat), JSON.parse(jCmd.lng)]).addTo(map).bindPopup('ID:'+JSON.stringify(jCmd.id))
        }
    );

    return gvNodeMarkers.length-1;
}

function ndDelete(jCmd)
{
    nIdx = getNodeIdx(jCmd.id);
    if(nIdx < 0)
        return;

    map.removeLayer(gvNodeMarkers[nIdx].marker);
    gvNodeMarkers[nIdx].marker = null;
    gvNodeMarkers.splice(nIdx, 1);
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