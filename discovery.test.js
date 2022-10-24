/* eslint-disable @typescript-eslint/no-var-requires */

const assert = require('assert');
const crypto = require('crypto');
const { setTimeout: delay } = require('node:timers/promises');
const { XLinkProtocol, XLinkPlatform, XLinkState } = require('../lib/protocol/dist/lib/agent-methods');
const discovery = require('./discovery');
const native = require('./discovery.node');

assert(native);

const exported = [
  'listDevices',
  'streamMonitor',
  'stopStreamMonitor',
  'stopMonitor',
  'monitor',
  'requestStream',
  'connectStream',
  'sendFrame',
  'configureIceServers',
];

for (const name of exported) {
  assert(typeof native[name] === 'function');
}

async function testMonitoring(serialNumber) {
  console.log(`Test monitor/release with delay`);
  for (let i = 0; i < 3; i++) {
    try {
      const ms = i * 1500;
      console.log(`Monitoring...`);
      await discovery.monitor(serialNumber, noop);
      console.log(`Delaying release by ${ms} ms..`);
      await delay(ms);
      console.log(`Releasing...`);
      await discovery.release(serialNumber);
    } catch (e) {
      console.error(e);
    }
  }

  console.log(`Call monitor/release randomly`);
  for (let i = 0; i < 100; i++) {
    if (roll()) {
      console.log('Call monitor');
      discovery.monitor(serialNumber, noop).catch(noop);
    }
    if (roll()) {
      console.log('Call release');
      discovery.release(serialNumber).catch(noop);
    }
  }
  await delay(100);
  await discovery.release(serialNumber);
}

async function testStreaming(serialNumber) {
  for (let i = 0; i < 20; i++) {
    await discovery.requestStream(crypto.randomUUID(), serialNumber);
  }
  for (let i = 0; i < 20; i++) {
    await discovery.connectStream(crypto.randomUUID(), '.....');
  }
  for (let i = 0; i < 20; i++) {
    discovery.sendFrame(crypto.randomUUID(), 1, Buffer.from('Test'));
  }
}

async function stressTestMonitor(serialNumber) {
  for (let i = 0; i < 10; i++) {
    try {
      console.log('discovery.monitor start');
      const result = await discovery.monitor(serialNumber, payload => {
        console.log(payload);
      });
      console.log(`discovery.monitor result ${result}`);
      await delay(i);
    } catch (e) {
      console.log(e);
    }
  }
  await discovery.release(serialNumber);
}

async function testStreamMonitor() {
  ids = []
  for (let i = 0; i < 10; i++) {
    try {
      console.log('Testing stream monitor');
      id = discovery.streamMonitor(streams => {console.log(streams);});
      console.log(`streamId: ${id}`);
      ids.push(id);
      await delay(1000);
    } catch (e) {
      console.log(e);
    }
  }
  for (let i = 0; i < 10; i++) {
    console.log(`Stopping stream ${ids[i]}`);
    discovery.stopStreamMonitor(ids[i]);
  }
}

const testConfig = {
  monitoringSingle: false,
  monitoringParallel: false,
  stressTestMonitor: false,
  testStreamMonitor: false,
  streaming: false,
};

(async () => {
  const devices = await discovery.scan();
  if (devices.length === 0) {
    console.error('No devices to test!');
    return;
  }

  console.log(`Testing ${devices.length}`);
  for (const device of devices) {
    console.log(
      `Device ${device.serialNumber} [${device.name ?? 'unknown'}] protocol: ${
        XLinkProtocol[device.protocol]
      } platform: ${XLinkPlatform[device.platform]} state: ${XLinkState[device.state]}`,
    );
  }
  if (testConfig.monitoringParallel) {
    await Promise.all(devices.map(device => testMonitoring(device.serialNumber)));
  }

  if (testConfig.testStreamMonitor) {
    await testStreamMonitor();
  }

  for (const device of devices) {
    if (testConfig.stressTestMonitor) {
      await stressTestMonitor(device.serialNumber);
    }
    if (testConfig.monitoringSingle) {
      await testMonitoring(device.serialNumber);
    }
    if (testConfig.streaming) {
      await testStreaming(device.serialNumber);
    }
  }
})();

//eslint-disable-next-line @typescript-eslint/no-empty-function
function noop() {}

function roll() {
  return Math.random() < 0.5;
}
