/*
 * creator:		'pdal'
 * type:		'rsrc'
 * revision/entries:	0x01/0xc3 for 5.0
 *			0x02/0xcf for 5.2
 *			0x02/0xf0 for 5.4
 */
#ifndef __DALENTRYNUMS_H__
#define __DALENTRYNUMS_H__

#define entryNumDALGetGlobalsPtr		0x00	/* 0 */
#define entryNumHALDbgBreak			0x01	/* 4 */
#define entryNumHALDbgMessage			0x02	/* 8 */
#define entryNumHALAttnGetLEDAttributes		0x03	/* 12 */
#define entryNumHALAttnGetVibrateAttributes	0x04	/* 16 */
#define entryNumHALAttnSetLEDAttributes		0x05	/* 20 */
#define entryNumHALAttnSetVibrateAttributes	0x06	/* 24 */
#define entryNumHALBatteryGetInfo		0x07	/* 28 */
#define entryNumHALBatteryGetValidKinds		0x08	/* 32 */
#define entryNumHALBatterySetInfo		0x09	/* 36 */
#define entryNumHALDelay			0x0a	/* 40 */
#define entryNumHALDisplayGetAttributes		0x0b	/* 44 */
#define entryNumHALDisplayGetPalette		0x0c	/* 48 */
#define entryNumHALDisplaySetAttributes		0x0d	/* 52 */
#define entryNumHALDisplaySetPalette		0x0e	/* 56 */
#define entryNumHALDisplayDoze			0x0f	/* 60 */
#define entryNumHALDisplayWake			0x10	/* 64 */
#define entryNumHALDisplayLock			0x11	/* 68 */
#define entryNumHALDisplayUnlock		0x12	/* 72 */
#define entryNumHALDockStatus			0x13	/* 76 */
#define entryNumHALDrawInit			0x14	/* 80 */
#define entryNumHALDrawCopyRectangle		0x15	/* 84 */
#define entryNumHALDrawFindIndexes		0x16	/* 88 */
#define entryNumHALDrawGetPixel			0x17	/* 92 */
#define entryNumHALDrawChars			0x18	/* 96 */
#define entryNumHALDrawLine			0x19	/* 100 */
#define entryNumHALDrawRectangle		0x1a	/* 104 */
#define entryNumHALDrawSetPixel			0x1b	/* 108 */
#define entryNumHALDrawSetPixels		0x1c	/* 112 */
#define entryNumHALErrDisplay			0x1d	/* 116 */
#define entryNumHALEventPost			0x1e	/* 120 */
#define entryNumHALEventRegisterCallBack	0x1f	/* 124 */
#define entryNumHALFlpAddDouble			0x20	/* 128 */
#define entryNumHALFlpULongLongToFloat		0x43	/* 268 */
#define entryNumHALGetHwrMiscFlags		0x44	/* 272 */
#define entryNumHALGetHwrMiscFlagsExt		0x45	/* 276 */
#define entryNumHALGetHwrWakeUp			0x46	/* 280 */
#define entryNumHALInterruptAllSetStatus	0x47	/* 284 */
#define entryNumHALInterruptAllGetStatus	0x48	/* 288 */
#define entryNumHALInterruptGetHandler		0x49	/* 292 */
#define entryNumHALInterruptSetHandler		0x4a	/* 296 */
#define entryNumHALInterruptSetState		0x4b	/* 300 */
#define entryNumHALKeyGetRates			0x4c	/* 304 */
#define entryNumHALKeyGetState			0x4d	/* 308 */
#define entryNumHALKeyResetDoubleTap		0x4e	/* 312 */
#define entryNumHALKeySetMask			0x4f	/* 316 */
#define entryNumHALKeySetRates			0x50	/* 320 */
#define entryNumHALMemoryGetMemoryMap		0x51	/* 324 */
#define entryNumHALMemoryGetStorageAreaProtectionState	0x52	/* 328 */
#define entryNumHALMemorySetStorageAreaProtectionState	0x53	/* 332 */
#define entryNumHALMemoryValidateArea		0x54	/* 336 */
#define entryNumHALMemoryGetAreaSize		0x55	/* 340 */
#define entryNumHALPenCalibrate			0x56	/* 344 */
#define entryNumHALPenRawToScreen		0x57	/* 348 */
#define entryNumHALPenResetCalibration		0x58	/* 352 */
#define entryNumHALPenGetPosition		0x59	/* 356 */
#define entryNumHALPenScreenToRaw		0x5a	/* 360 */
#define entryNumHALPowerGetAutoOffEvtTime	0x5b	/* 364 */
#define entryNumHALPowerGetAutoOffSeconds	0x5c	/* 368 */
#define entryNumHALPowerSetAutoOffEvtTime	0x5d	/* 372 */
#define entryNumHALPowerSetAutoOffSeconds	0x5e	/* 376 */
#define entryNumHALPowerSleepReady		0x5f	/* 380 */
#define entryNumHALProcessorID			0x60	/* 384 */
#define entryNumHALRandomGetBits		0x61	/* 388 */
#define entryNumHALRandomInitializeSeed		0x62	/* 392 */
#define entryNumHALReset			0x63	/* 396 */
#define entryNumHALScreenDefaultPalette		0x64	/* 400 */
#define entryNumHALScreenDrawNotify		0x65	/* 404 */
#define entryNumHALScreenGetColortable		0x66	/* 408 */
#define entryNumHALScreenInit			0x67	/* 412 */
#define entryNumHALScreenLock			0x68	/* 416 */
#define entryNumHALScreenPalette		0x69	/* 420 */
#define entryNumHALScreenSendUpdateArea		0x6a	/* 424 */
#define entryNumHALScreenUnlock			0x6b	/* 428 */
#define entryNumHALScreenUpdateBitmap		0x6c	/* 432 */
#define entryNumHALSetInitStage			0x6d	/* 436 */
#define entryNumHALSetHwrMiscFlags		0x6e	/* 440 */
#define entryNumHALSilkscreenGetID		0x6f	/* 444 */
#define entryNumHALSoundOff			0x70	/* 448 */
#define entryNumHALSoundPlay			0x71	/* 452 */
#define entryNumHALTimeGetAlarm			0x72	/* 456 */
#define entryNumHALTimeGetSeconds		0x73	/* 460 */
#define entryNumHALTimeGetSystemTime		0x74	/* 464 */
#define entryNumHALTimeGetSystemTimerInterval	0x75	/* 468 */
#define entryNumHALTimeSetAlarm			0x76	/* 472 */
#define entryNumHALTimeSetSeconds		0x77	/* 476 */
#define entryNumHALTraceInit			0x78	/* 480 */
#define entryNumHALTraceClose			0x79	/* 484 */
#define entryNumHALTraceOutputT			0x7a	/* 488 */
#define entryNumHALTraceOutputTL		0x7b	/* 492 */
#define entryNumHALTraceOutputVT		0x7c	/* 496 */
#define entryNumHALTraceOutputVTL		0x7d	/* 500 */
#define entryNumHALTraceOutputB			0x7e	/* 504 */
#define entryNumHALOEMGetCompanyID		0x7f	/* 508 */
#define entryNumHALOEMGetHALID			0x80	/* 512 */
#define entryNumHALOEMGetDeviceID		0x81	/* 516 */
#define entryNumHwrCalcDynamicHeapSize		0x82	/* 520 */
#define entryNumHwrCalcDynamicRAMSize		0x83	/* 524 */
#define entryNumHALDisplayDrawBootScreen	0x84	/* 528 */
#define entryNumHALGetROMToken			0x85	/* 532 */
#define entryNumKALEventGroupClear		0x86	/* 536 */
#define entryNumKALEventGroupCreate		0x87	/* 540 */
#define entryNumKALEventGroupDelete		0x88	/* 544 */
#define entryNumKALEventGroupRead		0x89	/* 548 */
#define entryNumKALEventGroupSignal		0x8a	/* 552 */
#define entryNumKALEventGroupWait		0x8b	/* 556 */
#define entryNumKALEventGroupGetInfo		0x8c	/* 560 */
#define entryNumKALMailboxCreate		0x8d	/* 564 */
#define entryNumKALMailboxDelete		0x8e	/* 568 */
#define entryNumKALMailboxSend			0x8f	/* 572 */
#define entryNumKALMailboxWait			0x90	/* 576 */
#define entryNumKALMailboxGetInfo		0x91	/* 580 */
#define entryNumKALMutexCreate			0x92	/* 584 */
#define entryNumKALMutexDelete			0x93	/* 588 */
#define entryNumKALMutexRelease			0x94	/* 592 */
#define entryNumKALMutexReserve			0x95	/* 596 */
#define entryNumKALMutexGetInfo			0x96	/* 600 */
#define entryNumKALSemaphoreCreate		0x97	/* 604 */
#define entryNumKALSemaphoreDelete		0x98	/* 608 */
#define entryNumKALSemaphoreSignal		0x99	/* 612 */
#define entryNumKALSemaphoreWait		0x9a	/* 616 */
#define entryNumKALSemaphoreGetInfo		0x9b	/* 620 */
#define entryNumKALTaskCreate			0x9c	/* 624 */
#define entryNumKALTaskDelay			0x9d	/* 628 */
#define entryNumKALTaskDelete			0x9e	/* 632 */
#define entryNumKALTaskGetCurrentID		0x9f	/* 636 */
#define entryNumKALTaskGetInfo			0xa0	/* 640 */
#define entryNumKALTaskResume			0xa1	/* 644 */
#define entryNumKALTaskSuspend			0xa2	/* 648 */
#define entryNumKALTaskStart			0xa3	/* 652 */
#define entryNumKALTaskSwitching		0xa4	/* 656 */
#define entryNumKALTaskWait			0xa5	/* 660 */
#define entryNumKALTaskWaitClr			0xa6	/* 664 */
#define entryNumKALTaskWake			0xa7	/* 668 */
#define entryNumKALTaskExit			0xa8	/* 672 */
#define entryNumKALTimerCreate			0xa9	/* 676 */
#define entryNumKALTimerDelete			0xaa	/* 680 */
#define entryNumKALTimerSet			0xab	/* 684 */
#define entryNumKALTimerGetInfo			0xac	/* 688 */
#define entryNumRALCallWithNewStack		0xad	/* 692 */
#define entryNumRALGetEntryAdresses		0xae	/* 696 */
#define entryNumRALGetModuleInfo		0xaf	/* 700 */
#define entryNumRALGetStaticBase		0xb0	/* 704 */
#define entryNumRALLinkClient			0xb1	/* 708 */
#define entryNumRALLoadModule			0xb2	/* 712 */
#define entryNumRALPatchEntry			0xb3	/* 716 */
#define entryNumRALSetA5			0xb4	/* 720 */
#define entryNumRALUnloadModule			0xb5	/* 724 */
#define entryNumRALUnloadNext			0xb6	/* 728 */
#define entryNumDALSetDbgLockout		0xb7	/* 732 */
#define entryNumHALKeyGetDoubleTapDelay		0xb8	/* 736 */
#define entryNumHALKeySetDoubleTapDelay		0xb9	/* 740 */
#define entryNumHALInvalidateICache		0xba	/* 744 */
#define entryNumHALDraw_BitmapV50		0xbc	/* 752 */
#define entryNumHALDraw_CharsV50		0xbd	/* 756 */
#define entryNumHALDraw_FindIndexesV50		0xbe	/* 760 */
#define entryNumHALDraw_GetPixelV50		0xbf	/* 764 */
#define entryNumHALDraw_LineV50			0xc0	/* 768 */
#define entryNumHALDraw_PixelsV50		0xc1	/* 772 */
#define entryNumHALDraw_RectangleV50		0xc2	/* 776 */

/* PalmOS 5.1+ entry points */
#define entryNumHALDraw_Bitmap			0xbe	/* 760 */
#define entryNumHALDraw_FindIndexes		0xbf	/* 764 */
#define entryNumHALDraw_GetPixel		0xc0	/* 768 */
#define entryNumHALDraw_Chars			0xc1	/* 772 */
#define entryNumHALDraw_Line			0xc2	/* 776 */
#define entryNumHALDraw_Rectangle		0xc3	/* 780 */
#define entryNumHALDraw_Pixels			0xc4	/* 784 */
#define entryNumHALSoundOpen			0xc5	/* 788 */
#define entryNumHALSoundClose			0xc6	/* 792 */
#define entryNumHALSoundRead			0xc7	/* 796 */
#define entryNumHALSoundWrite			0xc8	/* 800 */
#define entryNumHALSoundIoctl			0xc9	/* 804 */
#define entryNumHALSoundInitialize		0xca	/* 808 */
#define entryNumHALSoundDispose			0xcb	/* 812 */
#define entryNumdprintf				0xcc	/* 816 */
#define entryNumHALDraw_GetSupportedDensity	0xcd	/* 820 */
#define entryNumHALPlaySmf			0xce	/* 824 */
#define entryNumOEMReserved01			0xcf	/* 828 */
#define entryNumOEMReserved02			0xd0	/* 832 */
#define entryNumOEMReserved03			0xd1
#define entryNumOEMReserved04			0xd2
#define entryNumOEMReserved05			0xd3
#define entryNumOEMReserved06			0xd4
#define entryNumOEMReserved07			0xd5
#define entryNumHALDisplaySleep			0xd6	/* 856 */
#define entryNumOEMReserved08			0xd7
#define entryNumOEMReserved09			0xd8
#define entryNumOEMReserved10			0xd9
#define entryNumOEMReserved11			0xda
#define entryNumOEMReserved12			0xdb
#define entryNumOEMReserved13			0xdc
#define entryNumOEMReserved14			0xdd
#define entryNumOEMReserved15			0xde
#define entryNumOEMReserved16			0xdf
#define entryNumOEMReserved17			0xe0
#define entryNumOEMReserved18			0xe1
#define entryNumOEMReserved19			0xe2
#define entryNumOEMReserved20			0xe3
#define entryNumOEMReserved21			0xe4
#define entryNumOEMReserved22			0xe5
#define entryNumOEMReserved23			0xe6
#define entryNumOEMReserved24			0xe7
#define entryNumOEMReserved25			0xe8
#define entryNumOEMReserved26			0xe9
#define entryNumOEMReserved27			0xea
#define entryNumOEMReserved28			0xeb
#define entryNumOEMReserved29			0xec
#define entryNumOEMReserved30			0xed
#define entryNumOEMReserved31			0xee
#define entryNumOEMReserved32			0xef

/* PalmOS 5.3+ entry points */
#define entryNumHALRedrawInputArea		0xf0

/* Zire 71? */
#define entryNumHALAddressConvertPhysicalToVirtual	0xcf	/* 828 */
#define entryNumHALAddressConvertVirtualToPhysical	0xd0	/* 832 */
#define entryNumHALDSPMemorySpace		0xd1
#define entryNumHALi2cSingleWrite		0xd2
#define entryNumHALi2cBurstWrite		0xd3
#define entryNumHALGpioGetInput			0xd4
#define entryNumHALGpioSetOutput		0xd5
#define entryNumHALGpioConfigureDirection	0xd6
#define entryNumHALGpioGetDirection		0xd7
#define entryNumHALGpioGetOutput		0xd8
#define entryNumHALARMIOConfigureDirection	0xda
#define entryNumHALARMIOGetInput		0xdb
#define entryNumHALARMIOSetOutput		0xdc
#define entryNumHALARMIOConfigureIntEdge	0xdd

/* Sony CLIE NX/NZ/TG specific entry points */
#define entryNumHALScheduleBottomHalfV50	0xc3	/* 780 */

/* Sony CLIE UX/TH/VZ specific entry points */
#define entryNumHALScheduleBottomHalf		0xcf	/* 828 */
#define entryNumDAL_832				0xd0	/* 832: swi 0xba */
#define entryNumDAL_836				0xd1	/* 836: swi 0xbb */
#define entryNumDAL_840				0xd2	/* 840: swi 0xbc */
#define entryNumDAL_844				0xd3	/* 844: swi 0xbd */
#define entryNumHALAddressVirtualToPhysical	0xd4	/* 848: swi 0xbe */
#define entryNumDAL_852				0xd5	/* 852: swi 0xbf */

/* Garmin iQue 3200 (?), iQue 3600 specific entry points */
#define entryNumHALGetSystemFreq		0xcf	/* 828 */
#define entryNumPrvVirtualtoPhysicalAddr	0xd0	/* 832 */
#define entryNumHwrGpioSetHandler		0xd1	/* 836 */
#define entryNumHwrGpioGetHandler		0xd2	/* 840 */
#define entryNumHwrGpioSetSignalMode		0xd3	/* 844 */
#define entryNumHwrGpioSetSignalState		0xd4	/* 848 */
#define entryNumHwrGpioGetSignalState		0xd5	/* 852 */
#define entryNumHwrGpioSetIntMode		0xd6	/* 856 */
#define entryNumHwrGpioSetIntState		0xd7	/* 860 */
#define entryNumHwrGpioGetIntState		0xd8	/* 864 */
#define entryNumHwrGpioClearInt			0xd9	/* 868 */
#define entryNumI2CWrite			0xda	/* 872 */
#define entryNumI2CRead				0xdb	/* 876 */
#define entryNumI2CSetDeviceAddress		0xdc	/* 880 */
#define entryNumI2CGetDeviceAddress		0xdd	/* 884 */
#define entryNumI2CSetDeviceSpeed		0xde	/* 888 */
#define entryNumI2CGetDeviceSpeed		0xdf	/* 892 */
#define entryNumI2CReset			0xe0	/* 896 */
#define entryNumI2CTransfer			0xe1	/* 900 */
#define entryNumI2CEnableSlaveTransfer		0xe2	/* 904 */
#define entryNumI2CDisableSlaveTransfer		0xe3	/* 908 */
#define entryNumI2CIsSlaveEnabled		0xe4	/* 912 */
#define entryNumHWMDalEntry			0xe5	/* 916 */

/* palmOne T|T3 */

/* palmOne Zire 72, T|T5, Treo 650 specific entry points */
#define entryNumHALHRTimerTicksPerSecond	0xd9	/* 868 */
#define entryNumHALHRTimerGetTicks		0xda	/* 872 */

/* Tapwave Zodiac */
/* NDA */

#endif
