#pragma once

// ESP32 RMT clockless controller
// this is a stripped template of ClocklessController from FastLED that does not include
// gpio as template parameter
template <EOrder RGB_ORDER = RGB, int XTRA0 = 0, bool FLIP = false, int WAIT_TIME = 5>
class ESP32RMT_ClocklessController : public CPixelLEDController<RGB_ORDER>
{
private:

    // -- The actual controller object for ESP32
    ESP32RMTController mRMTController;

    // -- Verify that the pin is valid
    // no static checks for run-time defined gpio
    //static_assert(FastPin<DATA_PIN>::validpin(), "Invalid pin specified");

public:

    ESP32RMT_ClocklessController(uint8_t pin, unsigned t1, unsigned t2, unsigned t3)
        : mRMTController(pin, t1, t2, t3, FASTLED_RMT_MAX_CHANNELS, FASTLED_RMT_MEM_BLOCKS)
        {}

    void init()
    {
    }

    virtual uint16_t getMaxRefreshRate() const { return 400; }

protected:

    // -- Load pixel data
    //    This method loads all of the pixel data into a separate buffer for use by
    //    by the RMT driver. Copying does two important jobs: it fixes the color
    //    order for the pixels, and it performs the scaling/adjusting ahead of time.
    //    It also packs the bytes into 32 bit chunks with the right bit order.
    void loadPixelData(PixelController<RGB_ORDER> & pixels)
    {
        // -- Make sure the buffer is allocated
        int size_in_bytes = pixels.size() * 3;
        uint8_t * pData = mRMTController.getPixelBuffer(size_in_bytes);

        // -- This might be faster
        while (pixels.has(1)) {
            *pData++ = pixels.loadAndScale0();
            *pData++ = pixels.loadAndScale1();
            *pData++ = pixels.loadAndScale2();
            pixels.advanceData();
            pixels.stepDithering();
        }
    }

    // -- Show pixels
    //    This is the main entry point for the controller.
    virtual void showPixels(PixelController<RGB_ORDER> & pixels)
    {
        if (FASTLED_RMT_BUILTIN_DRIVER) {
            convertAllPixelData(pixels);
        } else {
            loadPixelData(pixels);
        }

        mRMTController.showPixels();
    }

    // -- Convert all pixels to RMT pulses
    //    This function is only used when the user chooses to use the
    //    built-in RMT driver, which needs all of the RMT pulses
    //    up-front.
    void convertAllPixelData(PixelController<RGB_ORDER> & pixels)
    {
        // -- Make sure the data buffer is allocated
        mRMTController.initPulseBuffer(pixels.size() * 3);

        // -- Cycle through the R,G, and B values in the right order,
        //    storing the pulses in the big buffer

        uint32_t byteval;
        while (pixels.has(1)) {
            byteval = pixels.loadAndScale0();
            mRMTController.convertByte(byteval);
            byteval = pixels.loadAndScale1();
            mRMTController.convertByte(byteval);
            byteval = pixels.loadAndScale2();
            mRMTController.convertByte(byteval);
            pixels.advanceData();
            pixels.stepDithering();
        }
    }
};


// WS2812 - 250ns, 625ns, 375ns
// a reduced template for WS2812Controller800Khz that instatiates clockless controller with run-time gpio

template <EOrder RGB_ORDER = RGB>
class ESP32RMT_WS2812Controller800Khz : public ESP32RMT_ClocklessController<RGB_ORDER> {
public:
    ESP32RMT_WS2812Controller800Khz(uint8_t pin) : ESP32RMT_ClocklessController<RGB_ORDER>(pin, C_NS(250), C_NS(625), C_NS(375)) {}
};

/*
 WS2812 controller class @ 800 KHz.
 with RTM run-time defined gpio
*/
template<EOrder RGB_ORDER = RGB>
class ESP32RMT_WS2812B : public ESP32RMT_WS2812Controller800Khz<RGB_ORDER> {
public:
    ESP32RMT_WS2812B(uint8_t pin) : ESP32RMT_WS2812Controller800Khz<RGB_ORDER>(pin){}
};