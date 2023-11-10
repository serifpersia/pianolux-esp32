bool HueChange; // Boolean to control hue change

int snakePosition = random(MAX_NUM_LEDS);
int snakeLength = 4;
int foodPosition = -1;
bool startSnake = true;


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
  for (int i = 0; i < snakeLength; i++) {
    int brightness = map(i, 0, snakeLength - 1, 255, 0);
    CRGB color = CRGB::Red;
    color.nscale8(calculateBrightness(i, snakeLength));
    leds[(snakePosition - i + NUM_LEDS) % NUM_LEDS] = color;
  }
  leds[foodPosition] = CRGB::Orange;
}

uint8_t calculateBrightness(int index, int snakeLength) {
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

bool isFoodOnSnake(int position) {
  for (int i = 0; i < snakeLength; i++) {
    if ((snakePosition - i + NUM_LEDS) % NUM_LEDS == position) {
      return true;
    }
  }
  return false;
}
