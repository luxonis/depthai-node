# depthai-node

## Prepare crosscompile
```bash
sudo apt update
sudo apt install -y g++-aarch64-linux-gnu gcc-aarch64-linux-gnu
mkdir sysroot && cd sysroot
wget https://commondatastorage.googleapis.com/chrome-linux-sysroot/toolchain/953c2471bc7e71a788309f6c2d2003e8b703305d/debian_sid_arm64_sysroot.tar.xz        
tar xf debian_sid_arm64_sysroot.tar.xz
export ARM64_SYSROOT="$(pwd)"
```