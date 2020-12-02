/* ***********************************************************
 * This file was automatically generated on 2020-11-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_e_paper_296x128.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef TF_IMPLEMENT_CALLBACKS
static bool tf_e_paper_296x128_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    TF_EPaper296x128 *e_paper_296x128 = (TF_EPaper296x128 *) dev;
    (void)payload;

    switch(fid) {

        case TF_E_PAPER_296X128_CALLBACK_DRAW_STATUS: {
            TF_EPaper296x128DrawStatusHandler fn = e_paper_296x128->draw_status_handler;
            void *user_data = e_paper_296x128->draw_status_user_data;
            if (fn == NULL)
                return false;

            uint8_t draw_status = tf_packetbuffer_read_uint8_t(payload);
            TF_HalCommon *common = tf_hal_get_common(e_paper_296x128->tfp->hal);
            common->locked = true;
            fn(e_paper_296x128, draw_status, user_data);
            common->locked = false;
            break;
        }
        default:
            return false;
    }

    return true;
}
#else
static bool tf_e_paper_296x128_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    return false;
}
#endif
int tf_e_paper_296x128_create(TF_EPaper296x128 *e_paper_296x128, const char *uid, TF_HalContext *hal) {
    memset(e_paper_296x128, 0, sizeof(TF_EPaper296x128));

    uint32_t numeric_uid;
    int rc = tf_base58_decode(uid, &numeric_uid);
    if (rc != TF_E_OK) {
        return rc;
    }

    uint8_t port_id;
    int inventory_index;
    rc = tf_hal_get_port_id(hal, numeric_uid, &port_id, &inventory_index);
    if (rc < 0) {
        return rc;
    }

    //rc = tf_tfp_init(e_paper_296x128->tfp, numeric_uid, TF_E_PAPER_296X128_DEVICE_IDENTIFIER, hal, port_id, inventory_index, tf_e_paper_296x128_callback_handler);
    rc = tf_hal_get_tfp(hal, &e_paper_296x128->tfp, TF_E_PAPER_296X128_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    e_paper_296x128->tfp->device = e_paper_296x128;
    e_paper_296x128->tfp->cb_handler = tf_e_paper_296x128_callback_handler;
    
    e_paper_296x128->response_expected[0] = 0x06;
    e_paper_296x128->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_e_paper_296x128_destroy(TF_EPaper296x128 *e_paper_296x128) {
    int result = tf_tfp_destroy(e_paper_296x128->tfp);
    e_paper_296x128->tfp = NULL;
    return result;
}

int tf_e_paper_296x128_get_response_expected(TF_EPaper296x128 *e_paper_296x128, uint8_t function_id, bool *ret_response_expected) {
    switch(function_id) {
        case TF_E_PAPER_296X128_FUNCTION_DRAW:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_BLACK_WHITE_LOW_LEVEL:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_COLOR_LOW_LEVEL:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_FILL_DISPLAY:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_TEXT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_LINE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_BOX:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_UPDATE_MODE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_TYPE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[1] & (1 << 0)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[1] & (1 << 1)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[1] & (1 << 2)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[1] & (1 << 3)) != 0;
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (e_paper_296x128->response_expected[1] & (1 << 4)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_e_paper_296x128_set_response_expected(TF_EPaper296x128 *e_paper_296x128, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_E_PAPER_296X128_FUNCTION_DRAW:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 0);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_BLACK_WHITE_LOW_LEVEL:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 1);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_COLOR_LOW_LEVEL:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 2);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_FILL_DISPLAY:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 3);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_TEXT:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 4);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_LINE:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 5);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_DRAW_BOX:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 6);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_UPDATE_MODE:
            if (response_expected) {
                e_paper_296x128->response_expected[0] |= (1 << 7);
            } else {
                e_paper_296x128->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_TYPE:
            if (response_expected) {
                e_paper_296x128->response_expected[1] |= (1 << 0);
            } else {
                e_paper_296x128->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                e_paper_296x128->response_expected[1] |= (1 << 1);
            } else {
                e_paper_296x128->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                e_paper_296x128->response_expected[1] |= (1 << 2);
            } else {
                e_paper_296x128->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_RESET:
            if (response_expected) {
                e_paper_296x128->response_expected[1] |= (1 << 3);
            } else {
                e_paper_296x128->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_E_PAPER_296X128_FUNCTION_WRITE_UID:
            if (response_expected) {
                e_paper_296x128->response_expected[1] |= (1 << 4);
            } else {
                e_paper_296x128->response_expected[1] &= ~(1 << 4);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_e_paper_296x128_set_response_expected_all(TF_EPaper296x128 *e_paper_296x128, bool response_expected) {
    memset(e_paper_296x128->response_expected, response_expected ? 0xFF : 0, 2);
}

int tf_e_paper_296x128_draw(TF_EPaper296x128 *e_paper_296x128) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_DRAW, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_DRAW, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_draw_status(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_draw_status) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_DRAW_STATUS, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_draw_status != NULL) { *ret_draw_status = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_write_black_white_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, bool pixels_chunk_data[432]) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_WRITE_BLACK_WHITE_LOW_LEVEL, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_WRITE_BLACK_WHITE_LOW_LEVEL, 64, 0, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    x_start = tf_leconvert_uint16_to(x_start); memcpy(buf + 0, &x_start, 2);
    buf[2] = (uint8_t)y_start;
    x_end = tf_leconvert_uint16_to(x_end); memcpy(buf + 3, &x_end, 2);
    buf[5] = (uint8_t)y_end;
    pixels_length = tf_leconvert_uint16_to(pixels_length); memcpy(buf + 6, &pixels_length, 2);
    pixels_chunk_offset = tf_leconvert_uint16_to(pixels_chunk_offset); memcpy(buf + 8, &pixels_chunk_offset, 2);
    memset(buf + 10, 0, 54); for (i = 0; i < 432; ++i) buf[10 + (i / 8)] |= (pixels_chunk_data[i] ? 1 : 0) << (i % 8);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_read_black_white_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[464]) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_READ_BLACK_WHITE_LOW_LEVEL, 6, 62, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    x_start = tf_leconvert_uint16_to(x_start); memcpy(buf + 0, &x_start, 2);
    buf[2] = (uint8_t)y_start;
    x_end = tf_leconvert_uint16_to(x_end); memcpy(buf + 3, &x_end, 2);
    buf[5] = (uint8_t)y_end;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_pixels_length != NULL) { *ret_pixels_length = tf_packetbuffer_read_uint16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        if (ret_pixels_chunk_offset != NULL) { *ret_pixels_chunk_offset = tf_packetbuffer_read_uint16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        if (ret_pixels_chunk_data != NULL) { tf_packetbuffer_read_bool_array(&e_paper_296x128->tfp->spitfp->recv_buf, ret_pixels_chunk_data, 464);} else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 58); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_write_color_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, bool pixels_chunk_data[432]) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_WRITE_COLOR_LOW_LEVEL, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_WRITE_COLOR_LOW_LEVEL, 64, 0, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    x_start = tf_leconvert_uint16_to(x_start); memcpy(buf + 0, &x_start, 2);
    buf[2] = (uint8_t)y_start;
    x_end = tf_leconvert_uint16_to(x_end); memcpy(buf + 3, &x_end, 2);
    buf[5] = (uint8_t)y_end;
    pixels_length = tf_leconvert_uint16_to(pixels_length); memcpy(buf + 6, &pixels_length, 2);
    pixels_chunk_offset = tf_leconvert_uint16_to(pixels_chunk_offset); memcpy(buf + 8, &pixels_chunk_offset, 2);
    memset(buf + 10, 0, 54); for (i = 0; i < 432; ++i) buf[10 + (i / 8)] |= (pixels_chunk_data[i] ? 1 : 0) << (i % 8);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_read_color_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[464]) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_READ_COLOR_LOW_LEVEL, 6, 62, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    x_start = tf_leconvert_uint16_to(x_start); memcpy(buf + 0, &x_start, 2);
    buf[2] = (uint8_t)y_start;
    x_end = tf_leconvert_uint16_to(x_end); memcpy(buf + 3, &x_end, 2);
    buf[5] = (uint8_t)y_end;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_pixels_length != NULL) { *ret_pixels_length = tf_packetbuffer_read_uint16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        if (ret_pixels_chunk_offset != NULL) { *ret_pixels_chunk_offset = tf_packetbuffer_read_uint16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        if (ret_pixels_chunk_data != NULL) { tf_packetbuffer_read_bool_array(&e_paper_296x128->tfp->spitfp->recv_buf, ret_pixels_chunk_data, 464);} else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 58); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_fill_display(TF_EPaper296x128 *e_paper_296x128, uint8_t color) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_FILL_DISPLAY, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_FILL_DISPLAY, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    buf[0] = (uint8_t)color;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_draw_text(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x, uint8_t position_y, uint8_t font, uint8_t color, uint8_t orientation, const char text[50]) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_DRAW_TEXT, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_DRAW_TEXT, 56, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    position_x = tf_leconvert_uint16_to(position_x); memcpy(buf + 0, &position_x, 2);
    buf[2] = (uint8_t)position_y;
    buf[3] = (uint8_t)font;
    buf[4] = (uint8_t)color;
    buf[5] = (uint8_t)orientation;
    memcpy(buf + 6, text, 50);


    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_draw_line(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x_start, uint8_t position_y_start, uint16_t position_x_end, uint8_t position_y_end, uint8_t color) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_DRAW_LINE, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_DRAW_LINE, 7, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    position_x_start = tf_leconvert_uint16_to(position_x_start); memcpy(buf + 0, &position_x_start, 2);
    buf[2] = (uint8_t)position_y_start;
    position_x_end = tf_leconvert_uint16_to(position_x_end); memcpy(buf + 3, &position_x_end, 2);
    buf[5] = (uint8_t)position_y_end;
    buf[6] = (uint8_t)color;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_draw_box(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x_start, uint8_t position_y_start, uint16_t position_x_end, uint8_t position_y_end, bool fill, uint8_t color) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_DRAW_BOX, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_DRAW_BOX, 8, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    position_x_start = tf_leconvert_uint16_to(position_x_start); memcpy(buf + 0, &position_x_start, 2);
    buf[2] = (uint8_t)position_y_start;
    position_x_end = tf_leconvert_uint16_to(position_x_end); memcpy(buf + 3, &position_x_end, 2);
    buf[5] = (uint8_t)position_y_end;
    buf[6] = fill ? 1 : 0;
    buf[7] = (uint8_t)color;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_set_update_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t update_mode) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_SET_UPDATE_MODE, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_SET_UPDATE_MODE, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    buf[0] = (uint8_t)update_mode;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_update_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_update_mode) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_UPDATE_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_update_mode != NULL) { *ret_update_mode = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_set_display_type(TF_EPaper296x128 *e_paper_296x128, uint8_t display_type) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_TYPE, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_TYPE, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    buf[0] = (uint8_t)display_type;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_display_type(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_display_type) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_DISPLAY_TYPE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_display_type != NULL) { *ret_display_type = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_spitfp_error_count(TF_EPaper296x128 *e_paper_296x128, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_set_bootloader_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t mode, uint8_t *ret_status) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_bootloader_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_mode) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_set_write_firmware_pointer(TF_EPaper296x128 *e_paper_296x128, uint32_t pointer) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_write_firmware(TF_EPaper296x128 *e_paper_296x128, uint8_t data[64], uint8_t *ret_status) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_set_status_led_config(TF_EPaper296x128 *e_paper_296x128, uint8_t config) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_status_led_config(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_config) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_chip_temperature(TF_EPaper296x128 *e_paper_296x128, int16_t *ret_temperature) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_reset(TF_EPaper296x128 *e_paper_296x128) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_write_uid(TF_EPaper296x128 *e_paper_296x128, uint32_t uid) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_e_paper_296x128_get_response_expected(e_paper_296x128, TF_E_PAPER_296X128_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(e_paper_296x128->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_read_uid(TF_EPaper296x128 *e_paper_296x128, uint32_t *ret_uid) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_get_identity(TF_EPaper296x128 *e_paper_296x128, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if(tf_hal_get_common(e_paper_296x128->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(e_paper_296x128->tfp, TF_E_PAPER_296X128_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(e_paper_296x128->tfp->hal) + tf_hal_get_common(e_paper_296x128->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(e_paper_296x128->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&e_paper_296x128->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 8); }
        *tmp_connected_uid = tf_packetbuffer_read_char(&e_paper_296x128->tfp->spitfp->recv_buf);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&e_paper_296x128->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&e_paper_296x128->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&e_paper_296x128->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name(e_paper_296x128->tfp->hal, e_paper_296x128->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(e_paper_296x128->tfp);
    }

    result = tf_tfp_finish_send(e_paper_296x128->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_e_paper_296x128_write_black_white(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *pixels, uint16_t pixels_length) {
    int ret = TF_E_OK;
    uint16_t pixels_chunk_offset = 0;
    bool pixels_chunk_data[432];
    uint16_t pixels_chunk_length = 0;

    if (pixels_length == 0) {
        memset(&pixels_chunk_data, 0, sizeof(bool) * 432);

        ret = tf_e_paper_296x128_write_black_white_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);
    } else {

        while (pixels_chunk_offset < pixels_length) {
            pixels_chunk_length = pixels_length - pixels_chunk_offset;

            if (pixels_chunk_length > 432) {
                pixels_chunk_length = 432;
            }

            memcpy(pixels_chunk_data, &pixels[pixels_chunk_offset], sizeof(bool) * pixels_chunk_length);
            memset(&pixels_chunk_data[pixels_chunk_length], 0, sizeof(bool) * (432 - pixels_chunk_length));

            ret = tf_e_paper_296x128_write_black_white_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                break;
            }

            pixels_chunk_offset += 432;
        }

    }

    return ret;
}

int tf_e_paper_296x128_read_black_white(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length) {
    int ret = TF_E_OK;
    uint16_t pixels_length = 0;
    uint16_t pixels_chunk_offset = 0;
    bool pixels_chunk_data[464];
    bool pixels_out_of_sync;
    uint16_t pixels_chunk_length = 0;

    *ret_pixels_length = 0;

    ret = tf_e_paper_296x128_read_black_white_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

    if (ret != TF_E_OK) {
        return ret;
    }

    pixels_out_of_sync = pixels_chunk_offset != 0;

    if (!pixels_out_of_sync) {
        pixels_chunk_length = pixels_length - pixels_chunk_offset;

        if (pixels_chunk_length > 464) {
            pixels_chunk_length = 464;
        }

        memcpy(ret_pixels, pixels_chunk_data, sizeof(bool) * pixels_chunk_length);
        *ret_pixels_length = pixels_chunk_length;

        while (*ret_pixels_length < pixels_length) {
            ret = tf_e_paper_296x128_read_black_white_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }

            pixels_out_of_sync = pixels_chunk_offset != *ret_pixels_length;

            if (pixels_out_of_sync) {
                break;
            }

            pixels_chunk_length = pixels_length - pixels_chunk_offset;

            if (pixels_chunk_length > 464) {
                pixels_chunk_length = 464;
            }

            memcpy(&ret_pixels[*ret_pixels_length], pixels_chunk_data, sizeof(bool) * pixels_chunk_length);
            *ret_pixels_length += pixels_chunk_length;
        }
    }

    if (pixels_out_of_sync) {
        *ret_pixels_length = 0; // return empty array

        // discard remaining stream to bring it back in-sync
        while (pixels_chunk_offset + 464 < pixels_length) {
            ret = tf_e_paper_296x128_read_black_white_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }
        }

        ret = TF_E_STREAM_OUT_OF_SYNC;
    }

    return ret;
}

int tf_e_paper_296x128_write_color(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *pixels, uint16_t pixels_length) {
    int ret = TF_E_OK;
    uint16_t pixels_chunk_offset = 0;
    bool pixels_chunk_data[432];
    uint16_t pixels_chunk_length = 0;

    if (pixels_length == 0) {
        memset(&pixels_chunk_data, 0, sizeof(bool) * 432);

        ret = tf_e_paper_296x128_write_color_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);
    } else {

        while (pixels_chunk_offset < pixels_length) {
            pixels_chunk_length = pixels_length - pixels_chunk_offset;

            if (pixels_chunk_length > 432) {
                pixels_chunk_length = 432;
            }

            memcpy(pixels_chunk_data, &pixels[pixels_chunk_offset], sizeof(bool) * pixels_chunk_length);
            memset(&pixels_chunk_data[pixels_chunk_length], 0, sizeof(bool) * (432 - pixels_chunk_length));

            ret = tf_e_paper_296x128_write_color_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                break;
            }

            pixels_chunk_offset += 432;
        }

    }

    return ret;
}

int tf_e_paper_296x128_read_color(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length) {
    int ret = TF_E_OK;
    uint16_t pixels_length = 0;
    uint16_t pixels_chunk_offset = 0;
    bool pixels_chunk_data[464];
    bool pixels_out_of_sync;
    uint16_t pixels_chunk_length = 0;

    *ret_pixels_length = 0;

    ret = tf_e_paper_296x128_read_color_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

    if (ret != TF_E_OK) {
        return ret;
    }

    pixels_out_of_sync = pixels_chunk_offset != 0;

    if (!pixels_out_of_sync) {
        pixels_chunk_length = pixels_length - pixels_chunk_offset;

        if (pixels_chunk_length > 464) {
            pixels_chunk_length = 464;
        }

        memcpy(ret_pixels, pixels_chunk_data, sizeof(bool) * pixels_chunk_length);
        *ret_pixels_length = pixels_chunk_length;

        while (*ret_pixels_length < pixels_length) {
            ret = tf_e_paper_296x128_read_color_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }

            pixels_out_of_sync = pixels_chunk_offset != *ret_pixels_length;

            if (pixels_out_of_sync) {
                break;
            }

            pixels_chunk_length = pixels_length - pixels_chunk_offset;

            if (pixels_chunk_length > 464) {
                pixels_chunk_length = 464;
            }

            memcpy(&ret_pixels[*ret_pixels_length], pixels_chunk_data, sizeof(bool) * pixels_chunk_length);
            *ret_pixels_length += pixels_chunk_length;
        }
    }

    if (pixels_out_of_sync) {
        *ret_pixels_length = 0; // return empty array

        // discard remaining stream to bring it back in-sync
        while (pixels_chunk_offset + 464 < pixels_length) {
            ret = tf_e_paper_296x128_read_color_low_level(e_paper_296x128, x_start, y_start, x_end, y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }
        }

        ret = TF_E_STREAM_OUT_OF_SYNC;
    }

    return ret;
}
#ifdef TF_IMPLEMENT_CALLBACKS
void tf_e_paper_296x128_register_draw_status_callback(TF_EPaper296x128 *e_paper_296x128, TF_EPaper296x128DrawStatusHandler handler, void *user_data) {
    if (handler == NULL) {
        e_paper_296x128->tfp->needs_callback_tick = false;
        
    } else {
        e_paper_296x128->tfp->needs_callback_tick = true;
    }
    e_paper_296x128->draw_status_handler = handler;
    e_paper_296x128->draw_status_user_data = user_data;
}
#endif
int tf_e_paper_296x128_callback_tick(TF_EPaper296x128 *e_paper_296x128, uint32_t timeout_us) {
    return tf_tfp_callback_tick(e_paper_296x128->tfp, tf_hal_current_time_us(e_paper_296x128->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
