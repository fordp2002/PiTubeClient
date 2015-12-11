// swi.h

#ifndef SWI_H
#define SWI_H

#define SWI_OS_WriteC                  asm volatile ("SWI 0x000000")
#define SWI_OS_WriteS                  asm volatile ("SWI 0x000001")
#define SWI_OS_Write0                  asm volatile ("SWI 0x000002")
#define SWI_OS_NewLine                 asm volatile ("SWI 0x000003")
#define SWI_OS_ReadC                   asm volatile ("SWI 0x000004")
#define SWI_OS_CLI                     asm volatile ("SWI 0x000005")
#define SWI_OS_Byte                    asm volatile ("SWI 0x000006")
#define SWI_OS_Word                    asm volatile ("SWI 0x000007")
#define SWI_OS_File                    asm volatile ("SWI 0x000008")
#define SWI_OS_Args                    asm volatile ("SWI 0x000009")
#define SWI_OS_BGet                    asm volatile ("SWI 0x00000A")
#define SWI_OS_BPut                    asm volatile ("SWI 0x00000B")
#define SWI_OS_GBPB                    asm volatile ("SWI 0x00000C")
#define SWI_OS_Find                    asm volatile ("SWI 0x00000D")
#define SWI_OS_ReadLine                asm volatile ("SWI 0x00000E")
#define SWI_OS_Control                 asm volatile ("SWI 0x00000F")
#define SWI_OS_GetEnv                  asm volatile ("SWI 0x000010")
#define SWI_OS_Exit                    asm volatile ("SWI 0x000011")
#define SWI_OS_SetEnv                  asm volatile ("SWI 0x000012")
#define SWI_OS_IntOn                   asm volatile ("SWI 0x000013")
#define SWI_OS_IntOff                  asm volatile ("SWI 0x000014")
#define SWI_OS_CallBack                asm volatile ("SWI 0x000015")
#define SWI_OS_EnterOS                 asm volatile ("SWI 0x000016")
#define SWI_OS_BreakPt                 asm volatile ("SWI 0x000017")
#define SWI_OS_BreakCtrl               asm volatile ("SWI 0x000018")
#define SWI_OS_UnusedSWI               asm volatile ("SWI 0x000019")
#define SWI_OS_UpdateMEMC              asm volatile ("SWI 0x00001A")
#define SWI_OS_SetCallBack             asm volatile ("SWI 0x00001B")
#define SWI_OS_Mouse                   asm volatile ("SWI 0x00001C")
#define SWI_OS_Heap                    asm volatile ("SWI 0x00001D")
#define SWI_OS_Module                  asm volatile ("SWI 0x00001E")
#define SWI_OS_Claim                   asm volatile ("SWI 0x00001F")
#define SWI_OS_Release                 asm volatile ("SWI 0x000020")
#define SWI_OS_ReadUnsigned            asm volatile ("SWI 0x000021")
#define SWI_OS_GenerateEvent           asm volatile ("SWI 0x000022")
#define SWI_OS_ReadVarVal              asm volatile ("SWI 0x000023")
#define SWI_OS_SetVarVal               asm volatile ("SWI 0x000024")
#define SWI_OS_GSInit                  asm volatile ("SWI 0x000025")
#define SWI_OS_GSRead                  asm volatile ("SWI 0x000026")
#define SWI_OS_GSTrans                 asm volatile ("SWI 0x000027")
#define SWI_OS_BinaryToDecimal         asm volatile ("SWI 0x000028")
#define SWI_OS_FSControl               asm volatile ("SWI 0x000029")
#define SWI_OS_ChangeDynamicArea       asm volatile ("SWI 0x00002A")
#define SWI_OS_GenerateError           asm volatile ("SWI 0x00002B")
#define SWI_OS_ReadEscapeState         asm volatile ("SWI 0x00002C")
#define SWI_OS_EvaluateExpression      asm volatile ("SWI 0x00002D")
#define SWI_OS_SpriteOp                asm volatile ("SWI 0x00002E")
#define SWI_OS_ReadPalette             asm volatile ("SWI 0x00002F")
#define SWI_OS_ServiceCall             asm volatile ("SWI 0x000030")
#define SWI_OS_ReadVduVariables        asm volatile ("SWI 0x000031")
#define SWI_OS_ReadPoint               asm volatile ("SWI 0x000032")
#define SWI_OS_UpCall                  asm volatile ("SWI 0x000033")
#define SWI_OS_CallAVector             asm volatile ("SWI 0x000034")
#define SWI_OS_ReadModeVariable        asm volatile ("SWI 0x000035")
#define SWI_OS_RemoveCursors           asm volatile ("SWI 0x000036")
#define SWI_OS_RestoreCursors          asm volatile ("SWI 0x000037")
#define SWI_OS_SWINumberToString       asm volatile ("SWI 0x000038")
#define SWI_OS_SWINumberFromString     asm volatile ("SWI 0x000039")
#define SWI_OS_ValidateAddress         asm volatile ("SWI 0x00003A")
#define SWI_OS_CallAfter               asm volatile ("SWI 0x00003B")
#define SWI_OS_CallEvery               asm volatile ("SWI 0x00003C")
#define SWI_OS_InstallKeyHandler       asm volatile ("SWI 0x00003E")
#define SWI_OS_CheckModeValid          asm volatile ("SWI 0x00003F")
#define SWI_OS_ChangeEnvironment       asm volatile ("SWI 0x000040")
#define SWI_OS_ClaimScreenMemory       asm volatile ("SWI 0x000041")
#define SWI_OS_ReadMonotonicTime       asm volatile ("SWI 0x000042")
#define SWI_OS_SubstituteArgs          asm volatile ("SWI 0x000043")
#define SWI_OS_PrettyPrintCode         asm volatile ("SWI 0x000044")
#define SWI_OS_Plot                    asm volatile ("SWI 0x000045")
#define SWI_OS_WriteN                  asm volatile ("SWI 0x000046")
#define SWI_OS_AddToVector             asm volatile ("SWI 0x000047")
#define SWI_OS_WriteEnv                asm volatile ("SWI 0x000048")
#define SWI_OS_ReadArgs                asm volatile ("SWI 0x000049")
#define SWI_OS_ReadRAMFsLimits         asm volatile ("SWI 0x00004A")
#define SWI_OS_ClaimDeviceVector       asm volatile ("SWI 0x00004B")
#define SWI_OS_ReleaseDeviceVector     asm volatile ("SWI 0x00004C")
#define SWI_OS_DelinkApplication       asm volatile ("SWI 0x00004D")
#define SWI_OS_RelinkApplication       asm volatile ("SWI 0x00004E")
#define SWI_OS_HeapSort                asm volatile ("SWI 0x00004F")
#define SWI_OS_ExitAndDie              asm volatile ("SWI 0x000050")
#define SWI_OS_ReadMemMapInfo          asm volatile ("SWI 0x000051")
#define SWI_OS_ReadMemMapEntries       asm volatile ("SWI 0x000052")
#define SWI_OS_SetMemMapEntries        asm volatile ("SWI 0x000053")
#define SWI_OS_AddCallBack             asm volatile ("SWI 0x000054")
#define SWI_OS_ReadDefaultHandler      asm volatile ("SWI 0x000055")
#define SWI_OS_SetECFOrigin            asm volatile ("SWI 0x000056")
#define SWI_OS_SerialOp                asm volatile ("SWI 0x000057")
#define SWI_OS_ReadSysInfo             asm volatile ("SWI 0x000058")
#define SWI_OS_Confirm                 asm volatile ("SWI 0x000059")
#define SWI_OS_ChangedBox              asm volatile ("SWI 0x00005A")
#define SWI_OS_CRC                     asm volatile ("SWI 0x00005B")
#define SWI_OS_ReadDynamicArea         asm volatile ("SWI 0x00005C")
#define SWI_OS_PrintChar               asm volatile ("SWI 0x00005D")
#define SWI_OS_ChangeRedirection       asm volatile ("SWI 0x00005E")
#define SWI_OS_RemoveCallBack          asm volatile ("SWI 0x00005F")
#define SWI_OS_FindMemMapEntries       asm volatile ("SWI 0x000060")
#define SWI_OS_SetColourCode           asm volatile ("SWI 0x000061")
#define SWI_OS_ClaimSWI                asm volatile ("SWI 0x000062")
#define SWI_OS_ReleaseSWI              asm volatile ("SWI 0x000063")
#define SWI_OS_Pointer                 asm volatile ("SWI 0x000064")
#define SWI_OS_ScreenMode              asm volatile ("SWI 0x000065")
#define SWI_OS_DynamicArea             asm volatile ("SWI 0x000066")
#define SWI_OS_AbortTrap               asm volatile ("SWI 0x000067")
#define SWI_OS_Memory                  asm volatile ("SWI 0x000068")
#define SWI_OS_ClaimProcessorVector    asm volatile ("SWI 0x000069")
#define SWI_OS_Reset                   asm volatile ("SWI 0x00006A")
#define SWI_OS_MMUControl              asm volatile ("SWI 0x00006B")
#define SWI_OS_ResyncTime              asm volatile ("SWI 0x00006C")
#define SWI_OS_PlatformFeatures        asm volatile ("SWI 0x00006D")
#define SWI_OS_SynchroniseCodeAreas    asm volatile ("SWI 0x00006E")
#define SWI_OS_CallASWI                asm volatile ("SWI 0x00006F")
#define SWI_OS_AMBControl              asm volatile ("SWI 0x000070")
#define SWI_OS_CallASWIR12             asm volatile ("SWI 0x000071")
#define SWI_OS_SpecialControl          asm volatile ("SWI 0x000072")
#define SWI_OS_EnterUSR32              asm volatile ("SWI 0x000073")
#define SWI_OS_EnterUSR26              asm volatile ("SWI 0x000074")
#define SWI_OS_VIDCDivider             asm volatile ("SWI 0x000075")
#define SWI_OS_NVMemory                asm volatile ("SWI 0x000076")
#define SWI_OS_ClaimOSSWI              asm volatile ("SWI 0x000077")
#define SWI_OS_TaskControl             asm volatile ("SWI 0x000078")
#define SWI_OS_DeviceDriver            asm volatile ("SWI 0x000079")
#define SWI_OS_Hardware                asm volatile ("SWI 0x00007A")
#define SWI_OS_IICOp                   asm volatile ("SWI 0x00007B")
#define SWI_OS_LeaveOS                 asm volatile ("SWI 0x00007C")
#define SWI_OS_ReadLine32              asm volatile ("SWI 0x00007D")
#define SWI_OS_SubstituteArgs32        asm volatile ("SWI 0x00007E")
#define SWI_OS_HeapSort32              asm volatile ("SWI 0x00007F")

#endif
