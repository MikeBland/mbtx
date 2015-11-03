#ifndef _VOICE_H_
#define _VOICE_H_

/* voice card related stuffs */

// voice queue masks
#define VQ_FNMASK     0x3FFF  // filenum mask for 4 digit decimal (0000-9999)
#define VQ_CMDMASK    0xF000  // short command mask

// upto 12 commands (0x4000,..,0xF000) and each cmd can carry upto 12b data
#define VQ_VOLUME     0x4000  // volume command + volume (0-7)
#define VQ_CONFIG     0x5000  // config command + 8b config info
  #define VB_MEGASOUND  0x01  // serial MegaSound card installed
  #define VB_BACKLIGHT  0x02  // let XPB1 controls BACKLIGHT
  #define VB_TRIM_LV    0x04  // TRIM_ON_DATA?
  #define VB_RECEIVED   0x80  // received @cop328

// serial voice commands
// er9x to cop328
#define VCMD_PLAY       0x1F  // voice file playback command
#define VCMD_BACKLIGHT  0x1D  // backlight on-off command
#define VCMD_BOOTREASON 0x1B  // BootReason command
  #define VOP_BACKUP    0x1B  // BootReason: model backup-restore operation
  #define VOP_UNKNOWN   0x1C  // BootReason: unknown
#define VCMD_GOBOOT     0x30  // goto bootloader command
  #define VOP_GOBOOT    0x20  // operand of goto bootloader (VCMD_GOBOOT)

// cop328 to er9x
#define XCMD_STATUS     0x1F  // (MSB:Busy,XPD7,XPC0,XPB1,XPD4,XPD3,XPD2,XPB0)
#define XCMD_FEATURE    0x1E  // MegaSound firmware features (8b)
  #define VC_MBACKUP    0x01  // model backup feature
  #define VC_RTC        0x02  // RTC DS1302(CE:XPC1/XPB2,SCLK:XPC2,SIO:XPC3)
  #define VC_DATALOG    0x04  // data logging feature
  #define VC_LSLIDER    0x10  // left-slider (XPC0/XPC2)
  #define VC_RSLIDER    0x20  // right-slider (XPC1)
  #define VC_CHANGED    0x80  // feature changed @cop328
  #define VC_RECEIVED   0x80  // received @er9x

#endif  // _VOICE_H_
