export declare const Build: {
  VERSION: string;
  BUILD_DATETIME: string;
  COMMIT: string;
  COMMIT_DATETIME: string;
}; 

export declare type XLinkDeviceState = 'unknown' | 'booted' | 'unbooted' | 'bootloader' | 'flash-booted' | 'gate' | 'gate-booted';
export declare type XLinkProtocol = 'usb-vsc' | 'usb-cdc' | 'pcie' | 'ipc' | 'tcp-ip' | 'nmb-of-protocols' | 'unknown';
export declare type XLinkPlatform = 'unknown' | 'myriad-2' | 'myriad-x' | 'rvc3'
export declare type XLinkError = 
  | 'success' 
  | 'already-open' 
  | 'communication-not-open'
  | 'communication-fail'
  | 'communication-unknown-error'
  | 'device-not-found'
  | 'timeout'
  | 'error'
  | 'out-of-memory'
  | 'insufficient-permissions'
  | 'device-already-in-use'
  | 'not-implemented'
  | 'usb-error'
  | 'tcp-ip-error'
  | 'pcie-error';

export declare type DeviceInfo = {
  name: string;
  mxid: string;
  state: XLinkDeviceState;
  protocol: XLinkProtocol;
  platform: XLinkPlatform;
  status: XLinkError;
};

export declare const Device: {
  getAllAvailableDevices(): DeviceInfo[];
  getAllConnectedDevices(): DeviceInfo[];
};
