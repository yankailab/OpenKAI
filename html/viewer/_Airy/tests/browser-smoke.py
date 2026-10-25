#!/usr/bin/env python3
"""Test Airy controls and IMU preview with a simulated or running backend."""

import argparse
import errno
import functools
import http.server
import importlib.util
import json
from pathlib import Path
import threading
import time
import urllib.parse

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
        const configReply = request => reply({...request, bSuccess:true, deviceOpen:true, config:{bIMUoutput:true}});
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
        reply(sample(first, {module:'DifferentLiDAR'}));
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
        assert(empty() && $('#imuStatus').textContent.length>0, 'Unavailable samples must clear the preview without enabling publication');
        request = await nextRequest();
        reply(sample(request, {available:false, deviceOpen:false}));
        assert(empty() && $('#imuStatus').textContent.includes('not connected'), 'LiDAR closure must be reflected in the panel');
        request = await nextRequest();
        reply(sample(request, {orientationValid:false}));
        assert(values()[0] === '0.100' && !$('#angles').textContent.includes('°'),
            'Raw graphs must work before orientation fusion initializes');
        request = await nextRequest();
        reply(sample(request, {enabled:false, tGyro:String(epoch+2n), tAcc:String(epoch+2n)}));
        assert(values()[0]==='0.100' && $('#angles').textContent.includes('°'),
            'Disabling OpenKAI IMU publication must not hide samples still received by the sensor');
        request = await nextRequest();
        reply(sample(request, {tGyro:String(epoch+3n), tAcc:String(epoch+3n), quaternion:[0,0,0,0]}));
        assert(values()[0]==='0.100' && !$('#angles').textContent.includes('°'),
            'Invalid orientation must clear angles while retaining valid raw values');
        request = await nextRequest();
        reply(sample(request, {tGyro:String(epoch+4n), tAcc:String(epoch+4n), gyro:[null,0,0]}));
        assert(empty() && $('#imuStatus').textContent.includes('Invalid'), 'Malformed IMU vectors must not render');
        $('#imuStop').click();
        const stoppedCount = sent.filter(command=>command.cmd==='getIMU').length;
        await delay(250);
        assert(sent.filter(command=>command.cmd==='getIMU').length === stoppedCount && !$('#imuStart').disabled && $('#imuStop').disabled && empty(),
            'Stop must cancel polling and clear the preview without sending a command');
        $('#imuStart').click();
        const inFlight = sent.at(-1);
        assert(inFlight.cmd === 'getIMU' && inFlight.requestId !== first.requestId, 'Restart must use a fresh requestId');
        $('#lidarModule').value='OtherAiry'; $('#lidarModule').dispatchEvent(new Event('change'));
        reply(sample(inFlight));
        assert(empty() && !$('#imuStart').disabled && $('#imuStop').disabled,
            'Switching the selected module must stop the preview and reject its previous reply');
        $('#imuStart').click();
        const otherFlight=sent.at(-1);
        assert(otherFlight.cmd==='getIMU' && otherFlight.module==='OtherAiry', 'Restart must use the newly selected module');
        window.wsSocket = null;
        window.dispatchEvent(new Event('wscmdstatechange'));
        reply(sample(otherFlight));
        const disconnectedCount = sent.length;
        await delay(250);
        assert(empty() && $('#imuStart').disabled && $('#imuStop').disabled && sent.length === disconnectedCount,
            'Disconnect must cancel polling and ignore an outstanding reply');
        assert(sent.every(command => command.cmd === 'getConfig' || command.cmd === 'getIMU'),
            'IMU preview must not change LiDAR configuration or streams');
        $('#lidarModule').value='Airy'; $('#lidarModule').dispatchEvent(new Event('change'));
        return {graphs:6, imuRequests:sent.filter(command => command.cmd === 'getIMU').length,
            checks:['fragmented IMU replies', 'one outstanding request', 'request/module correlation',
                'config request isolation', 'nanosecond precision', 'duplicate sample suppression', 'stale sample clearing',
                'unavailable/disabled/closed sensor', 'orientation validity', 'malformed sample', 'Stop', 'module switch', 'disconnect']};
    })()""")


def check_controls(browser, captured=None):
    browser.evaluate('window.capturedResponse = ' + json.dumps(captured))
    return browser.evaluate(r"""(async () => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const $ = selector => document.querySelector(selector);
        const field = key => document.getElementById('param-' + key);
        const sent = [];
        window.wsSocket = {readyState: WebSocket.OPEN};
        window.wsSendCmd = command => { sent.push(command); return true; };
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connect must load config');
        assert($('#lidarModule').value === 'Airy', 'Default module must match launch configuration');
        let config = {deviceIP:'192.168.1.200', msopPort:6699, imuPort:6688,
            bIMUoutput:true, bIMUpreview:true, bReflectivity:true, frameTimeoutMs:250,
            vRangeD:[0.1,60], vColorDefault:[1,1,1]};
        const schema = {
            deviceIP:{type:'string',category:'Receiver'}, msopPort:{type:'integer',min:1,max:65535},
            imuPort:{type:'integer',min:1,max:65535}, bIMUoutput:{type:'bool',category:'IMU'},
            bIMUpreview:{type:'bool',label:'Enable IMU preview'},
            bReflectivity:{type:'bool'}, frameTimeoutMs:{type:'integer',min:10,max:10000},
            vRangeD:{type:'range',length:2,min:0,max:81.915}, vColorDefault:{type:'array',length:3,min:0,max:1},
            deviceReturnMode:{type:'integer',readOnly:true,category:'Device parameters',
                choices:[{value:0,label:'Strongest'}]},
            unavailable:{type:'integer',supported:false}
        };
        const reply = (extra = {}, request = sent.at(-1)) => {
            const text = JSON.stringify({...request, bSuccess:true, deviceOpen:true, config,
                status:{state:'streaming', pointPackets:123, difopPackets:7, imuPackets:140,
                    calibrationReady:true, observed:{deviceReturnMode:0,sensorIp:'192.168.1.200'},
                    imu:{available:true,gyro:[0.1,-0.2,0.3],acc:[1,2,9.7],outputRate:3,
                        timestampNs:'9007199254740993001',temperatureRaw:42}}, ...extra}) + 'EOJ';
            for (let at=0; at<text.length; at+=7) cmdHandler({data:text.slice(at,at+7)});
        };
        reply({schema});
        for (const key of ['vPos','vOrt','vT','vR']) assert(!field(key), 'Driver must not expose pose controls: '+key);
        assert(!$('#lidarFields').disabled, 'Reply must unlock controls');
        assert(document.querySelectorAll('.control').length === Object.keys(schema).length, 'Every schema field must render');
        assert(!field('deviceIP').disabled, 'Host source filter must be editable');
        assert(field('deviceReturnMode').disabled && field('deviceReturnMode').value === '0', 'Device readback must be read-only');
        assert(field('unavailable').disabled, 'Unsupported controls must remain disabled');
        assert($('#deviceStatus').textContent.includes('123'), 'Packet counters must be visible');
        assert($('#imuTelemetry').textContent.includes('[0.1,-0.2,0.3]') &&
            $('#imuTelemetry').textContent.includes('9007199254740993001'), 'IMU values and precise timestamp must be visible');
        assert($('#sensorSettings').href==='http://192.168.1.200/', 'Hardware settings must link to observed sensor');
        const change = (key,value) => { const input=field(key); input.value=value; input.dispatchEvent(new Event('change')); return sent.at(-1); };
        let count=sent.length;
        change('deviceReturnMode','0'); assert(sent.length===count,'Read-only fields must never send writes');
        let request=change('bReflectivity','false');
        assert(request.config.bReflectivity === false, 'Boolean controls must remain boolean');
        assert($('#lidarFields').disabled, 'Pending write must lock settings');
        reply({requestId:'stale'}); reply({module:'OtherLiDAR'}); reply({cmd:'status'});
        assert($('#lidarFields').disabled, 'Unrelated replies must not unlock settings');
        reply({bSuccess:false,errors:{bReflectivity:'Setting rejected'}});
        assert(field('bReflectivity').value === 'true', 'Rejected edit must restore accepted value');
        assert($('#configStatus').textContent.includes('Setting rejected'), 'Errors must be visible');
        count=sent.length;
        change('frameTimeoutMs','0'); change('frameTimeoutMs','30.5'); change('msopPort','65536');
        assert(sent.length === count, 'Numeric bounds and integer validation must reject invalid inputs');
        request=change('frameTimeoutMs','500'); assert(request.config.frameTimeoutMs===500,'Valid number must be sent'); reply();
        for (const [key,value] of [['vRangeD','[40,1]'],['vRangeD','[0,"40"]'],['vColorDefault','[1,0]'],
                                  ['vColorDefault','[1,2,0]']]) {
            count=sent.length; change(key,value); assert(sent.length===count,'Invalid vector must not be sent: '+key+' '+value);
        }
        request=change('vColorDefault','[1,0.5,0]');
        assert(JSON.stringify(request.config.vColorDefault)==='[1,0.5,0]','Color must preserve numeric vector'); reply();
        request=change('deviceIP','0.0.0.0');
        assert(request.config.deviceIP==='0.0.0.0','Source filter must retain string type'); reply();
        request=change('bIMUoutput','false');
        assert(request.config.bIMUoutput===false,'IMU publication must be a local boolean setting'); reply();
        request=change('bIMUpreview','false');
        assert(request.config.bIMUpreview===false,'Preview must be a persisted boolean independent of IMU capture'); reply();
        $('#saveConfig').click();
        assert(sent.at(-1).cmd==='saveConfig' && !('config' in sent.at(-1)) && !('path' in sent.at(-1)), 'Save must use launch file');
        reply(); assert($('#configStatus').textContent.includes('saved to disk'), 'Save success must be visible');
        const arraySchema=Object.entries(schema).map(([key,spec])=>({key,...spec}));
        $('#getConfig').click(); reply({schema:arraySchema});
        assert(document.querySelectorAll('.control').length===arraySchema.length,'Array schema must also render');
        await new Promise(resolve=>setTimeout(resolve,2150));
        assert(sent.at(-1).cmd==='getConfig','Device status must refresh automatically'); reply();
        $('#getConfig').click(); reply({deviceOpen:false,status:{state:'waiting',pointPackets:0}});
        assert(!$('#lidarFields').disabled && $('#configStatus').textContent.includes('not connected'),
            'Local receiver settings must remain editable without sensor packets');
        assert(!$('#imuTelemetry').textContent.includes('9.7'), 'Missing IMU telemetry must clear stale values');
        $('#getConfig').click(); reply({status:{observed:{sensorIp:'not-an-ip'},imu:{available:false,gyro:[9,9,9]}}});
        assert(!$('#sensorSettings').hasAttribute('href'), 'Invalid sensor addresses must not become links');
        assert(!$('#imuTelemetry').textContent.includes('[9,9,9]'), 'Unavailable IMU must not show stale vectors');
        let capturedControls=0;
        if (window.capturedResponse) {
            $('#getConfig').click(); const source=window.capturedResponse;
            reply({...source,module:'Airy',cmd:'getConfig',requestId:sent.at(-1).requestId});
            const specs=Array.isArray(source.schema)?source.schema:Object.entries(source.schema).map(([key,spec])=>({key,...spec}));
            for (const key of ['vPos','vOrt','vT','vR']) {
                assert(!specs.some(spec=>spec.key===key) && !(key in source.config) && !field(key),
                    'Driver must not expose pose controls: '+key);
            }
            for (const spec of specs) {
                const input=field(spec.key); assert(input,'Missing backend field '+spec.key);
                assert(input.disabled===(!!spec.readOnly || spec.supported===false),'Wrong writability '+spec.key);
                ++capturedControls;
            }
        }
        window.wsSocket=null; window.dispatchEvent(new Event('wscmdstatechange'));
        assert($('#lidarFields').disabled && $('#saveConfig').disabled,'Disconnect must prevent writes');
        return {controls:Object.keys(schema).length,capturedControls,commands:sent.length,
            checks:['fragmented replies','all schema fields','read-only device parameters','typed values',
                'bounds/vector validation','reply correlation','rejected edit rollback','save','status polling',
                'hardware settings link','IMU publication and readback','disconnect']};
    })()""")


def run_live(browser, url, screenshot_path=None):
    # The shared offline browser harness permits localhost and blocks every
    # other host, including numeric loopback addresses, in Chrome's resolver.
    endpoint = urllib.parse.urlsplit(url)
    if endpoint.hostname == '127.0.0.1':
        endpoint = endpoint._replace(netloc='localhost' + (f':{endpoint.port}' if endpoint.port else ''))
        url = urllib.parse.urlunsplit(endpoint)
    browser.call('Page.navigate', url=url.split('#', 1)[0])
    browser_tools.wait_for(browser, "!!document.querySelector('#getConfig')?.onclick")
    browser.evaluate(r"""(() => {
        window.liveConfigs=[]; window.liveIMU=[];
        window.addEventListener('airycommand', event => {
            if(event.detail.cmd==='getConfig') window.liveConfigs.push(event.detail);
            if(event.detail.cmd==='getIMU') window.liveIMU.push(event.detail);
        });
        document.querySelector('#cmdConnect').click();
    })()""")
    try:
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r=>r.bSuccess)', timeout=20)
        result = browser.evaluate(r"""(() => {
            const config=window.liveConfigs.at(-1);
            const specs=Array.isArray(config.schema)?config.schema:Object.entries(config.schema).map(([key,spec])=>({key,...spec}));
            for(const key of ['vPos','vOrt','vT','vR']) {
                if(specs.some(spec=>spec.key===key) || key in config.config || document.getElementById('param-'+key))
                    throw Error('Driver must not expose pose controls: '+key);
            }
            for(const spec of specs) {
                const field=document.getElementById('param-'+spec.key);
                if(!field) throw Error('Missing field '+spec.key);
                if(field.disabled !== (!!spec.readOnly || spec.supported===false)) throw Error('Wrong writability '+spec.key);
            }
            return {controls:specs.length,deviceOpen:config.deviceOpen,status:config.status};
        })()""")
        browser.evaluate("document.querySelector('#start').click()")
        browser_tools.wait_for(browser, "document.querySelector('#status').textContent==='Connected'", timeout=20)
        if result['deviceOpen']:
            browser_tools.wait_for(browser, "parseInt(document.querySelector('#stats').textContent.replaceAll(',',''))>0", timeout=20)
        result['geometry'] = browser.evaluate("document.querySelector('#stats').textContent")
        browser.evaluate("document.querySelector('#imuStart').click()")
        browser_tools.wait_for(browser, 'window.liveIMU.filter(r=>r.available && r.orientationValid).length>=3', timeout=20)
        result['imu'] = browser.evaluate(r"""(() => {
            const imu=window.liveIMU.at(-1);
            for(const key of ['tAcc','tGyro']) {
                if(typeof imu[key]!=='string' || BigInt(imu[key])<=0n) throw Error('IMU timestamp must be a positive ns string');
            }
            if(Math.abs(Math.hypot(...imu.quaternion)-1)>0.001) throw Error('Orientation quaternion must be normalized');
            const values=Array.from(document.querySelectorAll('#imuGraphs figcaption span:last-child'),el=>el.textContent);
            if(values.length!==6 || values.some(value=>value==='—' || !Number.isFinite(Number(value))))
                throw Error('All six IMU channels must show finite samples');
            if(!document.querySelector('#angles').textContent.includes('°')) throw Error('Orientation angles must display');
            if(new Set(window.liveIMU.map(sample=>sample.tGyro)).size<2) throw Error('IMU timestamps must advance');
            return imu;
        })()""")
        browser.evaluate("document.querySelector('#cmdDisconnect').click()")
        browser_tools.wait_for(browser, "window.wsSocket===null && document.querySelector('#lidarFields').disabled")
        assert browser.evaluate("document.querySelector('#imuStart').disabled && document.querySelector('#imuStop').disabled && !document.querySelector('#angles').textContent.includes('°')"), 'Disconnect must clear IMU preview'
        browser.evaluate("window.liveConfigs=[];document.querySelector('#cmdConnect').click()")
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r=>r.bSuccess)', timeout=20)
        assert browser.evaluate("!document.querySelector('#imuStart').disabled && document.querySelector('#imuStop').disabled"), 'Reconnect must leave IMU preview stopped'
        browser.evaluate("window.liveIMU=[];document.querySelector('#imuStart').click()")
        browser_tools.wait_for(browser, 'window.liveIMU.filter(r=>r.available && r.orientationValid).length>=3', timeout=20)
        sent = [event['params']['response']['payloadData'] for event in browser.events
                if event.get('method')=='Network.webSocketFrameSent'
                and event['params']['response']['payloadData'].endswith('EOJ')]
        assert sent and all(json.loads(text[:-3])['cmd'] in ('getConfig','getIMU') for text in sent), 'Live control test must only read'
        result['commands'] = len(sent)
        if screenshot_path:
            import base64
            screenshot_path.write_bytes(base64.b64decode(browser.call('Page.captureScreenshot', format='png')['data']))
        return result
    except Exception:
        print('Browser diagnostics:', json.dumps(browser.evaluate(r"""({
            url:location.href,
            status:document.querySelector('#status')?.textContent,
            configStatus:document.querySelector('#configStatus')?.textContent,
            commandPort:document.querySelector('#cmdPort')?.value,
            commandLog:document.querySelector('#cmdState')?.value,
            socket:window.wsSocket ? {url:window.wsSocket.url,state:window.wsSocket.readyState} : null,
            configReplies:window.liveConfigs?.length,
            imuStatus:document.querySelector('#imuStatus')?.textContent,
            imuReplies:window.liveIMU?.length,
            latestIMU:window.liveIMU?.at(-1)
        })"""), indent=2))
        print('Browser events:', json.dumps([event for event in browser.events if event.get('method') in
            ('Network.loadingFailed', 'Network.webSocketCreated', 'Network.webSocketFrameError',
             'Network.webSocketFrameSent', 'Runtime.exceptionThrown')][-12:], indent=2))
        raise
    finally:
        browser.evaluate("document.querySelector('#imuStop').click();document.querySelector('#stop').click();if(window.wsSocket)wsStop()")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--response', type=Path, help='Replay a captured getConfig reply')
    mode.add_argument('--live-url', help='Read config and point cloud from a running Airy viewer')
    parser.add_argument('--screenshot', type=Path)
    args = parser.parse_args()
    if args.live_url and not args.live_url.startswith(('http://', 'https://')):
        parser.error('--live-url requires an HTTP or HTTPS URL')
    handler = functools.partial(QuietHandler, directory=str(ROOT / 'html/viewer/_Airy'))
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
                browser_tools.wait_for(browser, "document.readyState==='complete' && !document.querySelector('#start').disabled")
                result = check_controls(browser, json.loads(args.response.read_text()) if args.response else None)
                result['imu'] = check_mock_imu(browser)
            if args.screenshot and not args.live_url:
                import base64
                args.screenshot.write_bytes(base64.b64decode(browser.call('Page.captureScreenshot', format='png')['data']))
            browser.call('Emulation.setDeviceMetricsOverride', width=390, height=844, deviceScaleFactor=1, mobile=True)
            browser.evaluate('new Promise(resolve=>setTimeout(resolve,100))')
            assert browser.evaluate('document.documentElement.scrollWidth<=innerWidth'), 'Mobile layout must not overflow'
            errors = [event['params'] for event in browser.events if event.get('method')=='Runtime.exceptionThrown']
            assert not errors, errors
            print(json.dumps(result, indent=2))
            print('PASS: Airy controls, IMU preview, device status, and responsive layout')
    finally:
        server.shutdown()
        server.server_close()
        worker.join()


if __name__ == '__main__':
    main()

