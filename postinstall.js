/* eslint-disable @typescript-eslint/no-var-requires */
const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

if (fs.existsSync(path.resolve('./prebuilds'))) {
  const arch = childProcess.execSync('uname -m').toString('utf-8').trim();
  if (fs.existsSync(path.resolve('./discovery.node'))) {
    fs.unlinkSync(path.resolve('./discovery.node'));
  }
  fs.linkSync(path.resolve(`./prebuilds/discovery.${arch}.node`), path.resolve('./discovery.node'));
}
