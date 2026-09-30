#!/usr/bin/env python3
"""Test RealSense controls and IMU preview with a simulated or running backend."""

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
    def log_message(self, *_):
        pass


def check_response(browser, response):
    browser.evaluate('window.capturedCameraResponse = ' + json.dumps(response))
    return browser.evaluate(r"""(() => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const field = key => document.getElementById('param-' + key);
        const source = window.capturedCameraResponse;
        const sent = [];
        window.wsSocket = {readyState: WebSocket.OPEN};
        window.wsSendCmd = command => {sent.push(command); return true;};
        window.dispatchEvent(new Event('wscmdstatechange'));
        const reply = () => {
            const message = {...source, module:'RealSense', requestId:sent.at(-1).requestId, cmd:sent.at(-1).cmd};
            const text = JSON.stringify(message) + 'EOJ';
            for (let at = 0; at < text.length; at += 512) cmdHandler({data:text.slice(at, at + 512)});
        };
        reply();
        const keys = new Set();
        for (const spec of source.schema) {
            const key = spec.domain ? `sensorOptions.${spec.domain}.${spec.key}` : spec.key;
            keys.add(key);
            const input = field(key);
            assert(input, 'Missing captured control ' + key);
            assert(input.disabled === (spec.supported === false || !!spec.readOnly), 'Wrong writability for ' + key);
            const value = spec.domain ? source.config.sensorOptions?.[spec.domain]?.[spec.key] : source.config[spec.key];
            if (!spec.readOnly && value == null) assert(input.value === '', 'Null override must remain unset for ' + key);
            if (spec.readOnly && spec.current != null) assert(input.value === String(spec.current), 'Read-only value missing for ' + key);
        }
        assert(document.querySelectorAll('.control').length === keys.size, 'Captured controls must be unique');
        let checked = 0;
        for (const [key, value] of [
            ['sensorOptions.depth.RS2_OPTION_ENABLE_AUTO_EXPOSURE', 0],
            ['sensorOptions.color.RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE', 0],
            ['sensorOptions.depth.RS2_OPTION_VISUAL_PRESET', 3],
            ['sensorOptions.threshold.RS2_OPTION_MAX_DISTANCE', 4],
            ['sensorOptions.spatial.RS2_OPTION_FILTER_SMOOTH_ALPHA', 0.5],
            ['sensorOptions.temporal.RS2_OPTION_FILTER_SMOOTH_ALPHA', 0.4],
            ['sensorOptions.depth.RS2_OPTION_DEPTH_UNITS', 0.01]
        ]) {
            const input = field(key);
            if (!input || input.disabled) continue;
            const count = sent.length;
            input.value = String(value); input.dispatchEvent(new Event('change'));
            assert(sent.length === count + 1, `Captured setting ${key}=${value} must pass browser validation`);
            const [, domain, option] = key.split('.');
            assert(sent.at(-1).config.sensorOptions[domain][option] === value, 'SDK float controls must send numeric values');
            ++checked;
            reply();
        }
        window.wsSocket = null;
        window.dispatchEvent(new Event('wscmdstatechange'));
        delete window.capturedCameraResponse;
        return {controls:keys.size, numericEdits:checked};
    })()""")


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
        window.wsSendCmd = command => { sent.push(command); return true; };
        window.dispatchEvent(new Event('wscmdstatechange'));
        assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connecting must not automatically start IMU polling');
        const configReply = request => reply({...request, bSuccess:true, deviceOpen:true, config:{bIMU:true}});
        configReply(sent[0]);
        $('#imuStart').click();
        const first = sent.at(-1);
        assert(first.cmd === 'getIMU' && first.module === $('#cameraModule').value && /^imu-\d+$/.test(first.requestId),
            'Start must request IMU from the selected module with a separate requestId');
        assert(JSON.stringify(Object.keys(first).sort()) === '["cmd","module","requestId"]',
            'IMU Start must only read samples');
        assert($('#imuStart').disabled && !$('#imuStop').disabled, 'Start must enable Stop');
        await delay(250);
        assert(sent.at(-1) === first, 'IMU polling must keep at most one outstanding request');
        const epoch = 9007199254740993001n;
        const sample = (request, overrides = {}) => ({...request, bSuccess:true, enabled:true, available:true,
            deviceOpen:true, tGyro:String(epoch), tAcc:String(epoch), tFusion:String(epoch),
            gyro:[0.1, -0.2, 0.3], acc:[1, 2, 9.7], quaternion:[1, 0, 0, 0], rpy:[0.1, 0.2, 0.3],
            orientationValid:true, ...overrides});
        reply(sample(first, {requestId:'imu-stale'}));
        reply(sample(first, {module:'DifferentCamera'}));
        assert(empty(), 'Mismatched replies must not update IMU graphs or orientation');
        $('#getConfig').click();
        const configuration = sent.at(-1);
        assert(configuration.cmd === 'getConfig', 'Camera controls must remain independent of IMU polling');
        reply(sample(first));
        assert($('#cameraFields').disabled, 'IMU reply must not complete a pending config request');
        configReply(configuration);
        assert(!$('#cameraFields').disabled, 'The config reply must still unlock camera controls');
        assert(JSON.stringify(values()) === '["0.100","-0.200","0.300","1.000","2.000","9.700"]',
            'Each IMU channel must display its value and sign');
        assert($('#angles').textContent === 'Roll 5.7° · Pitch 11.5° · Yaw 17.2°',
            'Orientation angles must convert radians to degrees');
        assert($('#imuStatus').textContent.includes('fused orientation'), 'Fused orientation status must be visible');
        const nextRequest = async () => {
            const before = sent.filter(request => request.cmd === 'getIMU').length;
            for (let attempt = 0; attempt < 30; ++attempt) {
                await delay(20);
                if (sent.filter(request => request.cmd === 'getIMU').length > before) return sent.at(-1);
            }
            throw Error('IMU preview did not schedule its next request');
        };
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
        request = sent.at(-1);
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
        assert(empty() && $('#imuStatus').textContent.includes('not open'), 'Camera closure must be reflected in the panel');
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
            'IMU preview must not change camera configuration or streams');
        return {graphs:6, imuRequests:sent.filter(command => command.cmd === 'getIMU').length,
            checks:['fragmented IMU replies', 'one outstanding request', 'request/module correlation',
                'config request isolation', 'nanosecond precision', 'duplicate sample suppression', 'stale sample clearing',
                'unavailable/disabled/closed camera', 'orientation validity', 'Stop', 'disconnect']};
    })()""")


def check_live_imu(browser):
    browser.evaluate("document.querySelector('#imuStart').click()")
    live_wait(browser, r"""window.liveIMUReplies.filter(reply => reply.bSuccess && reply.available && reply.orientationValid).length >= 3 &&
        document.querySelector('#imuStatus').textContent.includes('fused orientation')""", 'Waiting for live IMU samples and orientation')
    result = browser.evaluate(r"""(() => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const replies = window.liveIMUReplies.filter(reply => reply.available && reply.orientationValid);
        const reply = replies.at(-1);
        assert(reply.enabled && reply.deviceOpen, 'Live IMU must be enabled on an open camera');
        assert(reply.module === document.querySelector('#cameraModule').value && /^imu-\d+$/.test(reply.requestId),
            'IMU response must identify its module and request');
        for (const key of ['tGyro', 'tAcc', 'tFusion'])
            assert(typeof reply[key] === 'string' && /^[0-9]+$/.test(reply[key]) && BigInt(reply[key]) > 0n,
                key + ' must preserve nanoseconds as a decimal string');
        for (const [key, size] of [['gyro',3], ['acc',3], ['rpy',3], ['quaternion',4]])
            assert(Array.isArray(reply[key]) && reply[key].length === size && reply[key].every(Number.isFinite),
                'Invalid live IMU vector: ' + key);
        assert(Math.abs(Math.hypot(...reply.quaternion) - 1) < 0.001, 'Fused quaternion must be normalized');
        assert(new Set(replies.map(sample => sample.tGyro)).size > 1 &&
               new Set(replies.map(sample => sample.tAcc)).size > 1, 'IMU capture timestamps must advance');
        assert(!('schema' in reply) && !('config' in reply), 'IMU polling must not resend the camera schema');
        const values = Array.from(document.querySelectorAll('#imuGraphs figcaption span:last-child'), el => el.textContent);
        assert(values.length === 6 && values.every(value => value !== '—' && Number.isFinite(Number(value))),
            'Every live gyro/accelerometer graph must show a numeric value');
        assert(document.querySelector('#angles').textContent.includes('°'), 'Live orientation must display angles');
        document.querySelector('#imuStop').click();
        assert(!document.querySelector('#imuStart').disabled && document.querySelector('#imuStop').disabled,
            'Stop must return IMU controls to their ready state');
        return {samples: replies.length, tGyro:reply.tGyro, tAcc:reply.tAcc, gyro:reply.gyro, acc:reply.acc,
            quaternion:reply.quaternion, rpy:reply.rpy, graphs:values};
    })()""")
    count = len([event for event in browser.events if event.get('method') == 'Network.webSocketFrameSent'])
    browser.evaluate('new Promise(resolve => setTimeout(resolve, 350))')
    after = len([event for event in browser.events if event.get('method') == 'Network.webSocketFrameSent'])
    assert after == count, 'IMU Stop must cancel subsequent requests'
    return result


def live_diagnostics(browser):
    state = browser.evaluate(r"""(() => ({
        status: document.querySelector('#configStatus')?.textContent,
        socketState: window.wsSocket?.readyState ?? null,
        replies: window.liveCameraReplies?.length ?? 0,
        imuReplies: window.liveIMUReplies?.length ?? 0,
        imuStatus: document.querySelector('#imuStatus')?.textContent,
        controls: document.querySelectorAll('.control').length,
        commandLog: document.querySelector('#cmdState')?.value.slice(0, 1800)
    }))()""")
    sent = [event['params']['response']['payloadData'] for event in browser.events
            if event.get('method') == 'Network.webSocketFrameSent']
    received = [event['params']['response'] for event in browser.events
                if event.get('method') == 'Network.webSocketFrameReceived']
    state['sentFrames'] = sent
    state['receivedFrames'] = len(received)
    state['heartbeatFrames'] = sum(frame.get('payloadData') == '{"cmd":"hb"}' for frame in received)
    return state


def live_wait(browser, expression, stage):
    try:
        return browser_tools.wait_for(browser, expression, timeout=18)
    except AssertionError as error:
        raise AssertionError(f'{stage}: {json.dumps(live_diagnostics(browser), indent=2)}') from error


def check_live_reply(browser, expected_replies):
    live_wait(browser, f'window.liveCameraReplies.length >= {expected_replies}',
              f'Waiting for getConfig reply {expected_replies}')
    return browser.evaluate(r"""(() => {
        const assert = (condition, message) => { if (!condition) throw Error(message); };
        const reply = window.liveCameraReplies.at(-1);
        assert(reply.cmd === 'getConfig', 'Expected a getConfig reply');
        assert(reply.module === document.querySelector('#cameraModule').value, 'Reply module must match');
        assert(typeof reply.requestId === 'string' && reply.requestId, 'Reply must carry a requestId');
        assert(reply.bSuccess, 'getConfig failed: ' + JSON.stringify(reply.errors || reply.error));
        assert(reply.config && Array.isArray(reply.schema) && reply.schema.length, 'Reply must include config and schema');
        assert(!document.querySelector('#cameraFields').disabled, 'Reply must unlock camera controls');
        assert(!document.querySelector('#getConfig').disabled, 'Reply must enable Refresh config');
        const keys = new Set();
        for (const spec of reply.schema) {
            const key = spec.domain ? `sensorOptions.${spec.domain}.${spec.key}` : spec.key;
            keys.add(key);
            const input = document.getElementById('param-' + key);
            assert(input, 'Missing control: ' + key);
            assert(input.disabled === (spec.supported === false || !!spec.readOnly), 'Incorrect writability: ' + key);
        }
        assert(document.querySelectorAll('.control').length === keys.size, 'Schema controls must be unique');
        assert(window.wsSocket instanceof WebSocket && window.wsSocket.readyState === WebSocket.OPEN,
            'Commands must use an open native WebSocket');
        return {requestId: reply.requestId, controls: keys.size, deviceOpen: reply.deviceOpen,
            status: document.querySelector('#configStatus').textContent};
    })()""")


def disconnect_live(browser):
    browser.evaluate(r"""(() => {
        window.livePreviousSocket = window.wsSocket;
        document.querySelector('#cmdDisconnect').click();
    })()""")
    live_wait(browser, 'window.livePreviousSocket?.readyState === WebSocket.CLOSED && window.wsSocket === null',
              'Waiting for command socket to close')
    browser.evaluate(r"""(() => {
        if (!document.querySelector('#cameraFields').disabled || !document.querySelector('#saveConfig').disabled ||
            document.querySelector('#cmdConnect').disabled)
            throw Error('Disconnect must disable camera writes and enable Connect');
    })()""")


def run_live(url):
    # Navigation must leave Connect under test control, even for a copied #connect URL.
    url = url.split('#', 1)[0]
    with Browser() as browser:
        browser.call('Network.emulateNetworkConditions', offline=False, latency=0,
                     downloadThroughput=-1, uploadThroughput=-1)
        browser.call('Page.navigate', url=url)
        live_wait(browser, "document.readyState === 'complete' && !!document.querySelector('#getConfig')?.onclick",
                  'Waiting for RealSense viewer to load')
        browser.evaluate(r"""(() => {
            window.liveCameraReplies = [];
            window.liveIMUReplies = [];
            window.addEventListener('realsensecommand', event => {
                if (event.detail.cmd === 'getConfig') window.liveCameraReplies.push(event.detail);
                if (event.detail.cmd === 'getIMU') window.liveIMUReplies.push(event.detail);
            });
        })()""")
        try:
            browser.evaluate("document.querySelector('#cmdConnect').click()")
            connected = check_live_reply(browser, 1)
            imu = check_live_imu(browser)
            browser.evaluate("document.querySelector('#getConfig').click()")
            refreshed = check_live_reply(browser, 2)
            disconnect_live(browser)
            browser.evaluate("document.querySelector('#cmdConnect').click()")
            reconnected = check_live_reply(browser, 3)
            browser.evaluate("if (window.wsSocket === window.livePreviousSocket) throw Error('Reconnect must open a new socket')")
            disconnect_live(browser)
            sent = [event['params']['response']['payloadData'] for event in browser.events
                    if event.get('method') == 'Network.webSocketFrameSent'
                    and event['params']['response']['opcode'] == 1]
            assert all(message.endswith('EOJ') for message in sent), f'Requests must use EOJ framing: {sent!r}'
            requests = [json.loads(message[:-3]) for message in sent]
            assert all(request.get('cmd') in ('getConfig', 'getIMU') and
                       set(request) == {'module', 'cmd', 'requestId'} for request in requests), \
                f'Live test must only read configuration and IMU: {requests!r}'
            config_requests = [request for request in requests if request['cmd'] == 'getConfig']
            assert len(config_requests) == 3, f'Expected three config requests: {requests!r}'
            assert any(request['cmd'] == 'getIMU' for request in requests), 'IMU preview must request samples'
            request_ids = [request['requestId'] for request in config_requests]
            reply_ids = [reply['requestId'] for reply in (connected, refreshed, reconnected)]
            assert len(set(request_ids)) == 3 and request_ids == reply_ids, 'Each request must receive its own correlated reply'
            errors = [event['params'] for event in browser.events if event.get('method') == 'Runtime.exceptionThrown']
            assert not errors, errors
            print(json.dumps({'url': url, 'connect': connected, 'imu': imu, 'refresh': refreshed, 'reconnect': reconnected,
                              'requests': requests, 'checks': ['native WebSocket', 'Connect', 'getConfig reply',
                              'schema rendering', 'live IMU samples', 'IMU graphs and orientation', 'IMU Stop',
                              'Refresh config', 'disconnect', 'reconnect', 'read-only requests']}, indent=2))
            print('PASS: RealSense live command connection')
        finally:
            # Release the single command connection on both success and failure.
            browser.evaluate("if (window.wsSocket) document.querySelector('#cmdDisconnect').click()")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--response', type=Path, help='Also replay a captured getConfig response without accessing hardware')
    mode.add_argument('--live-url', help='Test Connect, live IMU preview, Refresh and reconnect against a running viewer without changing settings')
    args = parser.parse_args()
    if args.live_url:
        if not args.live_url.startswith(('http://', 'https://')):
            parser.error('--live-url requires an http:// or https:// viewer URL')
        run_live(args.live_url)
        return
    handler = functools.partial(QuietHandler, directory=str(ROOT / 'html/viewer/_RealSense'))
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
    worker = threading.Thread(target=server.serve_forever, daemon=True)
    worker.start()
    try:
        with Browser() as browser:
            browser.call('Network.emulateNetworkConditions', offline=False, latency=0,
                         downloadThroughput=-1, uploadThroughput=-1)
            browser.call('Page.navigate', url=f'http://localhost:{server.server_port}/')
            browser_tools.wait_for(browser, "document.readyState === 'complete' && !document.querySelector('#start').disabled")
            result = browser.evaluate(r"""(() => {
                const assert = (condition, message) => { if (!condition) throw Error(message); };
                const $ = selector => document.querySelector(selector);
                const field = key => document.getElementById('param-' + key);
                const sent = [];
                window.wsSocket = { readyState: WebSocket.OPEN };
                window.wsSendCmd = command => { sent.push(command); return true; };
                window.dispatchEvent(new Event('wscmdstatechange'));
                assert(sent.length === 1 && sent[0].cmd === 'getConfig', 'Connect must load config');
                const config = { SN: '', bIMU: true, vSizeD: [848, 480], accelFPS: 250,
                    sensorOptions: { depth: { RS2_OPTION_EXPOSURE: 8500, RS2_OPTION_ENABLE_AUTO_EXPOSURE: false },
                        color: { RS2_OPTION_EXPOSURE: 100 } } };
                const schema = [
                    {key:'SN', category:'Streams and point cloud', type:'string', restart:true},
                    {key:'bIMU', category:'Streams and point cloud', type:'bool', restart:true},
                    {key:'vSizeD', category:'Streams and point cloud', type:'size', restart:true},
                    {key:'accelFPS', category:'Streams and point cloud', type:'int', min:0, restart:true},
                    {key:'RS2_OPTION_EXPOSURE', domain:'depth', category:'Depth sensor', type:'float',
                        nullable:true, min:1, max:20000, step:1, current:8500},
                    {key:'RS2_OPTION_EXPOSURE', domain:'color', category:'Color sensor', type:'float',
                        nullable:true, min:1, max:1000, step:1, current:100},
                    {key:'RS2_OPTION_ENABLE_AUTO_EXPOSURE', domain:'depth', category:'Depth sensor', type:'bool', nullable:true},
                    {key:'RS2_OPTION_VISUAL_PRESET', domain:'depth', category:'Depth sensor', type:'float', nullable:true,
                        choices:[{value:0,label:'Custom'},{value:1,label:'Default'},{value:3,label:'High accuracy'}]},
                    {key:'RS2_OPTION_ASIC_TEMPERATURE', domain:'depth', category:'Depth sensor', type:'float', readOnly:true, current:42},
                    {key:'RS2_OPTION_LASER_POWER', domain:'depth', category:'Depth sensor', type:'float', supported:false},
                    {key:'RS2_OPTION_EXPOSURE', domain:'depth', category:'Depth sensor', type:'float'},
                    {key:'RS2_OPTION_REGION_OF_INTEREST', domain:'color', category:'Color sensor', type:'rect', nullable:true},
                    {key:'RS2_OPTION_STRING', domain:'color', category:'Color sensor', type:'string', nullable:true},
                    {key:'RS2_OPTION_MAX_DISTANCE', domain:'threshold', category:'Threshold filter', type:'float', nullable:true,
                        min:0, max:16, step:0.10000000149011612},
                    {key:'RS2_OPTION_DEPTH_UNITS', domain:'depth', category:'Depth sensor', type:'float', nullable:true,
                        min:9.999999974752427e-7, max:0.009999999776482582, step:9.999999974752427e-7}
                ];
                const response = (options = {}) => ({module:'RealSense', cmd:sent.at(-1).cmd,
                    requestId:sent.at(-1).requestId, bSuccess:true, deviceOpen:true, config, ...options});
                const reply = options => {
                    const text = JSON.stringify(response(options)) + 'EOJ';
                    for (let i = 0; i < text.length; i += 7) cmdHandler({data:text.slice(i, i + 7)});
                };
                reply({schema});
                assert($('#cameraModule').value === 'RealSense', 'Default module must match config');
                assert(!$('#cameraFields').disabled, 'Reply must unlock controls');
                assert(document.querySelectorAll('.control').length === schema.length - 1, 'Duplicate domain/key must not render twice');
                assert(field('sensorOptions.depth.RS2_OPTION_EXPOSURE').value === '8500', 'Depth override must load');
                assert(field('sensorOptions.color.RS2_OPTION_EXPOSURE').value === '100', 'Color override must remain separate');
                assert(field('sensorOptions.depth.RS2_OPTION_ASIC_TEMPERATURE').disabled &&
                    field('sensorOptions.depth.RS2_OPTION_ASIC_TEMPERATURE').value === '42', 'Read-only current value must display disabled');
                assert(field('sensorOptions.depth.RS2_OPTION_LASER_POWER').disabled, 'Unsupported options must be disabled');
                assert(field('sensorOptions.depth.RS2_OPTION_VISUAL_PRESET').value === '', 'Readbacks must not become saved overrides');
                const change = (key, value) => {
                    const input = field(key); input.value = value; input.dispatchEvent(new Event('change'));
                    return sent.at(-1);
                };
                let request = change('sensorOptions.depth.RS2_OPTION_EXPOSURE', '9000');
                assert(JSON.stringify(request.config) === '{"sensorOptions":{"depth":{"RS2_OPTION_EXPOSURE":9000}}}', 'Depth patch must preserve other sensor overrides');
                assert($('#cameraFields').disabled, 'Pending change must lock controls');
                window.dispatchEvent(new CustomEvent('realsensecommand', {detail:response({requestId:'stale', config:{}})}));
                assert($('#cameraFields').disabled, 'Stale response must not unlock controls');
                reply({bSuccess:false, errors:{'sensorOptions.depth.RS2_OPTION_EXPOSURE':'Manual exposure disabled'}});
                assert(field('sensorOptions.depth.RS2_OPTION_EXPOSURE').value === '8500', 'Rejected edit must restore accepted value');
                assert($('#configStatus').textContent.includes('Manual exposure disabled'), 'Device error must be visible');
                request = change('sensorOptions.depth.RS2_OPTION_EXPOSURE', '');
                assert(request.config.sensorOptions.depth.RS2_OPTION_EXPOSURE === null, 'Blank option must clear its override');
                reply();
                request = change('sensorOptions.depth.RS2_OPTION_ENABLE_AUTO_EXPOSURE', 'true');
                assert(request.config.sensorOptions.depth.RS2_OPTION_ENABLE_AUTO_EXPOSURE === true, 'SDK boolean must stay boolean');
                reply();
                request = change('sensorOptions.depth.RS2_OPTION_VISUAL_PRESET', '3');
                assert(request.config.sensorOptions.depth.RS2_OPTION_VISUAL_PRESET === 3, 'Preset choices must stay numeric');
                reply();
                const beforeBad = sent.length;
                change('vSizeD', '[848,"480"]');
                assert(sent.length === beforeBad && $('#configStatus').textContent.includes('array of numbers'), 'Invalid dimensions must not be sent');
                request = change('vSizeD', '[640,480]');
                assert(JSON.stringify(request.config.vSizeD) === '[640,480]', 'Stream dimensions must be a numeric array');
                reply();
                request = change('accelFPS', '0');
                assert(request.config.accelFPS === 0, 'Automatic IMU rate must allow zero');
                reply();
                request = change('sensorOptions.color.RS2_OPTION_REGION_OF_INTEREST', '[0,0,100,100]');
                assert(JSON.stringify(request.config.sensorOptions.color.RS2_OPTION_REGION_OF_INTEREST) === '[0,0,100,100]', 'Rect options must stay coordinate arrays');
                reply();
                request = change('sensorOptions.color.RS2_OPTION_STRING', 'example');
                assert(request.config.sensorOptions.color.RS2_OPTION_STRING === 'example', 'String options must stay strings');
                reply();
                let count = sent.length;
                request = change('sensorOptions.threshold.RS2_OPTION_MAX_DISTANCE', '4');
                assert(sent.length === count + 1 && request.config.sensorOptions.threshold.RS2_OPTION_MAX_DISTANCE === 4,
                    'Float32 step metadata must not reject an ordinary decimal step');
                reply();
                count = sent.length;
                request = change('sensorOptions.depth.RS2_OPTION_DEPTH_UNITS', '0.01');
                assert(sent.length === count + 1 && request.config.sensorOptions.depth.RS2_OPTION_DEPTH_UNITS === 0.01,
                    'Float32 bounds must allow decimal SDK endpoints');
                reply();
                $('#saveConfig').click();
                assert(sent.at(-1).cmd === 'saveConfig' && !('config' in sent.at(-1)), 'Save must not contain a path or implicit readback overrides');
                reply();
                assert($('#configStatus').textContent === 'Configuration saved to disk', 'Save confirmation must be shown');
                window.wsSocket = null;
                window.dispatchEvent(new Event('wscmdstatechange'));
                assert($('#cameraFields').disabled && $('#saveConfig').disabled, 'Disconnect must disable device writes');
                return {title:document.title, controls:schema.length - 1, commands:sent.length,
                    checks:['page load','fragmented replies','sensor isolation','read-only values','override removal',
                            'booleans','preset choices','dimensions','automatic IMU rate','rectangles','strings',
                            'request correlation','error rollback','float32 steps and endpoints','save','disconnect']};
            })()""")
            result['imu'] = check_mock_imu(browser)
            errors = [event['params'] for event in browser.events
                      if event.get('method') == 'Runtime.exceptionThrown']
            assert not errors, errors
            if args.response:
                result['capturedResponse'] = check_response(browser, json.loads(args.response.read_text()))
            print(json.dumps(result, indent=2))
            print('PASS: RealSense viewer controls and command protocol')
    finally:
        server.shutdown()
        server.server_close()
        worker.join()


if __name__ == '__main__':
    main()
