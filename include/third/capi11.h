
/*------------------------------------------------------------------*/
/* File: api_defs.h                                                 */
/* Copyright (c) Diehl Elektronik GmbH 1989-1991                    */
/*                                                                  */
/* ISDN Common API definitions                                      */
/*------------------------------------------------------------------*/

#pragma pack(1)


#define TRUE 1
#define FALSE 0

#ifndef DWORD_DEFINED
#define DWORD_DEFINED
typedef unsigned long   dword ;
#endif

        /* operations on message queues                             */

/*
*/

        /* GD privat operations                                     */

#define API_CHECK       0x1100

#define API_SET_MAP     0xf800
#define API_GET_MAP     0xf900

        /* GD privat operations (available only for DEBUG versions) */

#define API_LOG_TOP     0xe000
#define API_LOG_LEVEL   0xe100
#define API_LOG_GET     0xe200

        /* ISDN Common API message types                            */

#define CONNECT_R               0x0002
#define CONNECT_I               0x0202
#define CONNECT_INFO_R          0x0009
#define CONNECT_ACTIVE_I        0x0203
#define DISCONNECT_R            0x0004
#define DISCONNECT_I            0x0204
#define LISTEN_R                0x0005
#define GET_PARMS_R             0x0006
#define INFO_R                  0x0007
#define INFO_I                  0x0207
#define DATA_R                  0x0008
#define DATA_I                  0x0208

#define SELECT_B2_PROTOCOL_R    0x0040

#define SELECT_B3_PROTOCOL_R    0x0080
#define LISTEN_B3_R             0x0081
#define CONNECT_B3_R            0x0082
#define CONNECT_B3_I            0x0282
#define CONNECT_B3_ACTIVE_I     0x0283
#define DISCONNECT_B3_R         0x0084
#define DISCONNECT_B3_I         0x0284
#define GET_B3_PARMS_R          0x0085
#define DATA_B3_R               0x0086
#define DATA_B3_I               0x0286
#define RESET_B3_R              0x0001
#define RESET_B3_I              0x0201

#define HANDSET_I               0x0287

        /* OR this to convert a REQUEST to a CONFIRM                */

#define CONFIRM                 0x0100

        /* OR this to convert a INDICATION to a RESPONSE            */

#define RESPONSE                0x0100

/*------------------------------------------------------------------*/
/* parameter structures                                             */
/*------------------------------------------------------------------*/

        /* CONNECT-REQUEST                                          */

typedef struct {
  byte          Controller;
  byte          B_Channel;
  dword         Info_Mask;
  byte          Sin[2];
  byte          Eaz;
  byte          Structs[1];
} CON_REQP;

        /* CONNECT-CONFIRM                                          */

typedef struct {
  word          PLCI;
  word          Info;
} CON_CONP;

        /* CONNECT-INDICATION                                       */

typedef struct {
  word          PLCI;
  byte          Controller;
  byte          Sin[2];
  byte          Eaz;
  byte          Structs[1];
} CON_INDP;

        /* CONNECT-RESPONSE                                         */

typedef struct {
  word          PLCI;
  byte          Reject;
} CON_RESP;

        /* CONNECT-INFO-REQ                                         */

typedef struct {
  word          PLCI;
  byte          Structs[1];
} CON_I_REQP;

        /* CONNECT-INFO-CON                                         */

typedef struct {
  word          PLCI;
  word          Info;
} CON_I_CONP;

        /* CONNECT-ACTIVE-INDICATION                                */

typedef struct {
  word          PLCI;
  byte          Structs[1];
} CON_A_INDP;

typedef struct {
  word          PLCI;
} CON_A_RESP;

        /* DISCONNECT-REQUEST                                       */

typedef struct {
  word          PLCI;
  byte          Cause;
} DIS_REQP;

        /* DISCONNECT-CONFIRM                                       */

typedef struct {
  word          PLCI;
  word          Info;
} DIS_CONP;

        /* DISCONNECT-INDICATION                                    */

typedef struct {
  word          PLCI;
  word          Info;
} DIS_INDP;

        /* DISCONNECT-RESPONSE                                      */

typedef struct {
  word          PLCI;
} DIS_RESP;


        /* LISTEN-REQUEST                                           */

typedef struct {
  byte          Controller;
  dword         Info_Mask;
  word          Eaz_Mask;
  word          Si_Mask;
} LIS_REQP;

        /* LISTEN-CONFIRM                                           */

typedef struct {
  byte          Controller;
  word          Info;
} LIS_CONP;

        /* GET-PARMS-REQUEST                                        */

typedef struct {
  word          PLCI;
} GET_P_REQP;

        /* GET-PARMS-CONFIRM                                        */

typedef struct {
  word          PLCI;
  byte          Controller;
  byte          B_Channel;
  byte          B3_Link_Count;
  byte          Sin[2];
  byte          Structs[1];
} GET_P_CONP;

        /* INFO-REQUEST                                             */

typedef struct {
  word          PLCI;
  dword         Info_Mask;
} INF_REQP;

        /* INFO-CONFIRM                                             */

typedef struct {
  word          PLCI;
  word          Info;
} INF_CONP;

        /* INFO-INDICATION                                          */

typedef struct {
  word          PLCI;
  word          Number;
  byte          Structs[1];
} INF_INDP;

        /* INFO-RESPONSE                                            */

typedef struct {
  word          PLCI;
} INF_RESP;

        /* DATA-REQUEST                                             */

typedef struct {
  word          PLCI;
  byte          Structs[1];
} DAT_REQP;

        /* DATA-CONFIRM                                             */

typedef struct {
  word          PLCI;
  word          Info;
} DAT_CONP;

        /* DATA-INDICATION                                          */

typedef struct {
  word          PLCI;
  byte          Structs[1];
} DAT_INDP;

        /* DATA-RESPONSE                                            */

typedef struct {
  word          PLCI;
} DAT_RESP;

        /* SELECT-B2-PROTOCOL-REQUEST                               */

typedef struct {
  word          PLCI;
  byte          B2_Protocol;
  byte          Structs[1];
} SEL_B2_REQP;

        /* SELECT-B2-PROTOCOL-CONFIRM                               */

typedef struct {
  word          PLCI;
  word          Info;
} SEL_B2_CONP;

        /* SELECT-B3-PROTOCOL-REQUEST                               */

typedef struct {
  word          PLCI;
  byte          B3_Protocol;
  byte          Structs[1];
} SEL_B3_REQP;

        /* SELECT-B3-PROTOCOL-CONFIRM                               */

typedef struct {
  word          PLCI;
  word          Info;
} SEL_B3_CONP;

        /* LISTEN-B3-REQUEST                                        */

typedef struct {
  word          PLCI;
} LIS_B3_REQP;

        /* LISTEN-B3-CONFIRM                                        */

typedef struct {
  word          PLCI;
  word          Info;
} LIS_B3_CONP;

        /* CONNECT-B3-REQUEST                                       */

typedef struct {
  word          PLCI;
  byte          Structs[1];
} CON_B3_REQP;

        /* CONNECT-B3-CONFIRM                                       */

typedef struct {
  word          PLCI;
  word          NCCI;
  word          Info;
} CON_B3_CONP;

        /* CONNECT-B3-INDICATION                                    */

typedef struct {
  word          NCCI;
  word          PLCI;
  byte          Structs[1];
} CON_B3_INDP;

        /* CONNECT-B3-RESPONSE                                      */

typedef struct {
  word          NCCI;
  byte          Reject;
  byte          Structs[1];
} CON_B3_RESP;

        /* CONNECT-B3-ACTIVE-INDICATION                             */

typedef struct {
  word          NCCI;
  byte          Structs[1];
} CON_B3_A_INDP;

        /* CONNECT-B3-ACTIVE-RESPONSE                               */

typedef struct {
  word          NCCI;
} CON_B3_A_RESP;


        /* DISCONNECT-B3-REQUEST                                    */

typedef struct {
  word          NCCI;
  byte          Structs[1];
} DIS_B3_REQP;

        /* DISCONNECT-B3-CONFIRM                                    */

typedef struct {
  word          NCCI;
  word          Info;
} DIS_B3_CONP;

        /* DISCONNECT-B3-INDICATION                                 */

typedef struct {
  word          NCCI;
  word          Info;
  byte          Structs[1];
} DIS_B3_INDP;

        /* DISCONNECT-B3-RESPONSE                                   */

typedef struct {
  word          NCCI;
} DIS_B3_RESP;

        /* GET-B3-PARMS-REQUEST                                     */

typedef struct {
  word          NCCI;
} GET_B3_P_REQP;

        /* GET-B3-PARMS-CONFIRM                                     */

typedef struct {
  word          NCCI;
  word          PLCI;
  word          Info;
} GET_B3_P_CONP;

        /* DATA-B3-REQUEST                                          */

typedef struct {
  word          NCCI;
  word          Data_Length;
  byte * _Seg16 Data;
  byte          Number;
  word          Flags;
} DAT_B3_REQP;

        /* DATA-B3-CONFIRM                                          */

typedef struct {
  word          NCCI;
  byte          Number;
  word          Info;
} DAT_B3_CONP;

        /* DATA-B3-INDICATION                                       */

typedef struct {
  word          NCCI;
  word          Data_Length;
  byte * _Seg16 Data;
  byte          Number;
  word          Flags;
} DAT_B3_INDP;

        /* DATA-B3-RESPONSE                                         */

typedef struct {
  word          NCCI;
  byte          Number;
} DAT_B3_RESP;

        /* RESET-B3-REQUEST                                         */

typedef struct {
  word          NCCI;
} RES_B3_REQP;

        /* RESET-B3-CONFIRM                                         */

typedef struct {
  word          NCCI;
  word          Info;
} RES_B3_CONP;

        /* RESET-B3-INDICATION                                      */

typedef struct {
  word          NCCI;
} RES_B3_INDP;

        /* RESET-B3-RESPONSE                                        */

typedef struct {
  word          NCCI;
} RES_B3_RESP;

        /* HANDSET-INDICATION                                       */

typedef struct {
  word          PLCI;
  byte          Controller;
  byte          Status;
} HAN_INDP;

        /* HANDSET-RESPONSE                                         */

typedef struct {
  word          PLCI;
} HAN_RESP;

/*------------------------------------------------------------------*/
/* message structure                                                */
/*------------------------------------------------------------------*/

typedef struct API_MSGS API_MSG;
typedef struct MSG_HEADERS MSG_HEADER;

struct API_MSGS {
  struct MSG_HEADERS {
    word        Length;
    word        Appl_id;
    word        Command;
    word        Number;
  } header;
  union info_u {
    byte                b[128];
    word                w[64];
    CON_REQP            connect_req;
    CON_CONP            connect_con;
    CON_INDP            connect_ind;
    CON_RESP            connect_res;
    CON_I_REQP          connect_i_req;
    CON_I_CONP          connect_i_con;
    CON_A_INDP          connect_a_ind;
    CON_A_RESP          connect_a_res;
    DIS_REQP            disconnect_req;
    DIS_CONP            disconnect_con;
    DIS_INDP            disconnect_ind;
    DIS_RESP            disconnect_res;
    LIS_REQP            listen_req;
    LIS_CONP            listen_con;
    GET_P_REQP          get_parms_req;
    GET_P_CONP          get_parms_con;
    INF_REQP            info_req;
    INF_CONP            info_con;
    INF_INDP            info_ind;
    INF_RESP            info_res;
    DAT_REQP            data_req;
    DAT_CONP            data_con;
    DAT_INDP            data_ind;
    DAT_RESP            data_res;
    SEL_B2_REQP         sel_b2_req;
    SEL_B2_CONP         sel_b2_con;
    SEL_B3_REQP         sel_b3_req;
    SEL_B3_CONP         sel_b3_con;
    LIS_B3_REQP         listen_b3_req;
    LIS_B3_CONP         listen_b3_con;
    CON_B3_REQP         connect_b3_req;
    CON_B3_CONP         connect_b3_con;
    CON_B3_INDP         connect_b3_ind;
    CON_B3_RESP         connect_b3_res;
    CON_B3_A_INDP       connect_b3_a_ind;
    CON_B3_A_RESP       connect_b3_a_res;
    DIS_B3_REQP         disconnect_b3_req;
    DIS_B3_CONP         disconnect_b3_con;
    DIS_B3_INDP         disconnect_b3_ind;
    DIS_B3_RESP         disconnect_b3_res;
    GET_B3_P_REQP       get_b3_p_req;
    GET_B3_P_CONP       get_b3_p_con;
    DAT_B3_REQP         data_b3_req;
    DAT_B3_CONP         data_b3_con;
    DAT_B3_INDP         data_b3_ind;
    DAT_B3_RESP         data_b3_res;
    RES_B3_REQP         reset_b3_req;
    RES_B3_CONP         reset_b3_con;
    RES_B3_INDP         reset_b3_ind;
    RES_B3_RESP         reset_b3_res;
    HAN_INDP            handset_ind;
    HAN_RESP            handset_res;
  } info;
};

/*------------------------------------------------------------------*/
/* API structs definitions                                          */
/*------------------------------------------------------------------*/

typedef struct {
  byte Length;
  word Block_Size;
  byte Adr_A;
  byte Adr_B;
  byte Modulo;
  byte Window_Size;
  byte Xid[6];
} DLPD;

typedef union {
  byte Length;
  struct {
    word LIC;
    word HIC;
    word LTC;
    word HTC;
    word LOC;
    word HOC;
    byte Modulo;
  } x25;
  struct {
    byte Resolution;
    byte Max_speed;
    byte Format;
    byte Xmit_level;
    byte Structs[64];
  } fax3;
} NCPD;

/*------------------------------------------------------------------*/
/* API parameter definitions                                        */
/*------------------------------------------------------------------*/

#define X75                     0x01
#define FRAME_TRANSPARENT       0x02
#define BIT_TRANSPARENT         0x03
#define SDLC                    0x04
#define X75_BTX                 0x05

#define T70NL                   0x01
#define ISO8208                 0x02
#define T90NL                   0x03
#define TRANSPARENT             0x04
#define T30                     0x05

/*------------------------------------------------------------------*/
/* API Interrupt return codes                                       */
/*------------------------------------------------------------------*/

#define GOOD                    0x0000
#define API_REGISTER_ERROR      0x1001
#define WRONG_APPL_ID           0x1002
#define BAD_MSG                 0x1003
#define WRONG_MSG_TYPE          0x1004
#define QUEUE_FULL              0x1005
#define GET_NO_MSG              0x1006
#define MSG_LOST                0x1007
#define API_REMOVE_ERROR        0x1008
#define WRONG_MSG_OPERATION     0x1009

/*------------------------------------------------------------------*/
/* Addressing error codes                                           */
/*------------------------------------------------------------------*/

#define WRONG_CONTROLLER        0x2001
#define WRONG_PLCI              0x2002
#define WRONG_NCCI              0x2003
#define WRONG_IDENTIFIER        0x2004

/*------------------------------------------------------------------*/
/* Configuration error codes                                        */
/*------------------------------------------------------------------*/

#define WRONG_B_CHANNEL         0x3101
#define WRONG_INFO_MASK         0x3102
#define WRONG_DLPD              0x3106
#define WRONG_NCPD              0x3108

#define APPL_CONFLICT           0x3202
#define NCCI_NOT_ACTIVE         0x3205
#define B2_NOT_SUPORTED         0x3206
#define B2_SELECT_WRONG_STATE   0x3207
#define B3_NOT_SUPORTED         0x3208
#define B3_SELECT_WRONG_STATE   0x3209
#define WRONG_DATA_LENGTH       0x320d

#define L1_ERROR                0x3301
#define L2_ERROR                0x3302

#define L3_ERROR                0x3400

static  USHORT ( *_Far16 _Pascal API_GET_MANUFACTURER)
(
  char *pszBuffer
) = NULL ;
static  USHORT ( *_Far16 _Pascal API_GET_VERSION)
(
  char *pszBuffer
) = NULL ;
static USHORT ( *_Far16 _Pascal API_INSTALLED)
(
  void
) = NULL ;
static USHORT ( *_Far16 _Pascal API_GET_SERIAL_NUMBER)
(
  char *pszBuffer
) = NULL ;
static USHORT ( *_Far16 _Pascal API_REGISTER)
(
  char *pszBuffer, USHORT ausCntMsg, USHORT ausCntLevel3,
  USHORT ausCntB3Data, USHORT ausMaxLenB3Data
) = NULL ;
static USHORT ( *_Far16 _Pascal API_RELEASE)
(
  USHORT usApplication
) = NULL ;
static USHORT ( *_Far16 _Pascal API_GET_MESSAGE)
(
  USHORT usApplication, API_MSG * _Seg16 * _Seg16 pMsg
) = NULL ;
static USHORT ( *_Far16 _Pascal API_PUT_MESSAGE)
(
  USHORT usApplication, API_MSG * pMsg
) = NULL ;
static USHORT ( *_Far16 _Pascal API_SET_SIGNAL)
(
  USHORT usApplication, void (* _Far16 _Pascal Signal)( void )
) = NULL ;

#pragma pack()

