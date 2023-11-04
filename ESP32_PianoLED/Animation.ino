bool HueChange; // Boolean to control hue change

void Animatons(int selectedAnimation) {
  //Animation(temp name_1)
  if (selectedAnimation == 0) {
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 1) {
    currentPalette = RainbowStripeColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 2) {
    currentPalette = OceanColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 3) {
    currentPalette = CloudColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 4) {
    currentPalette = LavaColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 5) {
    currentPalette = ForestColors_p;
    currentBlending = LINEARBLEND;
  } else if (selectedAnimation == 6) {
    currentPalette = PartyColors_p;
    currentBlending = LINEARBLEND;
  }
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex) {
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

void sineWave() {
  uint16_t sinBeat = beatsin16(15, 0, NUM_LEDS - 1, 0, 0);

  // Set the LED color using the hue value or use the current hue based on changeHue
  if (HueChange) {
    EVERY_N_MILLISECONDS(20)
    {
      hue++;
    }
  }
  
  leds[sinBeat] = CHSV(hue, 255, 255);
  fadeToBlackBy(leds, NUM_LEDS, 25);
}

void sparkleDots()
{

  if (HueChange) {
    EVERY_N_MILLISECONDS(20)
    {
      hue++;
    }
  }
  else
  {
    hue = random8(0, 15);
  }

  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  
  leds[pos] += CHSV( hue, 255, 255);

  FastLED.delay(1000 / 60);
}
