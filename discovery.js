/* eslint-disable @typescript-eslint/no-var-requires */
'use strict';

let discoveryNative;
try {
  discoveryNative = require('./discovery.node');
} catch (e1) {
  try {
    discoveryNative = require(process.cwd() + '/discovery.node');
  } catch (e2) {
    console.error('Failed to require native discovery module', e1, e2);
  }
}

function depthaiInfo() {
  return discoveryNative.depthaiInfo();
}

function scan() {
  return new Promise((resolve, reject) => {
    discoveryNative.listDevices((err, list) => {
      if (err) reject(err);
      else resolve(list);
    });
  });
}

function release(mxId) {
  return new Promise((resolve, reject) => {
    discoveryNative.stopMonitor(mxId, err => {
      if (err) reject(err);
      else resolve();
    });
  });
}

function monitor(mxId, callback) {
  return new Promise((resolve, reject) => {
    discoveryNative.monitor(mxId, callback, (err, result) => {
      if (err) reject(err);
      else resolve(result);
    });
  });
}

function requestStream(clientId, streamId) {
  return new Promise((resolve, reject) => {
    discoveryNative.requestStream(clientId, streamId, (err, offer) => {
      if (err) reject(err);
      else resolve(offer);
    });
  });
}

function connectStream(clientId, sdp) {
  return new Promise(resolve => {
    discoveryNative.connectStream(clientId, sdp, result => {
      resolve(result);
    });
  });
}

function streamMonitor(callback) {
  return discoveryNative.streamMonitor(callback);
}

function stopStreamMonitor(monitorId) {
  return discoveryNative.stopStreamMonitor(monitorId);
}

function stopStream(streamId) {
  return discoveryNative.stopStream(streamId);
}

module.exports = discoveryNative
  ? {
      depthaiInfo,
      streamMonitor,
      stopStreamMonitor,
      stopStream,
      monitor,
      release,
      scan,
      requestStream,
      connectStream,
      configureIceServers: discoveryNative.configureIceServers,
    }
  : {
      depthaiInfo: () => Promise.resolve({}),
      streamMonitor: () => undefined,
      stopStreamMonitor: () => undefined,
      stopStream: () => undefined,
      monitor: (mxId, callback) => {
        callback({
          type: 'error',
          serialNumber: mxId,
          message: 'Discovery module is not installed',
        });
      },
      release: () => Promise.resolve(),
      scan: () => Promise.resolve([]),
      requestStream: () => Promise.reject(new Error('Discovery module is not installed')),
      connectStream: () => false,
      configureIceServers: () => undefined,
    };
