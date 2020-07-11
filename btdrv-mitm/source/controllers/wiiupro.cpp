
#include "wiiupro.hpp"

#include <stratosphere.hpp>

#include "../btdrv_mitm_logging.hpp"

namespace ams::controller {

    WiiUProController::WiiUProController(const bluetooth::Address *address) 
    : WiiController(ControllerType_WiiUPro, address) {

    }

    Result WiiUProController::initialize(void) {
        WiiController::initialize();
        
        // This should actually probably be run in response to report 0x20
        R_TRY(this->sendInit1(&m_address));
        R_TRY(this->sendInit2(&m_address));
        R_TRY(this->setReportMode(&m_address, 0x34));

        return 0;
    }

    Result WiiUProController::sendInit1(const bluetooth::Address *address) {
        const uint8_t data[] = {0x55};
        return this->writeMemory(address, 0x04a400f0, data, sizeof(data));
    }

    Result WiiUProController::sendInit2(const bluetooth::Address *address) {
        const uint8_t data[] = {0x00};
        return this->writeMemory(address, 0x04a400fb, data, sizeof(data));
    }

    void WiiUProController::convertReportFormat(const bluetooth::HidReport *inReport, bluetooth::HidReport *outReport) {
        auto wiiUReport = reinterpret_cast<const WiiReportData *>(&inReport->data);
        auto switchReport = reinterpret_cast<SwitchReportData *>(&outReport->data);

        outReport->size = 0x31;
        switchReport->id = 0x30;
        switchReport->input0x30.conn_info = 0x0;
        switchReport->input0x30.battery = 0x8;

        switch(wiiUReport->id) {
            case 0x20:  //extension connected
                this->handleInputReport0x20(wiiUReport, switchReport);
                break;

            //case 0x22:  // Acknowledgement
                //break;

            //case 0x32:  // Buttons + Ext bytes
                //break;

            case 0x34:  // Buttons + 19 ext bytes
                this->handleInputReport0x34(wiiUReport, switchReport);
                break;

            default:
                BTDRV_LOG_FMT("WIIUPRO: RECEIVED REPORT [0x%02x]", wiiUReport->id);
                break;
        }
    }

    void WiiUProController::handleInputReport0x20(const WiiReportData *src, SwitchReportData *dst) {

    }

    void WiiUProController::handleInputReport0x34(const WiiReportData *src, SwitchReportData *dst) {
        auto extension = reinterpret_cast<const WiiUProExtensionData *>(src->input0x34.extension);

        packStickData(&dst->input0x30.left_stick,
            ((3 * (extension->left_stick_x - STICK_ZERO)) >> 1) + STICK_ZERO, 
            ((3 * (extension->left_stick_y - STICK_ZERO)) >> 1) + STICK_ZERO
        );
        packStickData(&dst->input0x30.right_stick,
            ((3 * (extension->right_stick_x - STICK_ZERO)) >> 1) + STICK_ZERO,
            ((3 * (extension->right_stick_y - STICK_ZERO)) >> 1) + STICK_ZERO
        );

        dst->input0x30.buttons.dpad_down   = !extension->buttons.dpad_down;
        dst->input0x30.buttons.dpad_up     = !extension->buttons.dpad_up;
        dst->input0x30.buttons.dpad_right  = !extension->buttons.dpad_right;
        dst->input0x30.buttons.dpad_left   = !extension->buttons.dpad_left;

        dst->input0x30.buttons.A = !extension->buttons.A;
        dst->input0x30.buttons.B = !extension->buttons.B;
        dst->input0x30.buttons.X = !extension->buttons.X;
        dst->input0x30.buttons.Y = !extension->buttons.Y;

        dst->input0x30.buttons.R  = !extension->buttons.R;
        dst->input0x30.buttons.ZR = !extension->buttons.ZR;
        dst->input0x30.buttons.L  = !extension->buttons.L;
        dst->input0x30.buttons.ZL = !extension->buttons.ZL;

        dst->input0x30.buttons.minus = !extension->buttons.minus;
        dst->input0x30.buttons.plus  = !extension->buttons.plus;

        dst->input0x30.buttons.lstick_press = !extension->buttons.lstick_press;
        dst->input0x30.buttons.rstick_press = !extension->buttons.rstick_press;

        dst->input0x30.buttons.home = !extension->buttons.home;
    }

}
