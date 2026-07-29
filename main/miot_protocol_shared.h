#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ============================================================
 * Shared protocol estimation helpers — used by both ble_manager.c
 * (BLE task) and main.c (app task) to avoid code duplication.
 * All functions are stateless; all external inputs are passed as
 * parameters.
 * ============================================================ */

static inline float min_dist_to_pd(float voltage) {
    static const float PD_FV[] = {5.0f, 9.0f, 12.0f, 15.0f, 20.0f};
    float d = fabsf(voltage - PD_FV[0]);
    for (int i = 1; i < 5; i++) {
        float nd = fabsf(voltage - PD_FV[i]);
        if (nd < d) d = nd;
    }
    return d;
}

static inline uint8_t estimate_pd_subtype(float voltage) {
    float md = min_dist_to_pd(voltage);
    if (voltage < 12.0f) {
        if (md <= 0.05f) return 7;
        return 8;
    }
    if (md <= 0.3f) return 7;
    if (voltage >= 3.0f && voltage <= 21.0f) return 8;
    return 7;
}

/* Unified protocol estimation.
 *
 * piid        — property ID (1-4 for ports)
 * voltage     — port voltage
 * code        — hardware protocol code byte from port data
 * pd_enabled  — whether PD is enabled for this port
 * pps_enabled — whether PPS is enabled for this port
 * pdo_kind    — PDO kind (high byte of port's PDO word), 0 if unknown
 * hw_protocol — hardware protocol from PIID 17/18, 0 if unknown
 *
 * Returns protocol code: 1=5V, 3=QC, 7=PD, 8=PPS, 10=UFCS, etc.
 * 0 = unknown/idle.
 */
static inline uint8_t estimate_protocol_shared(uint8_t piid, float voltage, uint8_t code,
                                                bool pd_enabled, bool pps_enabled,
                                                uint8_t pdo_kind, uint8_t hw_protocol) {
    /* Hardware protocol code takes priority */
    if (hw_protocol > 0) return hw_protocol;

    if (piid == 1 || piid == 2) {
        if (!pd_enabled && voltage > 0) return 1;  // 5V fallback
        if (code == 0x08) return 8;
        if (code == 0x70) {
            if (min_dist_to_pd(voltage) <= 0.05f) return 7;
            return 3;
        }
        if (code == 0x01 || code == 0x03 || code == 0x04 || code == 0x05 ||
            code == 0x06 || code == 0x07 || code == 0x0A || code == 0x0B || code == 0x30) {
            if (pdo_kind == 0x08) {  // PDO PPS
                if (!pps_enabled) return 7;
                float md = min_dist_to_pd(voltage);
                return (md <= 0.05f) ? 7 : 8;
            } else if (pdo_kind == 0x07) {  // PDO PD Fixed
                if (pps_enabled && voltage < 12.0f)
                    return estimate_pd_subtype(voltage);
                return 7;
            }
            // No PDO or unknown kind
            if (!pps_enabled) return 7;
            return estimate_pd_subtype(voltage);
        }
        float md = min_dist_to_pd(voltage);
        if (md <= 0.5f) return 7;
        if (voltage >= 3.0f && voltage <= 21.0f) return 8;
        return 0;
    }

    if (piid == 3) {
        if (code == 0x70) {
            if (pdo_kind == 0x07 || pdo_kind == 0x08) return 7;
            return 3;
        }
        if (voltage >= 15.0f) return 7;
        if (voltage >= 8.5f) return 3;
        if (voltage <= 5.5f) return 1;
        return voltage > 6.0f ? 3 : 1;
    }

    if (piid == 4) {
        if (code == 0x70) return 3;
        if (voltage > 5.5f) return 3;
        if (voltage > 0) return 1;
    }
    return 0;
}
