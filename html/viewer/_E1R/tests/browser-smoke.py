#!/usr/bin/env python3
"""Test E1R configuration controls with a simulated or running backend."""

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
        assert($('#lidarModule').value === 'E1R', 'Default module must match launch configuration');
        let config = {deviceIP:'192.168.1.200', msopPort:6699, bReflectivity:true,
            frameTimeoutMs:250, vRangeD:[0.1,120], vColorDefault:[1,1,1], vOrt:[0,0,0,1]};
        const schema = {
            deviceIP:{type:'string',category:'Receiver'}, msopPort:{type:'integer',min:1,max:65535},
            bReflectivity:{type:'bool'}, frameTimeoutMs:{type:'integer',min:10,max:10000},
            vRangeD:{type:'range',length:2,min:0,max:327.675}, vColorDefault:{type:'array',length:3,min:0,max:1},
            vOrt:{type:'array',length:4,min:-1,max:1},
            deviceReturnMode:{type:'integer',readOnly:true,category:'Device parameters',
                choices:[{value:4,label:'Strongest'}]},
            unavailable:{type:'integer',supported:false}
        };
        const reply = (extra = {}, request = sent.at(-1)) => {
            const text = JSON.stringify({...request, bSuccess:true, deviceOpen:true, config,
                status:{state:'streaming', pointPackets:123, difopPackets:7,
                    observed:{deviceReturnMode:4}}, ...extra}) + 'EOJ';
            for (let at=0; at<text.length; at+=7) cmdHandler({data:text.slice(at,at+7)});
        };
        reply({schema});
        assert(!$('#lidarFields').disabled, 'Reply must unlock controls');
        assert(document.querySelectorAll('.control').length === Object.keys(schema).length, 'Every schema field must render');
        assert(!field('deviceIP').disabled, 'Host source filter must be editable');
        assert(field('deviceReturnMode').disabled && field('deviceReturnMode').value === '4', 'Device readback must be read-only');
        assert(field('unavailable').disabled, 'Unsupported controls must remain disabled');
        assert($('#deviceStatus').textContent.includes('123'), 'Packet counters must be visible');
        const change = (key,value) => { const input=field(key); input.value=value; input.dispatchEvent(new Event('change')); return sent.at(-1); };
        let count=sent.length;
        change('deviceReturnMode','4'); assert(sent.length===count,'Read-only fields must never send writes');
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
                                  ['vColorDefault','[1,2,0]'],['vOrt','[0,0,1]']]) {
            count=sent.length; change(key,value); assert(sent.length===count,'Invalid vector must not be sent: '+key+' '+value);
        }
        request=change('vColorDefault','[1,0.5,0]');
        assert(JSON.stringify(request.config.vColorDefault)==='[1,0.5,0]','Color must preserve numeric vector'); reply();
        request=change('deviceIP','0.0.0.0');
        assert(request.config.deviceIP==='0.0.0.0','Source filter must retain string type'); reply();
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
        let capturedControls=0;
        if (window.capturedResponse) {
            $('#getConfig').click(); const source=window.capturedResponse;
            reply({...source,module:'E1R',cmd:'getConfig',requestId:sent.at(-1).requestId});
            const specs=Array.isArray(source.schema)?source.schema:Object.entries(source.schema).map(([key,spec])=>({key,...spec}));
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
                'bounds/vector validation','reply correlation','rejected edit rollback','save','status polling','disconnect']};
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
        window.liveConfigs=[];
        window.addEventListener('e1rcommand', event => {
            if(event.detail.cmd==='getConfig') window.liveConfigs.push(event.detail);
        });
        document.querySelector('#cmdConnect').click();
    })()""")
    try:
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r=>r.bSuccess)', timeout=20)
        result = browser.evaluate(r"""(() => {
            const config=window.liveConfigs.at(-1);
            const specs=Array.isArray(config.schema)?config.schema:Object.entries(config.schema).map(([key,spec])=>({key,...spec}));
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
        browser.evaluate("document.querySelector('#cmdDisconnect').click()")
        browser_tools.wait_for(browser, "window.wsSocket===null && document.querySelector('#lidarFields').disabled")
        browser.evaluate("window.liveConfigs=[];document.querySelector('#cmdConnect').click()")
        browser_tools.wait_for(browser, 'window.liveConfigs.some(r=>r.bSuccess)', timeout=20)
        sent = [event['params']['response']['payloadData'] for event in browser.events
                if event.get('method')=='Network.webSocketFrameSent'
                and event['params']['response']['payloadData'].endswith('EOJ')]
        assert sent and all(json.loads(text[:-3])['cmd']=='getConfig' for text in sent), 'Live control test must only read'
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
            configReplies:window.liveConfigs?.length
        })"""), indent=2))
        print('Browser events:', json.dumps([event for event in browser.events if event.get('method') in
            ('Network.loadingFailed', 'Network.webSocketCreated', 'Network.webSocketFrameError',
             'Network.webSocketFrameSent', 'Runtime.exceptionThrown')][-12:], indent=2))
        raise
    finally:
        browser.evaluate("document.querySelector('#stop').click();if(window.wsSocket)wsStop()")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--response', type=Path, help='Replay a captured getConfig reply')
    mode.add_argument('--live-url', help='Read config and point cloud from a running E1R viewer')
    parser.add_argument('--screenshot', type=Path)
    args = parser.parse_args()
    if args.live_url and not args.live_url.startswith(('http://', 'https://')):
        parser.error('--live-url requires an HTTP or HTTPS URL')
    handler = functools.partial(QuietHandler, directory=str(ROOT / 'html/viewer/_E1R'))
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
            if args.screenshot and not args.live_url:
                import base64
                args.screenshot.write_bytes(base64.b64decode(browser.call('Page.captureScreenshot', format='png')['data']))
            browser.call('Emulation.setDeviceMetricsOverride', width=390, height=844, deviceScaleFactor=1, mobile=True)
            browser.evaluate('new Promise(resolve=>setTimeout(resolve,100))')
            assert browser.evaluate('document.documentElement.scrollWidth<=innerWidth'), 'Mobile layout must not overflow'
            errors = [event['params'] for event in browser.events if event.get('method')=='Runtime.exceptionThrown']
            assert not errors, errors
            print(json.dumps(result, indent=2))
            print('PASS: E1R controls, device status, and responsive layout')
    finally:
        server.shutdown()
        server.server_close()
        worker.join()


if __name__ == '__main__':
    main()

