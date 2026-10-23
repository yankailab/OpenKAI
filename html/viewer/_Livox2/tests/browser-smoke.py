#!/usr/bin/env python3
"""Test Livox2 controls and IMU preview with a simulated or running backend."""

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
    'openkai_browser', ROOT / 'html/console/editor/tests/browser-smoke.py')
browser_tools = importlib.util.module_from_spec(spec)
spec.loader.exec_module(browser_tools)


class Browser(browser_tools.Browser):
    def __exit__(self, *args):
        try:
            super().__exit__(*args)
        except OSError as error:
            if error.errno != errno.ENOTEMPTY:
                raise
            # Chrome's child processes can finish profile writes after the
            # parent exits. Retry only this temporary-directory cleanup race.
            for attempt in range(20):
                time.sleep(0.1)
                try:
                    self.directory.cleanup()
                    return
                except OSError as retry:
                    if retry.errno != errno.ENOTEMPTY or attempt == 19:
                        raise


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        local = Path(super().translate_path(path))
        try:
            shared = local.relative_to(Path(self.directory) / 'common')
        except ValueError:
            return str(local)
        return str(ROOT / 'html/common' / shared)

    def log_message(self, *_):
        pass


def check_mock_imu(browser):
    return browser.evaluate(r"""(async () => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const $ = selector => document.querySelector(selector);
        const delay = ms => new Promise(resolve => setTimeout(resolve, ms));
        const values = () => Array.from(document.querySelectorAll('#imuGraphs figcaption span:last-child'), el => el.textContent);
        const empty = () => values().every(value => value === '—') && !$('#angles').textContent.includes('°');
        const sent = [];
        const reply = message => {
            const text = JSON.stringify(message) + 'EOJ';
            for (let at = 0; at < text.length; at += 7) cmdHandler({data:text.slice(at, at + 7)});
        };
        assert($('#imuPanel') && document.querySelectorAll('#imuGraphs .graph canvas').length === 6,
            'IMU panel must have six gyro/accelerometer graphs');
        assert($('#orientation canvas'), 'IMU panel must include the orientation axes canvas');
        assert($('#imuStart').disabled && $('#imuStop').disabled, 'Disconnected IMU preview must be disabled');
        window.wsSocket = {readyState: WebSocket.OPEN};
        window.wsSendCmd = command => {
            sent.push(command);
            if (command.cmd === 'getConfig' && sent.length > 3) queueMicrotask(() => configReply(command));
            return true;
        };
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connecting must not automatically start IMU polling');
        const configReply = request => reply({...request, bSuccess:true, deviceOpen:true, config:{lvxIMUdataEn:1}});
        configReply(sent[0]);
        $('#imuStart').click();
        const first = sent.at(-1);
        assert(first.cmd === 'getIMU' && first.module === $('#lidarModule').value && /^imu-\d+$/.test(first.requestId),
            'Start must request IMU from the selected module with a separate requestId');
        assert(JSON.stringify(Object.keys(first).sort()) === '["cmd","module","requestId"]',
            'IMU Start must only read samples');
        assert($('#imuStart').disabled && !$('#imuStop').disabled, 'Start must enable Stop');
        await delay(250);
        assert(sent.at(-1) === first, 'IMU polling must keep at most one outstanding request');
        const epoch = 9007199254740993001n;
        const sample = (request, overrides = {}) => ({...request, bSuccess:true, enabled:true, previewEnabled:true, available:true,
            deviceOpen:true, tGyro:String(epoch), tAcc:String(epoch), tFusion:String(epoch),
            gyro:[0.1, -0.2, 0.3], acc:[1, 2, 9.7], quaternion:[1, 0, 0, 0], rpy:[0.1, 0.2, 0.3],
            orientationValid:true, ...overrides});
        reply(sample(first, {requestId:'imu-stale'}));
        reply(sample(first, {module:'DifferentCamera'}));
        assert(empty(), 'Mismatched replies must not update IMU graphs or orientation');
        $('#getConfig').click();
        const configuration = sent.at(-1);
        assert(configuration.cmd === 'getConfig', 'LiDAR controls must remain independent of IMU polling');
        reply(sample(first));
        assert($('#lidarFields').disabled, 'IMU reply must not complete a pending config request');
        configReply(configuration);
        assert(!$('#lidarFields').disabled, 'The config reply must still unlock LiDAR controls');
        assert(JSON.stringify(values()) === '["0.100","-0.200","0.300","1.000","2.000","9.700"]',
            'Each IMU channel must display its value and sign');
        assert($('#angles').textContent === 'Roll 5.7° · Pitch 11.5° · Yaw 17.2°',
            'Orientation angles must convert radians to degrees');
        assert($('#imuStatus').textContent.includes('fused orientation'), 'Fused orientation status must be visible');
        const nextRequest = async () => {
            const before = sent.filter(request => request.cmd === 'getIMU').length;
            for (let attempt = 0; attempt < 30; ++attempt) {
                await delay(20);
                if (sent.filter(request => request.cmd === 'getIMU').length > before) return sent.filter(request => request.cmd === 'getIMU').at(-1);
            }
            throw Error('IMU preview did not schedule its next request');
        };
        let disabledRequest = await nextRequest();
        reply(sample(disabledRequest, {previewEnabled:false, available:false}));
        assert(empty() && $('#imuStatus').textContent.includes('preview is disabled'),
            'Disabled backend preview must clear samples and explain the configuration switch');
        assert(!$('#imuStart').disabled && $('#imuStop').disabled,
            'Disabled backend preview must stop browser polling');
        const disabledCount = sent.filter(command => command.cmd === 'getIMU').length;
        await delay(250);
        assert(sent.filter(command => command.cmd === 'getIMU').length === disabledCount,
            'Disabled preview must not keep polling or automatically enable fusion');
        $('#imuStart').click();
        reply(sample(sent.at(-1)));
        let request = await nextRequest();
        reply(sample(request, {tGyro:String(epoch + 1n), tAcc:String(epoch + 1n), tFusion:String(epoch + 1n),
            gyro:[0.4, 0.5, 0.6], acc:[3, 4, 9.8]}));
        assert(values()[0] === '0.400' && values()[3] === '3.000',
            'Nanosecond timestamps above Number.MAX_SAFE_INTEGER must retain one-nanosecond advances');
        request = await nextRequest();
        reply(sample(request, {tGyro:String(epoch + 1n), tAcc:String(epoch + 1n), gyro:[9, 9, 9], acc:[9, 9, 9]}));
        assert(values()[0] === '0.400' && values()[3] === '3.000', 'Duplicate capture timestamps must not append new values');
        await delay(2200);
        assert(empty() && $('#imuStatus').textContent.includes('fresh IMU'), 'A stalled IMU must clear stale data');
        request = sent.filter(request => request.cmd === 'getIMU').at(-1);
        reply(sample(request, {tGyro:String(epoch + 1n), tAcc:String(epoch + 1n)}));
        assert(empty(), 'A duplicate sample must not revive stale graphs or orientation');
        request = await nextRequest();
        reply(sample(request, {available:false, enabled:true}));
        assert(empty() && $('#imuStatus').textContent.includes('fresh IMU'), 'Unavailable samples must clear stale graphs and orientation');
        request = await nextRequest();
        reply(sample(request, {available:false, enabled:false}));
        assert(empty() && $('#imuStatus').textContent.includes('disabled'), 'Disabled hardware IMU must be explained without enabling it');
        request = await nextRequest();
        reply(sample(request, {available:false, deviceOpen:false}));
        assert(empty() && $('#imuStatus').textContent.includes('not connected'), 'Camera closure must be reflected in the panel');
        request = await nextRequest();
        reply(sample(request, {orientationValid:false}));
        assert(values()[0] === '0.100' && !$('#angles').textContent.includes('°'),
            'Raw graphs must work before orientation fusion initializes');
        $('#imuStop').click();
        const stoppedCount = sent.length;
        await delay(250);
        assert(sent.length === stoppedCount && !$('#imuStart').disabled && $('#imuStop').disabled && empty(),
            'Stop must cancel polling and clear the preview without sending a command');
        $('#imuStart').click();
        const inFlight = sent.at(-1);
        assert(inFlight.cmd === 'getIMU' && inFlight.requestId !== first.requestId, 'Restart must use a fresh requestId');
        window.wsSocket = null;
        window.dispatchEvent(new Event('wscmdstatechange'));
        reply(sample(inFlight));
        const disconnectedCount = sent.length;
        await delay(250);
        assert(empty() && $('#imuStart').disabled && $('#imuStop').disabled && sent.length === disconnectedCount,
            'Disconnect must cancel polling and ignore an outstanding reply');
        assert(sent.every(command => command.cmd === 'getConfig' || command.cmd === 'getIMU'),
            'IMU preview must not change LiDAR configuration or streams');
        return {graphs:6, imuRequests:sent.filter(command => command.cmd === 'getIMU').length,
            checks:['fragmented IMU replies', 'one outstanding request', 'request/module correlation',
                'config request isolation', 'nanosecond precision', 'duplicate sample suppression', 'stale sample clearing',
                'unavailable/disabled/closed camera', 'orientation validity', 'Stop', 'disconnect']};
    })()""")


def check_controls(browser, captured=None):
    browser.evaluate('window.capturedLivoxResponse = ' + json.dumps(captured))
    return browser.evaluate(r"""(async () => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const $ = selector => document.querySelector(selector);
        const field = key => document.getElementById('param-' + key);
        const sent = [];
        window.wsSocket = {readyState: WebSocket.OPEN};
        window.wsSendCmd = command => { sent.push(command); return true; };
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connect must load config');
        assert($('#lidarModule').value === 'Lvx1', 'Default module must match launch configuration');
        let config = {lvxIP:'192.168.16.120', lvxPCLdataType:1, lvxIMUdataEn:1, bIMUpreview:true, bReflectivity:true,
            frameDurationMs:100, vRangeD:[0.1,40], vColorDefault:[1,1,1]};
        const schema = {
            lvxIP:{type:'string',readOnly:true},
            lvxPCLdataType:{type:'integer', choices:[{value:1,label:'Cartesian high'},{value:2,label:'Cartesian low'},{value:3,label:'Spherical'}]},
            lvxIMUdataEn:{type:'integer',choices:[{value:0,label:'Off'},{value:1,label:'On'}]},
            bIMUpreview:{type:'bool',label:'Enable IMU preview'},
            bReflectivity:{type:'bool'}, frameDurationMs:{type:'integer',min:20,max:1000},
            vRangeD:{type:'range',length:2,min:0,max:10000}, vColorDefault:{type:'array',length:3,min:0,max:1},
            unsupported:{type:'int',supported:false}
        };
        const reply = (extra = {}, request = sent.at(-1)) => {
            const text = JSON.stringify({...request, bSuccess:true, deviceOpen:true, config,
                status:{state:'work',configApplied:true,pendingCommands:0,pointPackets:123,imuSamples:42}, ...extra}) + 'EOJ';
            for (let at=0; at<text.length; at+=7) cmdHandler({data:text.slice(at,at+7)});
        };
        reply({schema});
        assert(!$('#lidarFields').disabled, 'Reply must unlock controls');
        assert(document.querySelectorAll('.control').length === Object.keys(schema).length, 'Schema object must render all controls');
        assert(field('lvxIP').disabled && field('lvxIP').value === config.lvxIP, 'Network field must stay read-only');
        assert(field('unsupported').disabled, 'Unsupported controls must remain disabled');
        assert($('#deviceStatus').textContent.includes('123'), 'Packet counters must be visible');
        const change = (key,value) => { const input=field(key); input.value=value; input.dispatchEvent(new Event('change')); return sent.at(-1); };
        let request=change('lvxIMUdataEn','0');
        assert(request.config.lvxIMUdataEn === 0, 'IMU enum must be numeric');
        assert($('#lidarFields').disabled, 'Pending write must lock settings');
        reply({requestId:'stale'});
        reply({module:'OtherLiDAR'});
        reply({cmd:'getIMU'});
        assert($('#lidarFields').disabled, 'Unrelated replies must not unlock settings');
        reply({bSuccess:false,errors:{lvxIMUdataEn:'Sensor rejected setting'}});
        assert(field('lvxIMUdataEn').value === '1', 'Rejected setting must restore accepted value');
        assert($('#configStatus').textContent.includes('Sensor rejected setting'), 'Device error must be visible');
        request=change('bReflectivity','false');
        assert(request.config.bReflectivity === false, 'Boolean controls must remain boolean'); reply();
        let count=sent.length;
        change('frameDurationMs','0'); change('frameDurationMs','30.5');
        assert(sent.length === count, 'Numeric bounds and integer validation must reject invalid frame durations');
        request=change('frameDurationMs','50'); assert(request.config.frameDurationMs === 50, 'Valid frame duration must be sent'); reply();
        for (const [key,value] of [['vRangeD','[40,1]'],['vRangeD','[0,"40"]'],['vColorDefault','[1,0]'],['vColorDefault','[1,2,0]']]) {
            count=sent.length; change(key,value); assert(sent.length === count, 'Invalid vector must not be sent: '+key+' '+value);
        }
        request=change('vColorDefault','[1,0.5,0]');
        assert(JSON.stringify(request.config.vColorDefault) === '[1,0.5,0]', 'Color must preserve numeric vector'); reply();
        request=change('lvxPCLdataType','2'); config.lvxPCLdataType=2;
        reply({status:{state:'work',pendingCommands:1,configApplied:false}});
        assert($('#lidarFields').disabled && $('#saveConfig').disabled, 'Queued sensor writes must block further writes');
        assert($('#configStatus').textContent.includes('waiting for sensor acknowledgement'), 'Queued write must be distinguished from ACK');
        await new Promise(resolve => setTimeout(resolve,650));
        assert(sent.at(-1).cmd === 'getConfig', 'Queued write must poll device status');
        reply(); assert(!$('#lidarFields').disabled, 'Sensor ACK must unlock controls');
        request=change('bIMUpreview','false');
        assert(request.config.bIMUpreview===false,'Preview must be a persisted boolean independent of IMU capture'); reply();
        $('#saveConfig').click();
        assert(sent.at(-1).cmd === 'saveConfig' && !('config' in sent.at(-1)) && !('path' in sent.at(-1)), 'Save must use original launch file');
        reply(); assert($('#configStatus').textContent.includes('saved to disk'), 'Save success must be visible');
        const arraySchema=Object.entries(schema).map(([key,spec])=>({key,...spec}));
        $('#getConfig').click(); reply({schema:arraySchema});
        assert(document.querySelectorAll('.control').length === arraySchema.length, 'Array schema must also render');
        let capturedControls=0;
        if (window.capturedLivoxResponse) {
            $('#getConfig').click(); const source=window.capturedLivoxResponse;
            reply({...source,module:'Lvx1',cmd:'getConfig',requestId:sent.at(-1).requestId});
            const specs=Array.isArray(source.schema)?source.schema:Object.entries(source.schema).map(([key,spec])=>({key,...spec}));
            for (const spec of specs) {
                const input=field(spec.key); assert(input,'Missing live field '+spec.key);
                assert(input.disabled === (!!spec.readOnly || spec.supported===false),'Wrong live field writability '+spec.key);
                ++capturedControls;
            }
        }
        window.wsSocket=null; window.dispatchEvent(new Event('wscmdstatechange'));
        assert($('#lidarFields').disabled && $('#saveConfig').disabled, 'Disconnect must prevent settings writes');
        return {controls:Object.keys(schema).length,capturedControls,commands:sent.length,
            checks:['fragmented replies','schema object and array','read-only fields','numeric enum and bool types',
                'range/color validation','request/module/command correlation','rejected edit rollback','ACK polling','save','disconnect']};
    })()""")


def screenshot(browser, path):
    if path:
        import base64
        path.write_bytes(base64.b64decode(browser.call('Page.captureScreenshot', format='png')['data']))


def run_live(browser, url, screenshot_path=None):
    browser.call('Page.navigate', url=url.split('#', 1)[0])
    browser_tools.wait_for(browser, "!!document.querySelector('#getConfig')?.onclick")
    browser.evaluate(r"""(() => {
        window.liveConfigs=[]; window.liveIMU=[];
        window.addEventListener('livoxcommand', event => {
            if(event.detail.cmd === 'getConfig') window.liveConfigs.push(event.detail);
            if(event.detail.cmd === 'getIMU') window.liveIMU.push(event.detail);
        });
        document.querySelector('#cmdConnect').click();
    })()""")
    try:
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r => r.bSuccess && r.deviceOpen)', timeout=20)
        browser.evaluate("document.querySelector('#imuStart').click()")
        browser_tools.wait_for(browser, 'window.liveIMU.filter(r => r.available && r.orientationValid).length >= 3', timeout=20)
        result = browser.evaluate(r"""(() => {
            const assert=(c,m)=>{if(!c)throw Error(m)};
            const config=window.liveConfigs.at(-1), imu=window.liveIMU.at(-1);
            const specs=Array.isArray(config.schema)?config.schema:Object.entries(config.schema).map(([key,spec])=>({key,...spec}));
            for(const spec of specs) {
                const field=document.getElementById('param-'+spec.key);
                assert(field,'Missing live field '+spec.key);
                assert(field.disabled === (!!spec.readOnly || spec.supported===false),'Wrong field writability '+spec.key);
            }
            assert(config.config.lvxIMUdataEn === 1,'Embedded IMU must be enabled');
            assert(imu.enabled && imu.deviceOpen && imu.available,'IMU capture must be available');
            assert(Math.abs(Math.hypot(...imu.quaternion)-1)<0.001,'Quaternion must be normalized');
            for(const key of ['tAcc','tGyro','tFusion']) assert(typeof imu[key]==='string' && BigInt(imu[key])>0n,'IMU timestamps must be ns strings');
            assert(new Set(window.liveIMU.map(i=>i.tGyro)).size>1,'IMU timestamps must advance');
            const values=Array.from(document.querySelectorAll('#imuGraphs figcaption span:last-child'),el=>el.textContent);
            assert(values.length===6 && values.every(v=>v!=='—' && Number.isFinite(Number(v))),'All IMU graphs must show samples');
            assert(document.querySelector('#angles').textContent.includes('°'),'Orientation must display');
            document.querySelector('#imuStop').click();
            return {controls:specs.length,status:config.status,imu};
        })()""")
        browser.evaluate("document.querySelector('#cmdDisconnect').click()")
        browser_tools.wait_for(browser, "window.wsSocket === null && document.querySelector('#lidarFields').disabled")
        browser.evaluate("window.liveConfigs=[];document.querySelector('#cmdConnect').click()")
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r => r.bSuccess)', timeout=20)
        sent = [event['params']['response']['payloadData'] for event in browser.events
                if event.get('method') == 'Network.webSocketFrameSent']
        assert sent and all(text.endswith('EOJ') for text in sent), 'Command frames must end with EOJ'
        assert all(json.loads(text[:-3])['cmd'] in ('getConfig', 'getIMU') for text in sent), 'Live test must only read'
        result['commands'] = len(sent)
        browser.evaluate("document.querySelector('#start').click();document.querySelector('#imuStart').click()")
        browser_tools.wait_for(browser, "document.querySelector('#status').textContent === 'Connected' && parseInt(document.querySelector('#stats').textContent.replaceAll(',','')) > 0", timeout=20)
        browser_tools.wait_for(browser, "document.querySelector('#imuStatus').textContent.includes('fused orientation')", timeout=20)
        result['geometry'] = browser.evaluate("document.querySelector('#stats').textContent")
        stream_urls = [event['params']['url'] for event in browser.events
                       if event.get('method') == 'Network.webSocketCreated' and '/stream/' in event['params']['url']]
        assert any(url.endswith('/stream/points') for url in stream_urls), 'Point stream must use the shared geometry endpoint'
        assert any(url.endswith('/stream/lines') for url in stream_urls), 'Line stream must use the shared geometry endpoint'
        result['streams'] = stream_urls
        screenshot(browser, screenshot_path)
        return result
    finally:
        browser.evaluate("document.querySelector('#stop').click();if (window.wsSocket) wsStop()")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--response', type=Path, help='Replay a captured getConfig reply')
    mode.add_argument('--live-url', help='Read configuration and IMU from a running Livox2 viewer')
    parser.add_argument('--screenshot', type=Path, help='Save a desktop preview screenshot')
    args = parser.parse_args()
    if args.live_url and not args.live_url.startswith(('http://', 'https://')):
        parser.error('--live-url requires an HTTP or HTTPS URL')
    handler = functools.partial(QuietHandler, directory=str(ROOT / 'html/viewer/_Livox2'))
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
    worker = threading.Thread(target=server.serve_forever, daemon=True)
    worker.start()
    try:
        with Browser() as browser:
            browser.call('Network.emulateNetworkConditions', offline=False, latency=0,
                         downloadThroughput=-1, uploadThroughput=-1)
            if args.live_url:
                result = run_live(browser, args.live_url, args.screenshot)
            else:
                browser.call('Page.navigate', url=f'http://localhost:{server.server_port}/')
                browser_tools.wait_for(browser, "document.readyState === 'complete' && !document.querySelector('#start').disabled")
                result = check_controls(browser, json.loads(args.response.read_text()) if args.response else None)
                result['imu'] = check_mock_imu(browser)
            if not args.live_url:
                screenshot(browser, args.screenshot)
            browser.call('Emulation.setDeviceMetricsOverride', width=390, height=844, deviceScaleFactor=1, mobile=True)
            browser.evaluate('new Promise(resolve => setTimeout(resolve,100))')
            assert browser.evaluate('document.documentElement.scrollWidth <= innerWidth'), 'Mobile layout must not overflow horizontally'
            errors = [event['params'] for event in browser.events if event.get('method') == 'Runtime.exceptionThrown']
            assert not errors, errors
            print(json.dumps(result, indent=2))
            print('PASS: Livox2 browser controls, IMU preview, and responsive layout')
    finally:
        server.shutdown()
        server.server_close()
        worker.join()


if __name__ == '__main__':
    main()


