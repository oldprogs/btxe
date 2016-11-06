/*---------------------------------------------------------------------------*
 *
 *                               BinkleyTerm
 *
 *              (C) Copyright 1987-96, Bit Bucket Software Co.
 *     For license and contact information see /doc/orig_260/license.260.
 *
 *           This version was modified by the BinkleyTerm XE Team.
 *        For contact information see /doc/team.lst and /doc/join.us.
 *  For a complete list of changes see /doc/xe_user.doc and /doc/xe_hist.doc.
 *
 * Filename    : $Source: E:/cvs/btxe/include/msgs.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/28 12:46:02 $
 * State       : $State: Exp $
 *
 * Description : Message definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Make this easier to change in other environments (e.g. PM, Windows) */

#define MSG_TXT(m)               msgtxt[m]

#define M_DIALING_SCRIPT         0
#define M_NOOPEN_SCRIPT          1
#define M_MASTER_SCRIPT_TIMER    2
#define M_SCRIPT_FAILED          3
#define M_MEM_AVAIL              4
#define M_TOO_MANY_LABELS        5
#define M_MACRO_NUMBER           6
#define M_OUT_OF_RANGE           7
#define M_SHELL_NUMBER           8
#define M_ENOUGH                 9
#define M_REQUESTS_ALLOWED       10
#define M_BAD_LOGLEVEL           11
#define M_BAD_MAXPORT            12
#define M_ILLEGAL_PORT           13
#define M_ILLEGAL_CARRIER        14
#define M_KEY                    15
#define M_CALL                   16
#define M_UNKNOWN_LINE           17
#define M_INVALID_PRIO_ENVVAR    18
#define M_NOMEM_FOR_OUTB_CACHE   19
#define M_PRESS_ESCAPE           20
#define M_NO_BBS                 21
#define M_NOTHING_TO_SEND        22
#define M_CONNECT_ABORTED        23
#define M_MODEM_HANGUP           24
#define M_NO_OUT_REQUESTS        25
#define M_OUT_REQUESTS           26
#define M_END_OUT_REQUESTS       27
#define M_FREQ_DECLINED          28
#define M_ADDRESS                29
#define M_INTRO                  30
#define M_NOBODY_HOME            31
#define M_NO_CARRIER             32
#define M_PROTECTED_SESSION      33
#define M_PWD_ERROR              34
#define M_CALLED                 35
#define M_WAZOO_METHOD           36
#define M_WAZOO_END              37
#define M_PACKET_MSG             38
#define M_OPEN_MSG               39
#define M_KBD_MSG                40
#define M_TRUNC_MSG              41
#define M_RENAME_MSG             42
#define M_DEVICE_MSG             43
#define M_FUBAR_MSG              44
#define M_UNLINKING_MSG          45
#define M_CAN_MSG                46
#define M_NO_CTL_FILE            47
#define M_FOSSIL_GONE            48
#define M_NO_LOGFILE             49
#define M_FOSSIL_TYPE            50
#define M_REACTIVATING_REQ       51
#define M_STARTING_EVENT         52
#define M_EVENT_EXIT             53
#define M_CLEAN_PACK             54
#define M_AFTER_CLEAN_PACK       55
#define M_DATE_PROBLEM           56
#define M_BBS_EXIT               57
#define M_BBS_SPAWN              58
#define M_BBS_RETURN             59
#define M_EXT_MAIL               60
#define M_SETTING_BAUD           61
#define M_REMOTE_USES            62
#define M_VERSION                63
#define M_TAAG_ERRORS            64  /* CEH/311195: Today at a glace - Errors */
#define M_SEND_FALLBACK          65
#define M_REFUSING_IN_FREQ       66
#define M_TOO_LONG               67
#define M_0001_END               68
#define M_RECV_FALLBACK          69
#define M_GIVING_MAIL            70
#define M_REFUSE_PICKUP          71
#define M_MEM_ERROR              72
#define M_OUTBOUND               73
#define M_FILE_ATTACHES          74
#define M_MAKING_FREQ            75
#define M_END_OF                 76
#define M_RECV_MAIL              77
#define M_NO_PICKUP              78
#define M_INBOUND                79
#define M_MAIL_PACKET            80
#define M_PWD_ERR_ASSUMED        81
#define M_CANT_RENAME_MAIL       82
#define M_MAIL_PACKET_RENAMED    83
#define M_NO_PROTOCOL            84
#define M_DUP_PROTOCOL           85
#define M_PROCESSING_NODE        86
#define M_NO_ADDRESS             87
#define M_NUISANCE_CALLER        88
#define M_NO_BOSS                89
#define M_UNABLE_TO_OPEN         90
#define M_NODELIST_MEM           91
#define M_NODELIST_READ_ERR      92
#define M_NODELIST_SEEK_ERR      93
#define M_NEW_DAY                94  /* MR 971202 */
#define M_FILE_REQUESTS          95
#define M_MATCHING_FILES         96
#define M_BAD_BARK               97
#define M_BAD_CRC                98
#define M_MISCONFIGURED          99
#define M_V7P_INVALID_TXY        100
#define M_INCOMING_CALL          101
#define M_DYNAMIC_EVENT          102
#define M_EXIT_REQUEST           103
#define M_FUNCTION_KEY           104
#define M_DISABLE_MODEM          105
#define M_BEGIN_MESSAGE_READER   106
#define M_END_MESSAGE_READER     107
#define M_ENABLE_MODEM           108
#define M_NO_MESSAGE_READER      109
#define M_NO_GET                 110
#define M_NO_KILL                111
#define M_POLL_MODE              112
#define M_POLL_COMPLETED         113
#define M_NO_SEND                114
#define M_ENTER_TERMINAL_MODE    115
#define M_LAUNCH_KEYBOARD_SHELL  116
#define M_KEYBOARD_SHELL         117
#define M_END_KEYBOARD_SHELL     118
#define M_NO_KEYBOARD_SHELL      119
#define M_JUNK_CHARACTER         120
#define M_SHELLING               121
#define M_TYPE_EXIT              122
#define M_BINKLEY_BACK           123
#define M_NO_CALLS_NOW           124
#define M_IMMEDIATE_CALL         125
#define M_ENTER_NET_NODE         126
#define M_CURRENTLY_POLLING      127
#define M_READY_INIT             128
#define M_READY_WAITING          129
#define M_READY_DIALING          130
#define M_READY_HANGUP           131
#define M_READY_CONNECT          132
#define M_DIALING_NUMBER         133
#define M_NO_END_MIDNIGHT        134
#define M_INVALID_START          135
#define M_INVALID_END            136
#define M_INVALID_AVGWAIT        137
#define M_BAD_ERRORLEVEL         138
#define M_BAD_TRIES              139
#define M_INDECIPHERABLE         140
#define M_BAD_DAY                141
#define M_NOTHING_IN_OUTBOUND    142
#define M_OUTBOUND_HEADER        143
#define M_INSUFFICIENT_DATA      144
#define M_END_OF_ATTEMPT         145
#define M_STILL_HAVE_MAIL        146
#define M_EXIT_COMPRESSED        147
#define M_EXIT_AFTER_EXTENT      148
#define M_EXIT_AFTER_MAIL        149
#define M_AFTERMAIL              150
#define M_OK_AFTERMAIL           151
#define M_SETTING                152
#define M_INITIAL_SPEED          153
#define M_INTRO_END              154
#define M_LAUNCH_AFTER_MAIL      155
#define M_CURRENT_SETTINGS       156
#define M_TODAY_GLANCE           157
#define M_PENDING_OUTBOUND       158
#define M_INITIALIZING_SYSTEM    159
#define M_RECENT_ACTIVITY        160
#define M_TRANSFER_STATUS        161
#define M_EVENT_COLON            162
#define M_PORT_COLON             163
#define M_STATUS_INIT            164
#define M_CALLS_IN               165
#define M_CALLS_OUT              166
#define M_RX_C_V                 167
#define M_TX_C_V                 168
#define M_LAST                   169
#define M_ALTF10                 170
#define M_NODE_COLON             171
#define M_PASSWORD_OVERRIDE      172
#define M_ZMODEM_STARTED         173
#define M_ZMODEM_FINISHED        174
#define M_BINK_NOW_AT            175
#define M_PHONE_OR_NODE          176
#define M_NO_DROP_DTR            177
#define M_WELCOME_BACK           178
#define M_INPUT_LOGFILE          179
#define M_READY_ANSWERING        180  /* Answering Status DR 971022 */
#define M_MANUAL_MODE            181
#define M_BINK_NOW_USING         182
#define M_READY_TO_SEND          183
#define M_WHICH_PROTOCOL         184
#define M_READY_TO_RECEIVE       185
#define M_SENDING_BREAK          186
#define M_DONE                   187
#define M_YOU_ARE_USING          188
#define M_DONT_KNOW_PROTO        189
#define M_FILE_TO_SEND           190
#define M_CHOOSE                 191
#define M_FILE_TO_RECEIVE        192
#define M_LOG_CLOSED             193
#define M_LOG_NOT_OPEN           194
#define M_LOG_OPEN               195
#define M_END_OF_CONNECT         196
#define M_SWITCHING_BACK         197
#define M_POLLING_COLON          198
#define M_GET_FILE               199
#define M_ADDRESS_TO_GET_FROM    200
#define M_PASSWORD_TO_USE        201
#define M_ARE_YOU_SURE           202
#define M_YES                    203
#define M_NO                     204
#define M_SEND_FILE              205
#define M_ADDRESS_TO_SEND_TO     206
#define M_KILL_MAIL              207
#define M_ADDRESS_TO_KILL        208
#define M_KILL_ALL_MAIL          209
#define M_DRATS                  210
#define M_HE_HUNG_UP             211
#define M_CORRECTED_ERRORS       212
#define M_FILE_SENT              213
#define M_SYNCHRONIZING          214
#define M_TEMP_NOT_OPEN          215
#define M_ALREADY_HAVE           216
#define M_SYNCHRONIZING_EOF      217
#define M_unused_218             218  /* unused */
#define M_FILE_RECEIVED          219
#define M_ORIGINAL_NAME_BAD      220
#define M_UNEXPECTED_EOF         221
#define M_REMOTE_SYSTEM          222
#define M_UNKNOWN_MAILER         223
#define M_SYSTEM_INITIALIZING    224
#define M_UNRECOGNIZED_OPTION    225
#define M_THANKS                 226
#define M_ANOTHER_FINE_PRODUCT   227
#define M_REMOTE_REFUSED         228
#define M_ERROR                  229
#define M_CANT                   230
#define M_CPS_MESSAGE            231
#define M_COMPRESSED_MAIL        232
#define M_NET_FILE               233
#define M_unused_234             234  /* unused, was M_ELAPSED */
#define M_TROUBLE                235
#define M_RESENDING_FROM         236
#define M_SEND_MSG               237
#define M_UPDATE                 238
#define M_FILE                   239
#define M_REQUEST                240
#define M_EXECUTING              241
#define M_CARRIER_REQUEST_ERR    242
#define M_FREQ_LIMIT             243
#define M_EVENT_OVERRUN          244
#define M_NO_AVAIL               245
#define M_NO_ABOUT               246
#define M_OKFILE_ERR             247
#define M_FREQ_PW_ERR            248
#define M_RECEIVE_MSG            249
#define M_TIMEOUT                250
#define M_CHECKSUM               251
#define M_CRC_MSG                252
#define M_JUNK_BLOCK             253
#define M_ON_BLOCK               254
#define M_FIND_MSG               255
#define M_READ_MSG               256
#define M_SEEK_MSG               257
#define M_RAS_CAPTION            258
#define M_RAS_KEYS               259
#define M_unused_260             260  /* reserved for recent activity search  */
#define M_WRITE_MSG              261
#define M_SKIP_MSG               262
#define M_PHONE_HELP             263
#define M_PHONE_HELP2            264
#define M_LAST_SET               265
#define M_CURRENT_PHONES         266
#define M_INPUT_COMMAND          267
#define M_ELEMENT_CHOSEN         268
#define M_CURRENTLY_CONTAINS     269
#define M_PHONE_HELP3            270
#define M_PHONE_HELP4            271
#define M_SET_READ_ERROR         272
#define M_SET_CLOSE_ERR          273
#define M_SET_LOADED             274
#define M_SET_WRITE_ERROR        275
#define M_SET_SAVED              276
#define M_PRESS_ENTER            277
#define M_STARTING_SCAN          278
#define M_CONNECTED_TO_ITEM      279
#define M_DEBRIS                 280
#define M_LONG_PACKET            281
#define M_Z_IGNORING             282
#define M_OUT_OF_DISK_SPACE      283
#define M_RECEIVING              284
#define M_Z_INITRECV             285
#define M_BAD_POS                286
#define M_Z_RZ                   287
#define M_J_BAD_PACKET           288
#define M_OFFSET_RETRIES         289
#define M_OFFSET_RETRIES_SK      290
#define M_OTHER_DIED             291
#define M_GOING_ONE_WAY          292
#define M_REFUSING               293
#define M_UNKNOWN_PACKET         294
#define M_SESSION_ABORT          295
#define M_SENDING                296
#define M_NO_LENGTH              297
#define M_FINISHED_PART          298
#define M_SAVING_PART            299
#define M_REMOTE_CANT_FREQ       300
#define M_MAIN_HELP              301  /* 11 messages total */
#define M_MACRO_HELP             312
#define M_PRESS_ANYKEY           313
#define M_MAILER_HELP            314  /* 14 messages total */
#define M_BLANK_HELP             328
#define M_EDITOR_HELP            329
#define M_SHELL_HELP             330
#define M_unused_331             331  /* unused */
#define M_unused_332             332  /* unused */
#define M_DRIVER_DEAD_1          333
#define M_DRIVER_DEAD_2          334
#define M_DRIVER_DEAD_3          335
#define M_FAILED_CREATE_FLAG     336
#define M_CREATED_FLAGFILE       337
#define M_THIS_ADDRESS_LOCKED    338
#define M_BAD_CLEAR_FLAGFILE     339
#define M_CLEARED_FLAGFILE       340
#define M_FAILED_CLEAR_FLAG      341
#define M_BYTE_LIMIT             342
#define M_REFRESH_NODELIST       343
#define M_NO_SIGINT              344
#define M_FILTER                 345
#define M_NEXT_EVENT             346
#define M_CUR_EVENT              347
#define M_UNREAD_NETMAIL         348
#define M_NODE_COST              349
#define M_TIME_LIMIT             350
#define M_BAD_QSIZE              351
#define M_BAD_EVENT_ALLOC        352
#define M_BINK_BEGIN             353
#define M_BINK_END               354
#define M_NODE_TO_POLL           355
#define M_POLL_WHOM              356
#define M_NO_COMMON_PROTO        357
#define M_EXT_SPAWN              358
#define M_EXT_RETURN             359
#define M_FORCED_EXIT            360
#define M_unused_361             361  /* unused, was M_CRASH_HOLD_DIR_NORM */
#define M_CHDN_FLAGS             362
#define M_unused_363             363  /* unused, was M_EMSI_METHOD */
#define M_unused_364             364  /* unused, was M_EMSI_END */
#define M_EMSI_PROC_NODE         365
#define M_MAXFILE_SCANNING       366
#define M_MAX_SECURITY_MISMATCH  367
#define M_EMXMIT                 368
#define M_EMRCPT                 369
#define M_EMWAIT                 370
#define M_EMRECV                 371
#define M_EMSEND                 372
#define M_EMSHORT                373
#define M_EMSTART                374
#define M_EXIT_FAX               375
#define M_ZOOM_WAIT              376
#define M_ZOOM_HEADER            377
#define M_ZOOM_TITLE             378
#define M_NODE_BUSY              379
#define M_ZOOM_CHANGE            380
#define M_ZOOM_STOP              381
#define M_ZOOM_DELETE            382
#define M_ZOOM_POLL              383
#define M_ZOOM_DELREQ            384
#define M_ZOOM_DESTINATION       385
#define M_ZOOM_APPEND            386
#define M_ZOOM_RENAME            387
#define M_ZOOM_ERRAPP            388
#define M_ZOOM_DELCOUNT          389
#define M_ZOOM_NOSEND            390
#define M_ZOOM_ERRNOSEND         391
#define M_unused_392             392  /* unused, was M_ZOOM_POLLADDR */
#define M_unused_393             393  /* unused, was M_ZOOM_ERRCLO */
#define M_unused_394             394  /* unused, was M_ZOOM_CLO */
#define M_ZOOM_ERRREQ            395
#define M_ZOOM_DEL_REQ           396
#define M_ZOOM_NEWADDR           397
#define M_ZOOM_MAIL_ATT          398
#define M_unused_399             399  /* unused, was M_ZOOM_REQ_FIL */
#define M_ZOOM_MAIL_PKT          400
#define M_GET_FLAVOR             401
#define M_SEND_FLAVOR            402
#define M_GET_MORE               403
#define M_SEND_MORE              404
#define M_ZOOM_HELP              405  /* 7 lines */
#define M_NO_COSTFILE            412
#define M_BYTE_SUMMARY           413
#define M_COST_SUMMARY           414
#define M_REC_REQ_AS             415
#define M_HYDRA_AUTO_STARTED     416
#define M_HYDRA_AUTO_FINISHED    417
#define M_START_EXTMAIL          418
#define M_END_EXTMAIL            419
#define M_SHELL_FAX              420
#define M_SHELL_COMPRESSED       421
#define M_SHELL_AFTER_EXTENT     422
#define M_SHELL_AFTER_MAIL       423
#define M_START_ERRLVL_SESSION   424
#define M_RETURN_ERRLVL_SHELL    425
#define M_SQUISH_OPENERR         426
#define M_SQUISH_SEEKERR         427
#define M_SQUISH_READERR         428
#define M_SQUISH_TRASHED_HDR     429
#define M_EVENT_SHELL            430
#define M_BAD_ERRLEVELSHELLNUM   431
#define M_DIALING_ALTNUM         432
#define M_ALTNUM_NO_PRIMARY      433
#define M_ALTNUM_NO_ALTERNATE    434
#define M_FORCED_SHELL           435
#define M_NOENVVARTASK           436
#define M_SYNCMYCLOCK            437
#define M_ACCEPT2NDCALL          438
#define M_REJECTLOWVOLUMECALL    439
#define M_ACCEPTHIGHVOLUMECALL   440
#define M_SRIF_INVOKE            441
#define M_ACCEPT                 442
#define M_REJECT                 443
#define M_ACCEPT1STCALLEVER      444
#define M_CALL_REJECT            445
#define M_WRITEPROTECTED         446
#define M_CHAT_WINLOCAL          447
#define M_CHAT_WINREMOTE         448
#define M_I_DONT_KNOW            449
#define M_MODEM_ACTIVITY         450  /* MR 970215 */
#define M_DELETE_BTTASK          451  /* unused */
#define M_CEReserved452          452  /* unused */
#define M_CEReserved453          453  /* unused */
#define M_STATUS_YOOHOO          454
#define M_STATUS_YOOHOO2U2       455
#define M_STATUS_EMSISEND        456
#define M_STATUS_EMSIWAIT        457
#define M_STATUS_EMSIRECV        458
#define M_STATUS_EMSIRCVD        459
#define M_PROTOCOL               460
#define M_STATUS_XHYDRA          461
#define M_STATUS_EXTERN          462
#define M_STATUS_FTS1            463
#define M_SESSION                464
#define M_STATUS_EMSI            465
#define M_STATUS_HYDRA           466
#define M_STATUS_JANUS           467
#define M_STATUS_ZEDZAP          468
#define M_STATUS_ZEDZIP          469
#define M_STATUS_DTERATE         470
#define M_STATUS_OFFLINE         471
#define M_STATUS_SESSION         472
#define M_STATUS_DCERATE         473
#define M_STATUS_XMODEM          474
#define M_MTTReserved475         475  /* unused */
#define M_MTTReserved476         476  /* unused */
#define M_MTTReserved477         477  /* unused */
#define M_MTTReserved478         478  /* unused */
#define M_MTTReserved479         479  /* unused */
#define M_GETFILE_ANDPWD         480
#define M_SELECTANODE            481
#define M_LOOKUPTITLE            482
#define M_NODE_NOT_FOUND         483
#define M_SHOULD_I_USE           484
#define M_RHReserver485          485  /* unused */
#define M_RHReserved486          486  /* unused */
#define M_RHReserved487          487  /* unused */
#define M_READY_DIALCOUNT        488  /* MR 971223 */
#define M_READY_ANSWCOUNT        489  /* MR 971223 */
#define M_READY_RESCAN           490
#define M_OUTBOUND_RESCAN        491
#define M_REJECTING_HUMAN_CALLER 492
#define M_INVALID_COSTEVENT      493
#define M_READY_READING          494
#define M_OUTBOUND_READING       495
#define M_BAD_EVENTNAME          496
#define M_BAD_LEVENT             497
#define M_FOS_TRANS              498
#define M_READY_TERMINAL         499
#define M_unused_500             500  /* unused, was M_PROMETER_PERCENT */
#define M_unused_501             501  /* unused, was M_PROMETER_TX_CUR */
#define M_unused_502             502  /* unused, was M_PROMETER_RX_CUR */
#define M_unused_503             503  /* unused, was M_PROMETER_TX_MOH */
#define M_unused_504             504  /* unused, was M_PROMETER_RX_MOH */
#define M_TICKER                 505  /* MR 971204 */
#define M_CSS_CHARS_ASC          506
#define M_CSS_CHARS_GRA          507
#define M_CSS_TX_FNAME           508
#define M_CSS_RX_FNAME           509
#define M_CSS_CAPTION            510
#define M_CSS_TX_CUR             511
#define M_CSS_TX_TOT             512
#define M_CSS_RX_CUR             513
#define M_CSS_RX_TOT             514
#define M_MCP_FROZEN             515
#define M_MCP_INIT               516
#define M_MCP_WAITING            517
#define M_MCP_SHELLED            518
#define M_MCP_SPAWNED            519
#define M_MCP_HANGUP             520
#define M_MCP_ANSWERING          521
#define M_MCP_CONNECT            522
#define M_MCP_FAX_RECV           523
#define M_MCP_DIALING            524
#define M_MCP_MAIL_XFER          525
#define M_MCP_USERNAME           526
#define M_MCP_EXITING            527
#define M_MCP_RETURNING          528
#define M_MCP_COULDNTOPENPIPE    529
#define M_FAX_RECEIVED_PAGES     530
#define M_FAX_TRANSMISSION_ERROR 531
#define M_FAX_CREATE_FAILED      532
#define M_FAX_RECV               533
#define M_FAX_CONNECT_WITH       534
#define M_FAX_FILE_RECEIVED      535
#define M_FAX_RECEIVE_PAGE_ERROR 536
#define M_unused_537             537  /* unused */
#define M_unused_538             538  /* unused */
#define M_unused_539             539  /* unused */
#define M_OLD_BSY_IGNORED        540
#define M_FROZEN                 541
#define M_UNFROZEN               542
#define M_SPAWN_NO_OK_COMMAND    543
#define M_CACHEHOLD_STAT         544
#define M_COULD_NOT_RENAME       545
#define M_REMOTE_MOH             546
#define M_REMOTE_REFUSED_MAIL_PU 547
#define M_LOCAL_MOH              548
#define M_NO_YOOHOO_2U2          549
#define M_DISABLING_BIDI         550
#define M_ADDRS_SKIPPED          551
#define M_unused_552             552  /* unused, was M_MCP_INITIATED_SHUTDOWN */
#define M_unused_553             553  /* unused, was M_MCP_MESSAGE_TRUNCATED */
#define M_unused_554             554  /* unused, was M_MCP_ERR_DOSREAD */
#define M_unused_555             555  /* unused, was M_MCP_ERR_READPIPE */
#define M_unused_556             556  /* unused */
#define M_MCP_RESCAN             557
#define M_MCP_READING            558
#define M_MCP_TERM               559
#define M_POSTING_SEMAPHORE      560
#define M_NO_SEMAPHORE           561
#define M_unused_562             562  /* unused */
#define M_ERRLVL_POLL            563
#define M_ERRLVL_POLL_FAIL       564
#define M_MAILER_HELP_EXTRA      565  // 10 extra lines
#define M_MAIL_HELP_EXTRA_END    574

#define M_EMSI_SYSTEM            575
#define M_EMSI_AKA               576
#define M_EMSI_EMPTY             577
#define M_EMSI_SYSOP             578
#define M_EMSI_FLAGS             579
#define M_EMSI_PHONE             580
#define M_unused_581             581
#define M_unused_582             582
#define M_EMSI_REMTIME           583
#define M_EMSI_LOCTIME           584
#define M_EMSI_UTCDIFF           585
#define M_EMSI_ADDON             586
#define M_EMSI_FULL              587

/*
 * Easy way to test language file:
 * Always insert new messages before this one and bump his number.
 * Then startup Bink WITHOUT serial in binkley.cfg, and if you see
 * (UNREGISTERED) on the screen, you still have it OK.
 */

#define M_UNREGISTERED           595

/*
 * This last define should be one more than the last message number.
 * Always update this last line when you add or subtract messages!
 */

#define X_TOTAL_MSGS             596

/* $Id: msgs.h,v 1.3 1999/02/28 12:46:02 hjk Exp $ */
