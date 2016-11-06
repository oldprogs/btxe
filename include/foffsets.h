/*---------------------------------------------------------------------------*
 *
 *                               BinkleyTerm
 *
 *              (C) Copyright 1987-96, Bit Bucket Software Co.
 *     For lizense and contact information see /doc/orig_260/license.260.
 *
 *           This version was modified by the BinkleyTerm XE Team.
 *        For contact information see /doc/team.lst and /doc/join.us.
 *  For a complete list of changes see /doc/xe_user.doc and /doc/xe_hist.doc.
 *
 * Filename    : $Source: E:/cvs/btxe/include/foffsets.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/08/23 00:27:07 $
 * State       : $State: Exp $
 * Orig. Author: CEH
 *
 * Description : offset defines for files with packed 1 structures
 *
 *---------------------------------------------------------------------------*/

#define get_long(x)  ( (((long)(x)[3])<<24) + (((long)(x)[2])<<16) + (((long)(x)[1])<<8) + (((long)(x)[0])) )
#define get_short(x) ( (((short)(x)[1])<<8) + (((short)(x)[0])) )
#define get_word(x)  ( (((word )(x)[1])<<8) + (((word )(x)[0])) )

#define put_long(b,x) *(b++)=0xff&x;*(b++)=0xff&(x>>8);*(b++)=0xff&(x>>16);*(b++)=0xff&(x>>24);
#define put_short(b,x) *(b++)=0xff&x;*(b++)=0xff&(x>>8);
#define put_word(b,x) *(b++)=0xff&x;*(b++)=0xff&(x>>8);

/* Version 7 Nodelist */

#define OFS_v7_Zone_S          0
#define OFS_v7_Net_S           2
#define OFS_v7_Node_S          4
#define OFS_v7_HubNode_S       6
#define OFS_v7_CallCost_W      8
#define OFS_v7_MsgFee_W       10
#define OFS_v7_NodeFlags_W    12
#define OFS_v7_ModemType_B    14
#define OFS_v7_Phone_len_B    15
#define OFS_v7_Password_len_B 16
#define OFS_v7_Bname_len_B    17
#define OFS_v7_Sname_len_B    18
#define OFS_v7_Cname_len_B    19
#define OFS_v7_pack_len_B     20
#define OFS_v7_BaudRate_B     21

#define OFS_DTP_CTRL        0
#define OFS_DTP_TOPL        5
#define OFS_DTP_size_W      0
#define OFS_DTP_versi_B     2
#define OFS_DTP_allfs_B     3
#define OFS_DTP_addfs_B     4
#define OFS_DTP_ndowns_W    0
#define OFS_DTP_FlOfs_L     2

#define OFS_Ctl_BlkSize_W   0
#define OFS_Ctl_Root_L      2
#define OFS_Ctl_HiBlk_L     6
#define OFS_Ctl_LoLeaf_L   10
#define OFS_Ctl_HiLeaf_L   14
#define OFS_Ctl_Free_L     18
#define OFS_Ctl_Lvls_W     22
#define OFS_Ctl_Parity_W   24

#define OFS_Node_First_L   0
#define OFS_Node_FLink_L   4
#define OFS_Node_BLink_L   8
#define OFS_Node_Cnt_S    12
#define OFS_Node_Str_W    14
#define OFS_Node_Ref_Strt 16

#define OFS_IOfs_W    0
#define OFS_ILen_W    2
#define OFS_IData_L   4
#define OFS_IPtr_L    8
#define OFS_ILength  12

#define OFS_LOfs_W    0
#define OFS_LLen_W    2
#define OFS_LVal_L    4
#define OFS_LLength   8

/* $Id: foffsets.h,v 1.3 1999/08/23 00:27:07 ceh Exp $ */
