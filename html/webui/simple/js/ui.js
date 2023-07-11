function setHTML(id, h)
{
    document.getElementById(id).innerHTML = h;
}

function setWindowTitle(t)
{
    document.title = t;
}

// Save the file
function SaveFile()
{
    webui_fn('Save', content);
}

