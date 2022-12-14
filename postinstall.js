/* eslint-disable @typescript-eslint/no-var-requires */
const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

if (fs.existsSync(path.resolve('./prebuilds'))) {
  const arch = childProcess.execSync('uname -m').toString('utf-8').trim();
  if (fs.existsSync(path.resolve('./depthai_node.node'))) {
    fs.unlinkSync(path.resolve('./depthai_node.node'));
  }
  const prebuildForArchPath = path.resolve(`./prebuilds/depthai_node.${arch}.node`);
  if (fs.existsSync(prebuildForArchPath)) {
    fs.linkSync(prebuildForArchPath, path.resolve('./depthai_node.node'));
  } else {
    throw new Error(`prebuild not available for ${arch}`);
  }
}

const hunterConfig = path.resolve(__dirname, 'depthai-core/cmake/Hunter/config.cmake');
if (fs.existsSync(hunterConfig)) {
  let contents = fs.readFileSync(hunterConfig, 'utf-8');
  contents = contents.replace('BUILD_SHARED_LIBS=ON', 'BUILD_SHARED_LIBS=OFF');
  fs.writeFileSync(hunterConfig, contents);
}
