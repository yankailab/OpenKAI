#pragma once
// MESSAGE SERIAL_UDB_EXTRA_F13 PACKING

#define MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13 177


typedef struct __mavlink_serial_udb_extra_f13_t {
 int32_t sue_lat_origin; /*<  Serial UDB Extra MP Origin Latitude*/
 int32_t sue_lon_origin; /*<  Serial UDB Extra MP Origin Longitude*/
 int32_t sue_alt_origin; /*<  Serial UDB Extra MP Origin Altitude Above Sea Level*/
 int16_t sue_week_no; /*<  Serial UDB Extra GPS Week Number*/
} mavlink_serial_udb_extra_f13_t;

#define MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN 14
#define MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN 14
#define MAVLINK_MSG_ID_177_LEN 14
#define MAVLINK_MSG_ID_177_MIN_LEN 14

#define MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC 249
#define MAVLINK_MSG_ID_177_CRC 249



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_SERIAL_UDB_EXTRA_F13 { \
    177, \
    "SERIAL_UDB_EXTRA_F13", \
    4, \
    {  { "sue_week_no", NULL, MAVLINK_TYPE_INT16_T, 0, 12, offsetof(mavlink_serial_udb_extra_f13_t, sue_week_no) }, \
         { "sue_lat_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_serial_udb_extra_f13_t, sue_lat_origin) }, \
         { "sue_lon_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_serial_udb_extra_f13_t, sue_lon_origin) }, \
         { "sue_alt_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_serial_udb_extra_f13_t, sue_alt_origin) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_SERIAL_UDB_EXTRA_F13 { \
    "SERIAL_UDB_EXTRA_F13", \
    4, \
    {  { "sue_week_no", NULL, MAVLINK_TYPE_INT16_T, 0, 12, offsetof(mavlink_serial_udb_extra_f13_t, sue_week_no) }, \
         { "sue_lat_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_serial_udb_extra_f13_t, sue_lat_origin) }, \
         { "sue_lon_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_serial_udb_extra_f13_t, sue_lon_origin) }, \
         { "sue_alt_origin", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_serial_udb_extra_f13_t, sue_alt_origin) }, \
         } \
}
#endif

/**
 * @brief Pack a serial_udb_extra_f13 message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param sue_week_no  Serial UDB Extra GPS Week Number
 * @param sue_lat_origin  Serial UDB Extra MP Origin Latitude
 * @param sue_lon_origin  Serial UDB Extra MP Origin Longitude
 * @param sue_alt_origin  Serial UDB Extra MP Origin Altitude Above Sea Level
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int16_t sue_week_no, int32_t sue_lat_origin, int32_t sue_lon_origin, int32_t sue_alt_origin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN];
    _mav_put_int32_t(buf, 0, sue_lat_origin);
    _mav_put_int32_t(buf, 4, sue_lon_origin);
    _mav_put_int32_t(buf, 8, sue_alt_origin);
    _mav_put_int16_t(buf, 12, sue_week_no);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#else
    mavlink_serial_udb_extra_f13_t packet;
    packet.sue_lat_origin = sue_lat_origin;
    packet.sue_lon_origin = sue_lon_origin;
    packet.sue_alt_origin = sue_alt_origin;
    packet.sue_week_no = sue_week_no;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
}

/**
 * @brief Pack a serial_udb_extra_f13 message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param sue_week_no  Serial UDB Extra GPS Week Number
 * @param sue_lat_origin  Serial UDB Extra MP Origin Latitude
 * @param sue_lon_origin  Serial UDB Extra MP Origin Longitude
 * @param sue_alt_origin  Serial UDB Extra MP Origin Altitude Above Sea Level
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               int16_t sue_week_no, int32_t sue_lat_origin, int32_t sue_lon_origin, int32_t sue_alt_origin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN];
    _mav_put_int32_t(buf, 0, sue_lat_origin);
    _mav_put_int32_t(buf, 4, sue_lon_origin);
    _mav_put_int32_t(buf, 8, sue_alt_origin);
    _mav_put_int16_t(buf, 12, sue_week_no);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#else
    mavlink_serial_udb_extra_f13_t packet;
    packet.sue_lat_origin = sue_lat_origin;
    packet.sue_lon_origin = sue_lon_origin;
    packet.sue_alt_origin = sue_alt_origin;
    packet.sue_week_no = sue_week_no;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#endif
}

/**
 * @brief Pack a serial_udb_extra_f13 message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param sue_week_no  Serial UDB Extra GPS Week Number
 * @param sue_lat_origin  Serial UDB Extra MP Origin Latitude
 * @param sue_lon_origin  Serial UDB Extra MP Origin Longitude
 * @param sue_alt_origin  Serial UDB Extra MP Origin Altitude Above Sea Level
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int16_t sue_week_no,int32_t sue_lat_origin,int32_t sue_lon_origin,int32_t sue_alt_origin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN];
    _mav_put_int32_t(buf, 0, sue_lat_origin);
    _mav_put_int32_t(buf, 4, sue_lon_origin);
    _mav_put_int32_t(buf, 8, sue_alt_origin);
    _mav_put_int16_t(buf, 12, sue_week_no);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#else
    mavlink_serial_udb_extra_f13_t packet;
    packet.sue_lat_origin = sue_lat_origin;
    packet.sue_lon_origin = sue_lon_origin;
    packet.sue_alt_origin = sue_alt_origin;
    packet.sue_week_no = sue_week_no;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
}

/**
 * @brief Encode a serial_udb_extra_f13 struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param serial_udb_extra_f13 C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_serial_udb_extra_f13_t* serial_udb_extra_f13)
{
    return mavlink_msg_serial_udb_extra_f13_pack(system_id, component_id, msg, serial_udb_extra_f13->sue_week_no, serial_udb_extra_f13->sue_lat_origin, serial_udb_extra_f13->sue_lon_origin, serial_udb_extra_f13->sue_alt_origin);
}

/**
 * @brief Encode a serial_udb_extra_f13 struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param serial_udb_extra_f13 C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_serial_udb_extra_f13_t* serial_udb_extra_f13)
{
    return mavlink_msg_serial_udb_extra_f13_pack_chan(system_id, component_id, chan, msg, serial_udb_extra_f13->sue_week_no, serial_udb_extra_f13->sue_lat_origin, serial_udb_extra_f13->sue_lon_origin, serial_udb_extra_f13->sue_alt_origin);
}

/**
 * @brief Encode a serial_udb_extra_f13 struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param serial_udb_extra_f13 C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_serial_udb_extra_f13_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_serial_udb_extra_f13_t* serial_udb_extra_f13)
{
    return mavlink_msg_serial_udb_extra_f13_pack_status(system_id, component_id, _status, msg,  serial_udb_extra_f13->sue_week_no, serial_udb_extra_f13->sue_lat_origin, serial_udb_extra_f13->sue_lon_origin, serial_udb_extra_f13->sue_alt_origin);
}

/**
 * @brief Send a serial_udb_extra_f13 message
 * @param chan MAVLink channel to send the message
 *
 * @param sue_week_no  Serial UDB Extra GPS Week Number
 * @param sue_lat_origin  Serial UDB Extra MP Origin Latitude
 * @param sue_lon_origin  Serial UDB Extra MP Origin Longitude
 * @param sue_alt_origin  Serial UDB Extra MP Origin Altitude Above Sea Level
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_serial_udb_extra_f13_send(mavlink_channel_t chan, int16_t sue_week_no, int32_t sue_lat_origin, int32_t sue_lon_origin, int32_t sue_alt_origin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN];
    _mav_put_int32_t(buf, 0, sue_lat_origin);
    _mav_put_int32_t(buf, 4, sue_lon_origin);
    _mav_put_int32_t(buf, 8, sue_alt_origin);
    _mav_put_int16_t(buf, 12, sue_week_no);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13, buf, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#else
    mavlink_serial_udb_extra_f13_t packet;
    packet.sue_lat_origin = sue_lat_origin;
    packet.sue_lon_origin = sue_lon_origin;
    packet.sue_alt_origin = sue_alt_origin;
    packet.sue_week_no = sue_week_no;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13, (const char *)&packet, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#endif
}

/**
 * @brief Send a serial_udb_extra_f13 message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_serial_udb_extra_f13_send_struct(mavlink_channel_t chan, const mavlink_serial_udb_extra_f13_t* serial_udb_extra_f13)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_serial_udb_extra_f13_send(chan, serial_udb_extra_f13->sue_week_no, serial_udb_extra_f13->sue_lat_origin, serial_udb_extra_f13->sue_lon_origin, serial_udb_extra_f13->sue_alt_origin);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13, (const char *)serial_udb_extra_f13, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#endif
}

#if MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_serial_udb_extra_f13_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int16_t sue_week_no, int32_t sue_lat_origin, int32_t sue_lon_origin, int32_t sue_alt_origin)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, sue_lat_origin);
    _mav_put_int32_t(buf, 4, sue_lon_origin);
    _mav_put_int32_t(buf, 8, sue_alt_origin);
    _mav_put_int16_t(buf, 12, sue_week_no);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13, buf, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#else
    mavlink_serial_udb_extra_f13_t *packet = (mavlink_serial_udb_extra_f13_t *)msgbuf;
    packet->sue_lat_origin = sue_lat_origin;
    packet->sue_lon_origin = sue_lon_origin;
    packet->sue_alt_origin = sue_alt_origin;
    packet->sue_week_no = sue_week_no;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13, (const char *)packet, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_MIN_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_CRC);
#endif
}
#endif

#endif

// MESSAGE SERIAL_UDB_EXTRA_F13 UNPACKING


/**
 * @brief Get field sue_week_no from serial_udb_extra_f13 message
 *
 * @return  Serial UDB Extra GPS Week Number
 */
static inline int16_t mavlink_msg_serial_udb_extra_f13_get_sue_week_no(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  12);
}

/**
 * @brief Get field sue_lat_origin from serial_udb_extra_f13 message
 *
 * @return  Serial UDB Extra MP Origin Latitude
 */
static inline int32_t mavlink_msg_serial_udb_extra_f13_get_sue_lat_origin(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field sue_lon_origin from serial_udb_extra_f13 message
 *
 * @return  Serial UDB Extra MP Origin Longitude
 */
static inline int32_t mavlink_msg_serial_udb_extra_f13_get_sue_lon_origin(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field sue_alt_origin from serial_udb_extra_f13 message
 *
 * @return  Serial UDB Extra MP Origin Altitude Above Sea Level
 */
static inline int32_t mavlink_msg_serial_udb_extra_f13_get_sue_alt_origin(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Decode a serial_udb_extra_f13 message into a struct
 *
 * @param msg The message to decode
 * @param serial_udb_extra_f13 C-struct to decode the message contents into
 */
static inline void mavlink_msg_serial_udb_extra_f13_decode(const mavlink_message_t* msg, mavlink_serial_udb_extra_f13_t* serial_udb_extra_f13)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    serial_udb_extra_f13->sue_lat_origin = mavlink_msg_serial_udb_extra_f13_get_sue_lat_origin(msg);
    serial_udb_extra_f13->sue_lon_origin = mavlink_msg_serial_udb_extra_f13_get_sue_lon_origin(msg);
    serial_udb_extra_f13->sue_alt_origin = mavlink_msg_serial_udb_extra_f13_get_sue_alt_origin(msg);
    serial_udb_extra_f13->sue_week_no = mavlink_msg_serial_udb_extra_f13_get_sue_week_no(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN? msg->len : MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN;
        memset(serial_udb_extra_f13, 0, MAVLINK_MSG_ID_SERIAL_UDB_EXTRA_F13_LEN);
    memcpy(serial_udb_extra_f13, _MAV_PAYLOAD(msg), len);
#endif
}
