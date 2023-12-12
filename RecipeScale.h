#ifndef RECIPESCALE_H
#define RECIPESCALE_H

#include <math.h>
#include <SPI.h>
#include <FastLED.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "HX711.h"

// size of ecipe items
#define SIZE 2

#include "RecipeItem.h"
#include "Recipe.h"

// start running load cell
#define LOADCELL_DOUT_PIN 12
#define LOADCELL_SCK_PIN 13
HX711 scale;

// start running the RFID board
#define SS_PIN 23
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

// start running the LED strip
#define LED_PIN 21
#define NUM_LEDS 36
CRGB leds[NUM_LEDS];

// start using the rotation click
int CLK = 15;
int DT = 33;
int SW = 27;

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

// colors for different seasonings
int COLOR_0[3] = {24, 252, 206};
int COLOR_1[3] = {255, 188, 0};

// all recipes array
Recipe recipes[SIZE];

RecipeItem recipeItem_salt("43 F1 90 1A", 0, COLOR_0);
RecipeItem recipeItem_vinegar("33 29 85 1A", 0, COLOR_1);
#endif
