// verify the ABI version of compiled native modules
// Copyright (c) 2019 Jos Verlinde
// https://gist.github.com/Josverl/ccaf7f93dabc3fa8fc3088d058efd485
// License : MIT

const os = require('os');
const fs = require('fs');
const path = require('path');

/// dummy copy of  internal function
function Module(id, parent) {
    this.id = id;
    this.exports = {};
    this.parent = parent;
    // updateChildren(parent, this, false);
    this.filename = null;
    this.loaded = false;
    this.children = [];
 }

 // try to determine the ABI version for a native module
const getNativeABI = (filename) => {
    var moduleVersion = 0
    try {
        var test = new Module(filename, null);
        process.dlopen(module, filename) //,os.constants.dlopen.RTLD_NOW);
        // if this works the node version is the same 
        moduleVersion = process.versions['modules']
        // but now we need to unload it :-( 
        return moduleVersion
    } catch (error) {
        var match
        var versionRegexp = /NODE_MODULE_VERSION (\d*)./gm
        var platformRegexp = /(is not a valid Win32 application|invalid ELF header|wrong ELF class)/g
        // check for ABI version mismatch 
            // Uncaught Error: The module '..\bindings.node'
            // was compiled against a different Node.js version using
            // NODE_MODULE_VERSION 47. This version of Node.js requires
            // NODE_MODULE_VERSION 70. Please try re-compiling or re-installing
        match = versionRegexp.exec(error.message)
        if (match != null){
            return match[1] // first version is that of the module 
        } 
        // not for valid on this win32 / linux 
        match = platformRegexp.exec(error.message)
        if (match != null){
            // todo: @linux : use error for elfclass to determine architecture :: wrong ELF class: ELFCLASS32
            return 0 // can't validate cross platform
            // -1 is used for N-API modules
        } 
        // other error 
        console.debug( error.message)
    }
    return moduleVersion // just in case
}

// get list of files 
const walkSync = (dir, filelist = []) => {
    fs.readdirSync(dir).forEach(file => {
      filelist = fs.statSync(path.join(dir, file)).isDirectory()
        ? walkSync(path.join(dir, file), filelist)
        : filelist.concat(path.join(dir, file));
  
    });
  return filelist;
}

var abi = 0
var filename

var natives = walkSync("build/Release");
natives.forEach(filename => {
    if ( filename.endsWith('.node')){
        abi = getNativeABI(filename)
        // only show versions that we can determine
        if (abi != 0){
            console.log( `ABI ${abi} <- ${filename}`)
        }
    }
});
