/*
 * creator:		'puil'
 * type:		'rsrc'
 * revision/entries:	0x01/0x15c for 5.2
 *			0x01/0x160 for 5.3+
 */
#ifndef __UIENTRYNUMS_H__
#define __UIENTRYNUMS_H__

#define entryNumAbtShowAbout			0x0	/* 0 */
#define entryNumAttnAllowClose			0x1	/* 4 */
#define entryNumAttnDoEmergencySpecialEffects	0x2	/* 8 */
#define entryNumAttnDoSpecialEffects		0x3	/* 12 */
#define entryNumAttnEffectOfEvent		0x4	/* 16 */
#define entryNumAttnEnableNotification		0x5	/* 20 */
#define entryNumAttnForgetIt			0x6	/* 24 */
#define entryNumAttnGetAttention		0x7	/* 28 */
#define entryNumAttnGetCounts			0x8	/* 32 */
#define entryNumAttnHandleEvent			0x9	/* 36 */
#define entryNumAttnIterate			0xa	/* 40 */
#define entryNumAttnListOpen			0xb	/* 44 */
#define entryNumAttnIndicatorAllow		0xc	/* 48 */
#define entryNumAttnIndicatorAllowed		0xd	/* 52 */
#define entryNumAttnIndicatorCheckBlink		0xe	/* 56 */
#define entryNumAttnIndicatorEnable		0xf	/* 60 */
#define entryNumAttnIndicatorEnabled		0x10	/* 64 */
#define entryNumAttnIndicatorGetBlinkPattern	0x11	/* 68 */
#define entryNumAttnIndicatorSetBlinkPattern	0x12	/* 72 */
#define entryNumAttnIndicatorTicksTillNextBlink	0x13	/* 76 */
#define entryNumAttnReopen			0x14	/* 80 */
#define entryNumAttnUpdate			0x15	/* 84 */
#define entryNumCategoryCreateList		0x16	/* 88 */
#define entryNumCategoryCreateListV10		0x17	/* 92 */
#define entryNumCategoryEdit			0x18	/* 96 */
#define entryNumCategoryEditV10			0x19	/* 100 */
#define entryNumCategoryEditV20			0x1a	/* 104 */
#define entryNumCategoryFind			0x1b	/* 108 */
#define entryNumCategoryFreeList		0x1c	/* 112 */
#define entryNumCategoryFreeListV10		0x1d	/* 116 */
#define entryNumCategoryGetName			0x1e	/* 120 */
#define entryNumCategoryGetNext			0x1f	/* 124 */
#define entryNumCategoryInitialize		0x20	/* 128 */
#define entryNumCategorySelect			0x21	/* 132 */
#define entryNumCategorySelectV10		0x22	/* 136 */
#define entryNumCategorySetName			0x23	/* 140 */
#define entryNumCategorySetTriggerLabel		0x24	/* 144 */
#define entryNumCategoryTruncateName		0x25	/* 148 */
#define entryNumClipboardAddItem		0x26	/* 152 */
#define entryNumClipboardAppendItem		0x27	/* 156 */
#define entryNumClipboardGetItem		0x28	/* 160 */
#define entryNumCtlDrawControl			0x29	/* 164 */
#define entryNumCtlEnabled			0x2a	/* 168 */
#define entryNumCtlEraseControl			0x2b	/* 172 */
#define entryNumCtlGetLabel			0x2c	/* 176 */
#define entryNumCtlGetSliderValues		0x2d	/* 180 */
#define entryNumCtlGetValue			0x2e	/* 184 */
#define entryNumCtlHandleEvent			0x2f	/* 188 */
#define entryNumCtlHideControl			0x30	/* 192 */
#define entryNumCtlHitControl			0x31	/* 196 */
#define entryNumCtlNewControl			0x32	/* 200 */
#define entryNumCtlNewGraphicControl		0x33	/* 204 */
#define entryNumCtlNewSliderControl		0x34	/* 208 */
#define entryNumCtlSetEnabled			0x35	/* 212 */
#define entryNumCtlSetGraphics			0x36	/* 216 */
#define entryNumCtlSetLabel			0x37	/* 220 */
#define entryNumCtlSetSliderValues		0x38	/* 224 */
#define entryNumCtlSetUsable			0x39	/* 228 */
#define entryNumCtlSetValue			0x3a	/* 232 */
#define entryNumCtlShowControl			0x3b	/* 236 */
#define entryNumCtlValidatePointer		0x3c	/* 240 */
#define entryNumDayDrawDays			0x3d	/* 244 */
#define entryNumDayDrawDaySelector		0x3e	/* 248 */
#define entryNumDayHandleEvent			0x3f	/* 252 */
#define entryNumFind				0x40	/* 256 */
#define entryNumFindDrawHeader			0x41	/* 260 */
#define entryNumFindGetLineBounds		0x42	/* 264 */
#define entryNumFindSaveMatch			0x43	/* 268 */
#define entryNumFindStrInStr			0x44	/* 272 */
#define entryNumFldCalcFieldHeight		0x45	/* 276 */
#define entryNumFldCompactText			0x46	/* 280 */
#define entryNumFldCopy				0x47	/* 284 */
#define entryNumFldCut				0x48	/* 288 */
#define entryNumFldDelete			0x49	/* 292 */
#define entryNumFldDirty			0x4a	/* 296 */
#define entryNumFldDrawField			0x4b	/* 300 */
#define entryNumFldEraseField			0x4c	/* 304 */
#define entryNumFldFreeMemory			0x4d	/* 308 */
#define entryNumFldGetAttributes		0x4e	/* 312 */
#define entryNumFldGetBounds			0x4f	/* 316 */
#define entryNumFldGetFont			0x50	/* 320 */
#define entryNumFldGetInsPtPosition		0x51	/* 324 */
#define entryNumFldGetMaxChars			0x52	/* 328 */
#define entryNumFldGetNumberOfBlankLines	0x53	/* 332 */
#define entryNumFldGetScrollPosition		0x54	/* 336 */
#define entryNumFldGetScrollValues		0x55	/* 340 */
#define entryNumFldGetSelection			0x56	/* 344 */
#define entryNumFldGetTextAllocatedSize		0x57	/* 348 */
#define entryNumFldGetTextHandle		0x58	/* 352 */
#define entryNumFldGetTextHeight		0x59	/* 356 */
#define entryNumFldGetTextLength		0x5a	/* 360 */
#define entryNumFldGetTextPtr			0x5b	/* 364 */
#define entryNumFldGetVisibleLines		0x5c	/* 368 */
#define entryNumFldGrabFocus			0x5d	/* 372 */
#define entryNumFldHandleEvent			0x5e	/* 376 */
#define entryNumFldInsert			0x5f	/* 380 */
#define entryNumFldMakeFullyVisible		0x60	/* 384 */
#define entryNumFldNewField			0x61	/* 388 */
#define entryNumFldPaste			0x62	/* 392 */
#define entryNumFldRecalculateField		0x63	/* 396 */
#define entryNumFldReleaseFocus			0x64	/* 400 */
#define entryNumFldScrollable			0x65	/* 404 */
#define entryNumFldScrollField			0x66	/* 408 */
#define entryNumFldSendChangeNotification	0x67	/* 412 */
#define entryNumFldSendHeightChangeNotification	0x68	/* 416 */
#define entryNumFldSetAttributes		0x69	/* 420 */
#define entryNumFldSetBounds			0x6a	/* 424 */
#define entryNumFldSetDirty			0x6b	/* 428 */
#define entryNumFldSetFont			0x6c	/* 432 */
#define entryNumFldSetInsertionPoint		0x6d	/* 436 */
#define entryNumFldSetInsPtPosition		0x6e	/* 440 */
#define entryNumFldSetMaxChars			0x6f	/* 444 */
#define entryNumFldSetMaxVisibleLines		0x70	/* 448 */
#define entryNumFldSetScrollPosition		0x71	/* 452 */
#define entryNumFldSetSelection			0x72	/* 456 */
#define entryNumFldSetText			0x73	/* 460 */
#define entryNumFldSetTextAllocatedSize		0x74	/* 464 */
#define entryNumFldSetTextHandle		0x75	/* 468 */
#define entryNumFldSetTextPtr			0x76	/* 472 */
#define entryNumFldSetUsable			0x77	/* 476 */
#define entryNumFldUndo				0x78	/* 480 */
#define entryNumFldWordWrap			0x79	/* 484 */
#define entryNumFontSelect			0x7a	/* 488 */
#define entryNumFrmActiveState			0x7b	/* 492 */
#define entryNumFrmAddSpaceForObject		0x7c	/* 496 */
#define entryNumFrmAlert			0x7d	/* 500 */
#define entryNumFrmCloseAllForms		0x7e	/* 504 */
#define entryNumFrmCopyLabel			0x7f	/* 508 */
#define entryNumFrmCopyTitle			0x80	/* 512 */
#define entryNumFrmCustomAlert			0x81	/* 516 */
#define entryNumFrmCustomResponseAlert		0x82	/* 520 */
#define entryNumFrmDeleteForm			0x83	/* 524 */
#define entryNumFrmDispatchEvent		0x84	/* 528 */
#define entryNumFrmDoDialog			0x85	/* 532 */
#define entryNumFrmDrawForm			0x86	/* 536 */
#define entryNumFrmEraseForm			0x87	/* 540 */
#define entryNumFrmGetActiveField		0x88	/* 544 */
#define entryNumFrmGetActiveForm		0x89	/* 548 */
#define entryNumFrmGetActiveFormID		0x8a	/* 552 */
#define entryNumFrmGetControlGroupSelection	0x8b	/* 556 */
#define entryNumFrmGetControlValue		0x8c	/* 560 */
#define entryNumFrmGetFirstForm			0x8d	/* 564 */
#define entryNumFrmGetFocus			0x8e	/* 568 */
#define entryNumFrmGetFormBounds		0x8f	/* 572 */
#define entryNumFrmGetFormId			0x90	/* 576 */
#define entryNumFrmGetFormPtr			0x91	/* 580 */
#define entryNumFrmGetGadgetData		0x92	/* 584 */
#define entryNumFrmGetLabel			0x93	/* 588 */
#define entryNumFrmGetNumberOfObjects		0x94	/* 592 */
#define entryNumFrmGetObjectBounds		0x95	/* 596 */
#define entryNumFrmGetObjectId			0x96	/* 600 */
#define entryNumFrmGetObjectIndex		0x97	/* 604 */
#define entryNumFrmGetObjectIndexFromPtr	0x98	/* 608 */
#define entryNumFrmGetObjectPosition		0x99	/* 612 */
#define entryNumFrmGetObjectPtr			0x9a	/* 616 */
#define entryNumFrmGetObjectType		0x9b	/* 620 */
#define entryNumFrmGetTitle			0x9c	/* 624 */
#define entryNumFrmGetUserModifiedState		0x9d	/* 628 */
#define entryNumFrmGetWindowHandle		0x9e	/* 632 */
#define entryNumFrmGotoForm			0x9f	/* 636 */
#define entryNumFrmHandleEvent			0xa0	/* 640 */
#define entryNumFrmHelp				0xa1	/* 644 */
#define entryNumFrmHideObject			0xa2	/* 648 */
#define entryNumFrmInitForm			0xa3	/* 652 */
#define entryNumFrmNewBitmap			0xa4	/* 656 */
#define entryNumFrmNewForm			0xa5	/* 660 */
#define entryNumFrmNewGadget			0xa6	/* 664 */
#define entryNumFrmNewGsi			0xa7	/* 668 */
#define entryNumFrmNewLabel			0xa8	/* 672 */
#define entryNumFrmPointInTitle			0xa9	/* 676 */
#define entryNumFrmPopupForm			0xaa	/* 680 */
#define entryNumFrmRemoveObject			0xab	/* 684 */
#define entryNumFrmReturnToForm			0xac	/* 688 */
#define entryNumFrmSaveAllForms			0xad	/* 692 */
#define entryNumFrmSetActiveForm		0xae	/* 696 */
#define entryNumFrmSetCategoryLabel		0xaf	/* 700 */
#define entryNumFrmSetControlGroupSelection	0xb0	/* 704 */
#define entryNumFrmSetControlValue		0xb1	/* 708 */
#define entryNumFrmSetEventHandler		0xb2	/* 712 */
#define entryNumFrmSetFocus			0xb3	/* 716 */
#define entryNumFrmSetGadgetData		0xb4	/* 720 */
#define entryNumFrmSetGadgetHandler		0xb5	/* 724 */
#define entryNumFrmSetMenu			0xb6	/* 728 */
#define entryNumFrmSetNotUserModified		0xb7	/* 732 */
#define entryNumFrmSetObjectBounds		0xb8	/* 736 */
#define entryNumFrmSetObjectPosition		0xb9	/* 740 */
#define entryNumFrmSetTitle			0xba	/* 744 */
#define entryNumFrmShowObject			0xbb	/* 748 */
#define entryNumFrmUpdateForm			0xbc	/* 752 */
#define entryNumFrmUpdateScrollers		0xbd	/* 756 */
#define entryNumFrmValidatePtr			0xbe	/* 760 */
#define entryNumFrmVisible			0xbf	/* 764 */
#define entryNumGsiEnable			0xc0	/* 768 */
#define entryNumGsiEnabled			0xc1	/* 772 */
#define entryNumGsiInitialize			0xc2	/* 776 */
#define entryNumGsiSetLocation			0xc3	/* 780 */
#define entryNumGsiSetShiftState		0xc4	/* 784 */
#define entryNumInsPtCheckBlink			0xc5	/* 788 */
#define entryNumInsPtEnable			0xc6	/* 792 */
#define entryNumInsPtEnabled			0xc7	/* 796 */
#define entryNumInsPtGetHeight			0xc8	/* 800 */
#define entryNumInsPtGetLocation		0xc9	/* 804 */
#define entryNumInsPtInitialize			0xca	/* 808 */
#define entryNumInsPtSetHeight			0xcb	/* 812 */
#define entryNumInsPtSetLocation		0xcc	/* 816 */
#define entryNumKbdDraw				0xcd	/* 820 */
#define entryNumKbdErase			0xce	/* 824 */
#define entryNumKbdGetLayout			0xcf	/* 828 */
#define entryNumKbdGetPosition			0xd0	/* 832 */
#define entryNumKbdGetShiftState		0xd1	/* 836 */
#define entryNumKbdHandleEvent			0xd2	/* 840 */
#define entryNumKbdSetLayout			0xd3	/* 844 */
#define entryNumKbdSetPosition			0xd4	/* 848 */
#define entryNumKbdSetShiftState		0xd5	/* 852 */
#define entryNumKeyboardStatusFree		0xd6	/* 856 */
#define entryNumKeyboardStatusNew		0xd7	/* 860 */
#define entryNumLstDrawList			0xd8	/* 864 */
#define entryNumLstEraseList			0xd9	/* 868 */
#define entryNumLstGetNumberOfItems		0xda	/* 872 */
#define entryNumLstGetSelection			0xdb	/* 876 */
#define entryNumLstGetSelectionText		0xdc	/* 880 */
#define entryNumLstGetTopItem			0xdd	/* 884 */
#define entryNumLstGetVisibleItems		0xde	/* 888 */
#define entryNumLstHandleEvent			0xdf	/* 892 */
#define entryNumLstMakeItemVisible		0xe0	/* 896 */
#define entryNumLstNewList			0xe1	/* 900 */
#define entryNumLstPopupList			0xe2	/* 904 */
#define entryNumLstScrollList			0xe3	/* 908 */
#define entryNumLstSetDrawFunction		0xe4	/* 912 */
#define entryNumLstSetHeight			0xe5	/* 916 */
#define entryNumLstSetListChoices		0xe6	/* 920 */
#define entryNumLstSetPosition			0xe7	/* 924 */
#define entryNumLstSetSelection			0xe8	/* 928 */
#define entryNumLstSetTopItem			0xe9	/* 932 */
#define entryNumMenuAddItem			0xea	/* 936 */
#define entryNumMenuCmdBarAddButton		0xeb	/* 940 */
#define entryNumMenuCmdBarDisplay		0xec	/* 944 */
#define entryNumMenuCmdBarGetButtonData		0xed	/* 948 */
#define entryNumMenuDispose			0xee	/* 952 */
#define entryNumMenuDrawMenu			0xef	/* 956 */
#define entryNumMenuEraseMenu			0xf0	/* 960 */
#define entryNumMenuEraseStatus			0xf1	/* 964 */
#define entryNumMenuGetActiveMenu		0xf2	/* 968 */
#define entryNumMenuHandleEvent			0xf3	/* 972 */
#define entryNumMenuHideItem			0xf4	/* 976 */
#define entryNumMenuInit			0xf5	/* 980 */
#define entryNumMenuSetActiveMenu		0xf6	/* 984 */
#define entryNumMenuSetActiveMenuRscID		0xf7	/* 988 */
#define entryNumMenuShowItem			0xf8	/* 992 */
#define entryNumPhoneNumberLookup		0xf9	/* 996 */
#define entryNumPhoneNumberLookupCustom		0xfa	/* 1000 */
#define entryNumPrgHandleEvent			0xfb	/* 1004 */
#define entryNumPrgStartDialog			0xfc	/* 1008 */
#define entryNumPrgStartDialogV31		0xfd	/* 1012 */
#define entryNumPrgStopDialog			0xfe	/* 1016 */
#define entryNumPrgUpdateDialog			0xff	/* 1020 */
#define entryNumResLoadConstant			0x100	/* 1024 */
#define entryNumResLoadForm			0x101	/* 1028 */
#define entryNumResLoadMenu			0x102	/* 1032 */
#define entryNumSclDrawScrollBar		0x103	/* 1036 */
#define entryNumSclGetScrollBar			0x104	/* 1040 */
#define entryNumSclHandleEvent			0x105	/* 1044 */
#define entryNumSclSetScrollBar			0x106	/* 1048 */
#define entryNumSecSelectViewStatus		0x107	/* 1052 */
#define entryNumSecVerifyPW			0x108	/* 1056 */
#define entryNumSelectDay			0x109	/* 1060 */
#define entryNumSelectDayV10			0x10a	/* 1064 */
#define entryNumSelectOneTime			0x10b	/* 1068 */
#define entryNumSelectTime			0x10c	/* 1072 */
#define entryNumSelectTimeV33			0x10d	/* 1076 */
#define entryNumSelectTimeZone			0x10e	/* 1080 */
#define entryNumSysAppLauncherDialog		0x10f	/* 1084 */
#define entryNumSysBatteryDialog		0x110	/* 1088 */
#define entryNumSysFatalAlert			0x111	/* 1092 */
#define entryNumSysFatalAlertInit		0x112	/* 1096 */
#define entryNumSysKeyboardDialog		0x113	/* 1100 */
#define entryNumSysKeyboardDialogV10		0x114	/* 1104 */
#define entryNumTblDrawTable			0x115	/* 1108 */
#define entryNumTblEditing			0x116	/* 1112 */
#define entryNumTblEraseTable			0x117	/* 1116 */
#define entryNumTblFindRowData			0x118	/* 1120 */
#define entryNumTblFindRowID			0x119	/* 1124 */
#define entryNumTblGetBounds			0x11a	/* 1128 */
#define entryNumTblGetColumnSpacing		0x11b	/* 1132 */
#define entryNumTblGetColumnWidth		0x11c	/* 1136 */
#define entryNumTblGetCurrentField		0x11d	/* 1140 */
#define entryNumTblGetItemBounds		0x11e	/* 1144 */
#define entryNumTblGetItemFont			0x11f	/* 1148 */
#define entryNumTblGetItemInt			0x120	/* 1152 */
#define entryNumTblGetItemPtr			0x121	/* 1156 */
#define entryNumTblGetLastUsableRow		0x122	/* 1160 */
#define entryNumTblGetNumberOfRows		0x123	/* 1164 */
#define entryNumTblGetNumberOfColumns		0x124	/* 1168 */
#define entryNumTblGetRowData			0x125	/* 1172 */
#define entryNumTblGetRowHeight			0x126	/* 1176 */
#define entryNumTblGetRowID			0x127	/* 1180 */
#define entryNumTblGetSelection			0x128	/* 1184 */
#define entryNumTblGetTopRow			0x129	/* 1188 */
#define entryNumTblGrabFocus			0x12a	/* 1192 */
#define entryNumTblHandleEvent			0x12b	/* 1196 */
#define entryNumTblHasScrollBar			0x12c	/* 1200 */
#define entryNumTblInsertRow			0x12d	/* 1204 */
#define entryNumTblMarkRowInvalid		0x12e	/* 1208 */
#define entryNumTblMarkTableInvalid		0x12f	/* 1212 */
#define entryNumTblRedrawTable			0x130	/* 1216 */
#define entryNumTblReleaseFocus			0x131	/* 1220 */
#define entryNumTblRemoveRow			0x132	/* 1224 */
#define entryNumTblRowInvalid			0x133	/* 1228 */
#define entryNumTblRowMasked			0x134	/* 1232 */
#define entryNumTblRowSelectable		0x135	/* 1236 */
#define entryNumTblRowUsable			0x136	/* 1240 */
#define entryNumTblSelectItem			0x137	/* 1244 */
#define entryNumTblSetBounds			0x138	/* 1248 */
#define entryNumTblSetColumnEditIndicator	0x139	/* 1252 */
#define entryNumTblSetColumnMasked		0x13a	/* 1256 */
#define entryNumTblSetColumnSpacing		0x13b	/* 1260 */
#define entryNumTblSetColumnUsable		0x13c	/* 1264 */
#define entryNumTblSetColumnWidth		0x13d	/* 1268 */
#define entryNumTblSetCustomDrawProcedure	0x13e	/* 1272 */
#define entryNumTblSetItemFont			0x13f	/* 1276 */
#define entryNumTblSetItemInt			0x140	/* 1280 */
#define entryNumTblSetItemPtr			0x141	/* 1284 */
#define entryNumTblSetItemStyle			0x142	/* 1288 */
#define entryNumTblSetLoadDataProcedure		0x143	/* 1292 */
#define entryNumTblSetRowData			0x144	/* 1296 */
#define entryNumTblSetRowHeight			0x145	/* 1300 */
#define entryNumTblSetRowID			0x146	/* 1304 */
#define entryNumTblSetRowMasked			0x147	/* 1308 */
#define entryNumTblSetRowSelectable		0x148	/* 1312 */
#define entryNumTblSetRowStaticHeight		0x149	/* 1316 */
#define entryNumTblSetRowUsable			0x14a	/* 1320 */
#define entryNumTblSetSaveDataProcedure		0x14b	/* 1324 */
#define entryNumTblSetSelection			0x14c	/* 1328 */
#define entryNumTblUnhighlightSelection		0x14d	/* 1332 */
#define entryNumTimeZoneToAscii			0x14e	/* 1336 */
#define entryNumUIBrightnessAdjust		0x14f	/* 1340 */
#define entryNumUIColorGetTableEntryIndex	0x150	/* 1344 */
#define entryNumUIColorGetTableEntryRGB		0x151	/* 1348 */
#define entryNumUIColorInit			0x152	/* 1352 */
#define entryNumUIColorPopTable			0x153	/* 1356 */
#define entryNumUIColorPushTable		0x154	/* 1360 */
#define entryNumUIColorSetTableEntry		0x155	/* 1364 */
#define entryNumUIContrastAdjust		0x156	/* 1368 */
#define entryNumUIInitialize			0x157	/* 1372 */
#define entryNumUIPickColor			0x158	/* 1376 */
#define entryNumUIGetGlobalsPtr			0x159	/* 1380 */
#define entryNumUIReset				0x15a	/* 1384 */
#define entryNumAttnInitialize			0x15b	/* 1388 */

/* PalmOS 5.3+ traps */
#define entryNumFrmGetDIAPolicyAttr		0x15c	/* 1392 */
#define entryNumFrmSetDIAPolicyAttr		0x15d	/* 1396 */
#define entryNumFrmPrvSetActiveFormPINAttributes 0x15e	/* 1400 */
#define entryNumFrmPrvRedrawDisplay		0x15f	/* 1404 */

#endif
