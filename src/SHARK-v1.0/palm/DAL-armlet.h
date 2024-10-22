/*************************************************************************
 *
 * Copyright (c) 2002-2003 Mobile Wizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-armlet.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef _DAL_ARMLET_
#define _DAL_ARMLET_

/*************************************************************************
 *
 *     ARM -> m68k helper macros
 * 
 *************************************************************************/

#define __byte_swap16(n) \
        ( ((((uint16) n) << 8) & 0xFF00) | \
          ((((uint16) n) >> 8) & 0x00FF) )

#define __byte_swap32(n) \
        ( ((((uint32) n) << 24) & 0xFF000000) |	\
          ((((uint32) n) <<  8) & 0x00FF0000) |	\
          ((((uint32) n) >>  8) & 0x0000FF00) |	\
          ((((uint32) n) >> 24) & 0x000000FF) )
 
#define __read_unaligned32(addr)  \
        ( ((((uint8 *)(addr))[0]) << 24) | \
          ((((uint8 *)(addr))[1]) << 16) | \
          ((((uint8 *)(addr))[2]) <<  8) | \
          ((((uint8 *)(addr))[3])) )

#define __write_unaligned32(addr, value) \
	      ( ((uint8 *)(addr))[0] = (uint8)((uint32)(value) >> 24), \
	      	((uint8 *)(addr))[1] = (uint8)((uint32)(value) >> 16), \
	  	    ((uint8 *)(addr))[2] = (uint8)((uint32)(value) >>  8), \
	  	    ((uint8 *)(addr))[3] = (uint8)((uint32)(value)) )

#define __read_byte32(addr)  \
        ( ((((uint8 *)(addr))[3]) << 24) | \
          ((((uint8 *)(addr))[2]) << 16) | \
          ((((uint8 *)(addr))[1]) <<  8) | \
          ((((uint8 *)(addr))[0])) )

#define __read_byte32_16(addr)  \
        ( ((((uint16 *)(addr))[1]) << 16) | \
          ((((uint16 *)(addr))[0])) )

#define __write_byte32(addr, value) \
        ( ((uint8 *)(addr))[3] = (uint8)((uint32)(value) >> 24), \
	        ((uint8 *)(addr))[2] = (uint8)((uint32)(value) >> 16), \
	        ((uint8 *)(addr))[1] = (uint8)((uint32)(value) >>  8), \
	        ((uint8 *)(addr))[0] = (uint8)((uint32)(value)) )

#define __write_byte32_16(addr, value) \
        ( ((uint16 *)(addr))[1] = (uint16)((uint32)(value) >> 16), \
          ((uint16 *)(addr))[0] = (uint16)((uint32)(value)) )

#define __read_unaligned16(addr)  \
        ( ((((uint8 *)(addr))[0]) <<  8) | \
          ((((uint8 *)(addr))[1])) )

#define __read_byte16(addr)  \
        ( ((((uint8 *)(addr))[1]) <<  8) | \
          ((((uint8 *)(addr))[0])) )

#define __write_unaligned16(addr, value) \
        ( ((uint8 *)(addr))[0] = (uint8)((uint16)(value) >>  8), \
          ((uint8 *)(addr))[1] = (uint8)((uint16)(value)) )

#define __write_byte16(addr, value) \
        ( ((uint8 *)(addr))[1] = (uint8)((uint16)(value) >>  8), \
          ((uint8 *)(addr))[0] = (uint8)((uint16)(value)) )

/*************************************************************************
 *
 *     Communication Interface
 * 
 *************************************************************************/

#define MAX_PARAM 4

typedef struct
{
  DAL_GlobalsType *gDALGlobals;
  uint32          *gDALGOT;

  uint32           command;
  void            *param[MAX_PARAM]; 
  uint32           result;
} ParameterType;

#endif

/********************************* E O F *********************************/
