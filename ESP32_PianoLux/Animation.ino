bool HueChange; // Boolean to control hue change

uint8_t snakePosition = random(MAX_NUM_LEDS);
uint8_t snakeLength = 4;
uint8_t foodPosition = -1;
bool startSnake = true;

void Animatons(uint8_t selectedAnimation) {
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

  for (uint8_t i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

void sineWave() {
  uint16_t sinBeat = beatsin16(15, 0, NUM_LEDS - 1, 0, 0);

  // Set the LED color using the hue value or use the current hue based on changeHue
  if (HueChange) {
    EVERY_N_MILLISECONDS(20) {
      hue++;
    }
  }

  leds[sinBeat] = CHSV(hue, 255, 255);
  fadeToBlackBy(leds, NUM_LEDS, 25);
}

void sparkleDots() {
  uint8_t sparkleHue;

  if (HueChange) {
    EVERY_N_MILLISECONDS(20) {
      sparkleHue++;
    }
  } else {
    sparkleHue = random8(0, 15);
  }

  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  uint8_t pos = random16(NUM_LEDS);

  leds[pos] += CHSV(sparkleHue, 255, 255);

  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void Snake() {
  if (startSnake) {
    spawnFood();
    startSnake = false;
  }

  snakeAnimation();
  updateLEDs();
}

void updateLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t i = 0; i < snakeLength; i++) {
    uint8_t brightness = map(i, 0, snakeLength - 1, 255, 0);
    CRGB color = CRGB::Red;
    color.nscale8(calculateBrightness(i, snakeLength));
    leds[(snakePosition - i + NUM_LEDS) % NUM_LEDS] = color;
  }
  leds[foodPosition] = CRGB::Orange;
}

uint8_t calculateBrightness(uint8_t index, uint8_t snakeLength) {
  // Calculate the brightness gradient based on the position of the LED in the snake
  // Here, we use a simple linear gradient from full brightness to off (0) over the length of the snake
  return map(index, 0, snakeLength - 1, 255, 0);
}

void snakeAnimation() {
  snakePosition = (snakePosition + 1) % NUM_LEDS;
  if (snakePosition == foodPosition) {
    snakeLength += 4;
    spawnFood();
  }
}

void spawnFood() {
  if (snakeLength >= NUM_LEDS) {
    snakeLength = 4;
    startSnake = true;
  } else {
    do {
      foodPosition = random(NUM_LEDS);
    } while (isFoodOnSnake(foodPosition));
  }
}

bool isFoodOnSnake(uint8_t position) {
  for (uint8_t i = 0; i < snakeLength; i++) {
    if ((snakePosition - i + NUM_LEDS) % NUM_LEDS == position) {
      return true;
    }
  }
  return false;
}
