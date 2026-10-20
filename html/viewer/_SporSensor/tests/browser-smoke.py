#!/usr/bin/env python3
"""Check SporSensor controls with Chrome; optional live mode only reads config."""

import argparse
import errno
import functools
import http.server
import importlib.util
import json
from pathlib import Path
import threading
import time

ROOT = Path(__file__).resolve().parents[4]
spec = importlib.util.spec_from_file_location(
    "openkai_browser", ROOT / "html/console/editor/tests/browser-smoke.py")
browser_tools = importlib.util.module_from_spec(spec)
spec.loader.exec_module(browser_tools)


class Browser(browser_tools.Browser):
    def __exit__(self, *args):
        try:
            super().__exit__(*args)
        except OSError as error:
            if error.errno != errno.ENOTEMPTY:
                raise
            for attempt in range(20):
                time.sleep(0.1)
                try:
                    self.directory.cleanup()
                    break
                except OSError as retry:
                    if retry.errno != errno.ENOTEMPTY or attempt == 19:
                        raise


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


def navigate(browser, url):
    browser.call("Network.emulateNetworkConditions", offline=False, latency=0,
                 downloadThroughput=-1, uploadThroughput=-1)
    browser.call("Page.navigate", url=url.split("#", 1)[0])
    browser_tools.wait_for(browser, "document.readyState === 'complete' && !!document.querySelector('#getConfig')?.onclick")


def check_mock(browser):
    return browser.evaluate(r"""(() => {
        const assert = (ok, message) => { if (!ok) throw Error(message); };
        const $ = selector => document.querySelector(selector);
        const field = key => document.getElementById('param-' + key);
        const sent = [];
        window.wsSocket = {readyState: WebSocket.OPEN};
        window.wsSendCmd = command => { sent.push(command); return true; };
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connect must request config');
        const config = {address:'192.168.80.8', autoStart:true, laserPower:null, triggerMode:0,
            fallbackColor:[1,1,1], brightnessEnabled:null, frameRateHz:100,
            areaXRangeMm:null, areaZRangeMm:null, areaXStartMm:null, areaZStartMm:null};
        const schema = [
            {key:'address', type:'string', category:'Connection', restart:true},
            {key:'autoStart', type:'bool', category:'Acquisition'},
            {key:'laserPower', type:'float', category:'Laser', nullable:true, min:0, max:100, current:80},
            {key:'triggerMode', type:'int', category:'Acquisition', nullable:true,
                choices:[{value:0,label:'Continuous'}, {value:3,label:'Software'}]},
            {key:'fallbackColor', type:'array', category:'Point cloud'},
            {key:'brightnessEnabled', type:'bool', category:'Laser', nullable:true, current:false},
            {key:'frameRateHz', type:'float', category:'Acquisition', nullable:true, min:0.1},
            {key:'maximumFrameRate', type:'float', category:'Acquisition', readOnly:true, current:310},
            {key:'unsupported', type:'float', category:'Laser', supported:false},
            ...['areaXRangeMm','areaZRangeMm','areaXStartMm','areaZStartMm'].map(key => ({
                key,type:'float',category:'Sensor area',nullable:true,min:key.includes('Range') ? 0.000001 : -1e9})),
            {key:'address', type:'string', category:'Connection'}
        ];
        const response = extra => ({module:'sporSensor', cmd:sent.at(-1).cmd,
            requestId:sent.at(-1).requestId, bSuccess:true, deviceOpen:true, config, ...extra});
        const reply = extra => {
            const text = JSON.stringify(response(extra)) + 'EOJ';
            for (let i = 0; i < text.length; i += 7) cmdHandler({data:text.slice(i, i + 7)});
        };
        reply({schema, status:{connected:true, frameCount:123}, device:{note:'EOJ and braces { } inside a string'}});
        assert($('#cameraModule').value === 'sporSensor', 'Default module must match launch config');
        assert(!$('#cameraFields').disabled, 'Reply must unlock fields');
        assert(document.querySelectorAll('.control').length === schema.length - 1, 'Duplicate keys must not create controls');
        assert(field('laserPower').value === '' && field('brightnessEnabled').value === '', 'Null values must preserve sensor settings');
        assert(field('maximumFrameRate').value === '310' && field('maximumFrameRate').disabled, 'Read-only values must display disabled');
        assert(field('unsupported').disabled, 'Unavailable fields must be disabled');
        assert($('#sensorState').textContent.includes('123'), 'Status must display counters');
        assert($('#inspectionState').textContent.includes('EOJ'), 'Snapshot text must survive fragmented framing');
        const change = (key, value) => {
            const input = field(key); input.value = value; input.dispatchEvent(new Event('change'));
            return sent.at(-1);
        };
        let request = change('laserPower', '70');
        assert(request.config.laserPower === 70, 'Numeric settings must stay numeric');
        assert($('#cameraFields').disabled, 'Pending edit must lock fields');
        window.dispatchEvent(new CustomEvent('sporsensorcommand', {detail:response({requestId:'stale'})}));
        assert($('#cameraFields').disabled, 'Stale replies must not unlock fields');
        reply({bSuccess:false, errors:{laserPower:'Rejected by sensor'}});
        assert(field('laserPower').value === '', 'Rejected setting must restore accepted value');
        assert($('#configStatus').textContent.includes('Rejected by sensor'), 'Device errors must be visible');
        request = change('laserPower', '');
        assert(request.config.laserPower === null, 'Blank optional number must clear override');
        reply();
        request = change('brightnessEnabled', 'false');
        assert(request.config.brightnessEnabled === false, 'False must not become null');
        reply();
        request = change('brightnessEnabled', '');
        assert(request.config.brightnessEnabled === null, 'Preserve choice must send null');
        reply();
        request = change('triggerMode', '0');
        assert(request.config.triggerMode === 0, 'Zero enum value must remain numeric');
        reply();
        request = change('triggerMode', '3');
        assert(request.config.triggerMode === 3, 'Enum values must remain numeric');
        reply();
        const beforeBad = sent.length;
        change('fallbackColor', '{"x":1}');
        assert(sent.length === beforeBad && $('#configStatus').textContent.includes('JSON array'), 'Invalid array must not be sent');
        request = change('fallbackColor', '[0.5,0.5,1]');
        assert(JSON.stringify(request.config.fallbackColor) === '[0.5,0.5,1]', 'Array values must remain arrays');
        reply();
        const beforeArea = sent.length;
        change('areaXRangeMm', '200');
        assert(sent.length === beforeArea && $('#saveConfig').disabled, 'Area edits must stage and block saving unsubmitted values');
        $('#applyArea').click();
        assert(sent.length === beforeArea, 'Incomplete ROI must not be sent');
        for (const [key, value] of [['areaZRangeMm','150'],['areaXStartMm','-100'],['areaZStartMm','0']]) change(key,value);
        change('brightnessEnabled', 'true');
        reply({schema});
        assert(field('areaXRangeMm').value === '200' && field('areaZStartMm').value === '0', 'Other replies must retain staged ROI values');
        $('#applyArea').click();
        const area = {areaXRangeMm:200,areaZRangeMm:150,areaXStartMm:-100,areaZStartMm:0};
        assert(JSON.stringify(sent.at(-1).config) === JSON.stringify(area), 'ROI must send all four values together');
        Object.assign(config, area);
        reply();
        assert(!$('#saveConfig').disabled, 'Accepted ROI must permit saving');
        $('#preserveArea').click();
        assert(Object.keys(sent.at(-1).config).length === 4 && Object.values(sent.at(-1).config).every(value => value === null),
            'Preserve area must clear all four overrides together');
        Object.assign(config, sent.at(-1).config);
        reply();
        assert(field('areaXRangeMm').value === '' && field('areaZStartMm').value === '', 'Preserved ROI must display blank');
        $('#action-softwareTrigger').click();
        assert(sent.at(-1).cmd === 'softwareTrigger' && !('config' in sent.at(-1)), 'Trigger action must send the SDK command');
        reply();
        $('#modelGuid').value = 'DEA16047-ADEB-4D37-818B-6E4F8DF3ED17';
        $('#modelDeviceId').value = '10000845';
        $('#readModel').click();
        assert(sent.at(-1).cmd === 'readModel' && sent.at(-1).deviceId === '10000845' &&
            sent.at(-1).guid === $('#modelGuid').value, 'Read model must include its precise identifiers');
        reply({result:{laserPower:80}});
        assert($('#inspectionPanel').open && $('#inspectionState').textContent.includes('80'), 'Read model must show its result');
        $('#saveConfig').click();
        assert(sent.at(-1).cmd === 'saveConfig' && !('config' in sent.at(-1)), 'Save must not specify a path or include unsent values');
        reply();
        assert($('#configStatus').textContent === 'Configuration saved to disk', 'Save must be acknowledged');
        window.wsSocket = null;
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert($('#cameraFields').disabled && $('#sensorActions').disabled && $('#saveConfig').disabled, 'Disconnect must disable writes');
        return {controls:schema.length-1, commands:sent.length, checks:['page load','fragmented JSON',
            'nullable preservation','read-only fields','boolean false','enum zero','arrays','error rollback',
            'request correlation','atomic ROI apply/preserve','ROI draft retention','SDK actions','model inspection','save','disconnect']};
    })()""")


def check_live(browser):
    browser.evaluate("""window.liveReplies = [];
        window.addEventListener('sporsensorcommand', event => window.liveReplies.push(event.detail));
        document.querySelector('#cmdConnect').click();""")
    browser_tools.wait_for(browser, "window.liveReplies.some(reply => reply.cmd === 'getConfig')", timeout=120)
    result = browser.evaluate(r"""(() => {
        const assert = (ok, message) => { if (!ok) throw Error(message); };
        const reply = window.liveReplies.find(item => item.cmd === 'getConfig');
        assert(reply.bSuccess, 'getConfig failed: ' + JSON.stringify(reply.error || reply.errors));
        assert(reply.schema?.length && reply.config, 'Configuration and schema required');
        for (const spec of reply.schema) {
            const input = document.getElementById('param-' + spec.key);
            assert(input, 'Missing control: ' + spec.key);
            assert(input.disabled === (spec.supported === false || !!spec.readOnly || !!spec.readonly), 'Wrong writability: ' + spec.key);
            if (spec.nullable && reply.config[spec.key] == null) assert(input.value === '', 'Null override changed: ' + spec.key);
        }
        assert(!document.querySelector('#cameraFields').disabled, 'Connected controls must be unlocked');
        return {controls:reply.schema.length, deviceOpen:reply.deviceOpen, status:reply.status};
    })()""")
    browser.evaluate("document.querySelector('#getConfig').click()")
    browser_tools.wait_for(browser, "window.liveReplies.filter(reply => reply.cmd === 'getConfig').length === 2", timeout=120)
    browser.evaluate("document.querySelector('#cmdDisconnect').click()")
    browser_tools.wait_for(browser, "window.wsSocket === null && document.querySelector('#cameraFields').disabled")
    requests = [json.loads(event["params"]["response"]["payloadData"][:-3])
                for event in browser.events if event.get("method") == "Network.webSocketFrameSent"]
    assert len(requests) == 2 and all(request["cmd"] == "getConfig" for request in requests), requests
    result["checks"] = ["live connect", "all schema controls", "nullable values", "refresh", "disconnect", "read-only requests"]
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--live-url", help="Read configuration from a running backend without changing sensor settings")
    args = parser.parse_args()
    server = None
    if args.live_url:
        if not args.live_url.startswith(("http://", "https://")):
            parser.error("--live-url requires an HTTP(S) URL")
        url = args.live_url
    else:
        handler = functools.partial(QuietHandler, directory=str(ROOT / "html/viewer/_SporSensor"))
        server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
        worker = threading.Thread(target=server.serve_forever, daemon=True)
        worker.start()
        url = f"http://localhost:{server.server_port}/"
    try:
        with Browser() as browser:
            navigate(browser, url)
            result = check_live(browser) if args.live_url else check_mock(browser)
            errors = [event["params"] for event in browser.events if event.get("method") == "Runtime.exceptionThrown"]
            assert not errors, errors
            print(json.dumps(result, indent=2))
            print("PASS: SporSensor viewer controls")
    finally:
        if server:
            server.shutdown()
            server.server_close()
            worker.join()


if __name__ == "__main__":
    main()
