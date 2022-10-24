import type { XLinkPlatform, XLinkProtocol, XLinkState, XLinkStatus } from '@luxonis/protocol';

export type DepthAIInfo = {
  version: string;
  buildDate: string;
  commit: string;
  commitDate: string;
};

export type DeviceInfo = {
  serialNumber: string;
  name: string;
  state: XLinkState;
  protocol: XLinkProtocol;
  platform: XLinkPlatform;
  status: XLinkStatus;
};

export declare function depthaiInfo(): DepthAIInfo;

export declare function scan(): Promise<DeviceInfo[]>;

export declare function release(mxId: string): Promise<void>;

export type BootStage = 'SEARCH' | 'BOOTLOADER' | 'CREATING' | 'RUNNING';

export type MonitorEvent =
  | {
      type: 'statistics';
      serialNumber: string;
      cssUsage: number;
      mssUsage: number;
      ddrMemFree: number;
      ddrMemTotal: number;
      cmxMemFree: number;
      cmxMemTotal: number;
      cssTemperature: number;
      mssTemperature: number;
      upaTemperature: number;
      dssTemperature: number;
      temperature: number;
    }
  | {
      type: 'info';
      serialNumber: string;
      state: XLinkState;
      protocol: XLinkProtocol;
      platform: XLinkPlatform;
      productName: string;
      boardName: string;
      boardRev: string;
    }
  | {
      type: 'boot';
      serialNumber: string;
      stage: BootStage;
    }
  | {
      type: 'error';
      serialNumber: string;
      message: string;
    };

export type MonitorCallback = (data: MonitorEvent) => void;

export type StreamState = 'waiting' | 'closed' | 'ready' | 'open';

export type StreamInfo = {
  clientId: string;
  deviceId: string;
  inactive: boolean;
  state: StreamState;
};

export type StreamMonitorEvent = StreamInfo[];

export type StreamMonitorCallback = (data: StreamMonitorEvent) => void;

export type IceServer = {
  hostname: string;
  port: number;
  username: string;
  password: string;
};

export declare function monitor(mxId: string, callback: MonitorCallback): Promise<boolean>;
export declare function streamMonitor(callback: StreamMonitorCallback): string;
export declare function stopStreamMonitor(monitorId: string): void;
export declare function stopStream(streamId: string): void;
export declare function requestStream(
  clientId: string,
  streamId: string,
): Promise<{ id: string; type: 'offer'; sdp: string }>;
export declare function connectStream(clientId: string, sdp: string): Promise<boolean>;
export declare function configureIceServers(servers: IceServer[]): void;
