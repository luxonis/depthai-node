const depthai = require('../index.js');

console.log('getAllAvailableDevices:\n', JSON.stringify(depthai.Device.getAllAvailableDevices(), undefined, 2));
console.log('getAllConnectedDevices:\n', JSON.stringify(depthai.Device.getAllConnectedDevices(), undefined, 2));