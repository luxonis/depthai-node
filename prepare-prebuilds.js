const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

const prebuildsDir = path.join(__dirname, 'prebuilds');
if (!fs.existsSync(prebuildsDir)) {
  fs.mkdirSync(prebuildsDir);
}

const arch = process.argv[2];

fs.renameSync(
  path.join(__dirname, 'build/Release/depthai_node.node'),
  path.join(__dirname, `prebuilds/depthai_node.${arch}.node`),
);
