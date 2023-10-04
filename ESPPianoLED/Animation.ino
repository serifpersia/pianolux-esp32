
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
