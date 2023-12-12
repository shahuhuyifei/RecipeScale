#include "RecipeScale.h"

void setup()
{
  Serial.begin(9600);

  // initialize the scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(217);
  // scale.set_scale(-224);
  scale.tare(); // reset the scale to 0

  // initialize the RFID board
  SPI.begin();
  mfrc522.PCD_Init();

  // initialize LED strip
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();

  // initialize encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  // create recipe items
  RecipeItem recipeItem_0("43 F1 90 1A", 10, COLOR_0);
  RecipeItem recipeItem_1("33 29 85 1A", 200, COLOR_1);

  // create one recipe and store in the array of recipes
  RecipeItem recipeItem_list[] = {recipeItem_0, recipeItem_1};
  Recipe recipe_0("1C F2 2A 49", recipeItem_list);
  recipes[0] = recipe_0;
}

void loop()
{
  // default mode
  // weighting
  int temp_weight = (int)(floor(scale.get_units(10)));
  Serial.println(temp_weight);
  showByNum(temp_weight, 100, 100, 100);
  delay(50);

  // touch to use the timer
  if (touchRead(14) < 40)
  {
    breath(NUM_LEDS);
    useTimer();
  }

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // convert uid to a string
  String card_uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    card_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    card_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  card_uid.toUpperCase();
  Serial.println(card_uid.substring(1));

  // the variable that decides which mode the scale is in
  int state = 0;

  // enter using recipe mode if the recipe array have the tapped recipe card
  int current_recipe = 0;
  for (int i = 0; i < SIZE; i++)
  {
    if (recipes[i].recipeID == card_uid.substring(1))
    {
      current_recipe = i;
      state = 1;
    }
  }

  // Enter the recipe recording mode when the card is not in the list
  if (state == 0 && card_uid.substring(1) != "")
  {
    if (card_uid.substring(1) != recipeItem_salt.seasoningID && card_uid.substring(1) != recipeItem_vinegar.seasoningID)
    {
      state = 2;
      breath(NUM_LEDS);
    }
  }

  switch (state)
  {
  case 0:
  {
    // do nothing when no card are scanned
    break;
  }
  case 1:
  {
    // reading recipe mode
    int total_amount = 0;
    for (int i = 0; i < SIZE; i++)
    {

      if (recipes[current_recipe].recipeItem_list[i].amount == 0)
      {
        continue;
      }
      // Show the amount of LED lights by weight
      Serial.println(recipes[current_recipe].recipeItem_list[i].amount);
      showByNum(recipes[current_recipe].recipeItem_list[i].amount,
                recipes[current_recipe].recipeItem_list[i].color[0],
                recipes[current_recipe].recipeItem_list[i].color[1],
                recipes[current_recipe].recipeItem_list[i].color[2]);
      int seasoning_weight = recipes[current_recipe].recipeItem_list[i].amount;
      total_amount += seasoning_weight;
      // Decrease the amount of LED lights
      while (true)
      {
        int t_weight = (int)(floor(scale.get_units(10)));
        if (t_weight < total_amount)
        {
          Serial.println(t_weight);
          showByNum(total_amount - t_weight,
                    recipes[current_recipe].recipeItem_list[i].color[0],
                    recipes[current_recipe].recipeItem_list[i].color[1],
                    recipes[current_recipe].recipeItem_list[i].color[2]);
        }
        else
        {
          break;
        }
      }
      breath(NUM_LEDS);
    }
    break;
  }
  case 2:
  {
    // recording recipe mode
    while (true)
    {
      // Look for new cards
      if (!mfrc522.PICC_IsNewCardPresent())
      {
        continue;
      }
      // Select one of the cards
      if (!mfrc522.PICC_ReadCardSerial())
      {
        continue;
      }

      // convert uid to a string
      String seasoning_uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        seasoning_uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        seasoning_uid.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      seasoning_uid.toUpperCase();
      Serial.println(seasoning_uid.substring(1));

      // create recipe items
      if (seasoning_uid.substring(1) == recipeItem_salt.seasoningID)
      {
        breath(NUM_LEDS);
        Serial.println("in create recipe");

        recipeItem_salt.amount = getAmount(COLOR_0[0], COLOR_0[1], COLOR_0[2]);
        Serial.println(recipeItem_salt.amount);
        int breathIndex = 0;
        breath(lightMap(recipeItem_salt.amount));
      }
      if (seasoning_uid.substring(1) == recipeItem_vinegar.seasoningID)
      {
        breath(NUM_LEDS);
        Serial.println("in create recipe");

        recipeItem_vinegar.amount = getAmount(COLOR_1[0], COLOR_1[1], COLOR_1[2]);
        Serial.println(recipeItem_vinegar.amount);
        breath(lightMap(recipeItem_vinegar.amount));
      }
      if (seasoning_uid == card_uid)
      {
        // create new recipe and store in the array of recipes
        RecipeItem recipeItem_list[] = {recipeItem_salt, recipeItem_vinegar};
        Recipe recipe_1(card_uid.substring(1), recipeItem_list);
        recipes[1] = recipe_1;
        breath(NUM_LEDS);
        return;
      }
    }
    Serial.println("1qwer");
    break;
  }
  }
}

// Get the amount of seasoning of a newly created recipe
int getAmount(int R, int G, int B)
{
  counter = 0;
  while (true)
  {
    currentStateCLK = digitalRead(CLK);
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
    {
      // Increase amount
      if (digitalRead(DT) != currentStateCLK)
      {
        counter++;
        currentDir = "clockwise";
      }
      // Decrease amount
      else
      {
        counter--;
        currentDir = "counter-clockwise";
      }
      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Number of light: ");
      Serial.println(counter);
      if (counter <= 0)
      {
        counter = 0;
      }
      else if (counter >= 300)
      {
        counter = 300;
      }
      showByNum(counter, R, G, B);
    }
    lastStateCLK = currentStateCLK;
    int btnState = digitalRead(SW);
    lastButtonPress = millis();
    // Button is pressed when signal is LOW
    if (btnState == LOW)
    {
      Serial.print("press btn");
      if (millis() - lastButtonPress > 50)
      {
        Serial.println("Button pressed!");
        showByNum(400, 0, 0, 0);
        return counter;
      }
    }
  }
}

// allows user to use the timer
void useTimer()
{
  counter = 0;
  while (true)
  {
    currentStateCLK = digitalRead(CLK);
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
    {
      // Increase time
      if (digitalRead(DT) != currentStateCLK)
      {
        counter++;
        currentDir = "clockwise";
      }
      // Decrease time
      else
      {
        counter--;
        currentDir = "counter-clockwise";
      }
      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Number of light: ");
      Serial.println(counter);
      if (counter <= 0)
      {
        counter = 0;
      }
      else if (counter >= 300)
      {
        counter = 300;
      }
      showByNum(counter, 255, 247, 0);
    }
    lastStateCLK = currentStateCLK;
    int btnState = digitalRead(SW);
    lastButtonPress = millis();
    // Button is pressed when signal is LOW
    // Indicate the timer is setted
    if (btnState == LOW)
    {
      Serial.print("press btn");
      if (millis() - lastButtonPress > 50)
      {
        Serial.println("Button pressed!");
        startTimer(counter);
        counter = 0;
        break;
      }
    }
  }
}

// timer function
void startTimer(int t)
{
  Serial.println("enter the function!");

  Serial.println(t);

  while (t > 0)
  {
    Serial.println("enter while!");
    showByNum(t, 0, 0, 153);
    t--;
    delay(1000);
  }
  if (t == 0)
  {
    breath(NUM_LEDS);
    // FastLED.setBrightness(NORMAL_BRIGHTNESS);
  }
}
void breath(int ledNum)
{
  int breathIndex = 0;
  while (breathIndex < 2)
  {
    EVERY_N_SECONDS(1)
    {
      breathIndex++;
    }
    FastLED.clear();
    fill_solid(leds, ledNum, CRGB::White);
    float breath = (exp(sin(millis() / 1000.0 * 2 * PI)) - 0.36787944) * 30.0;
    FastLED.setBrightness(breath);
    FastLED.show();
  }
  FastLED.setBrightness(255);
  delay(500);
  showByNum(400, 0, 0, 0);
}
// show LEDs in a specific amount and color
void showByNum(int num, int R, int G, int B)
{
  // look up how many LED need to light up
  int lightNum = lightMap(num);
  // light up
  for (int i = 0; i < lightNum; i++)
  {
    leds[i] = CRGB(R, G, B);
  }
  FastLED.show();
  for (int j = NUM_LEDS - lightNum; j < NUM_LEDS; j++)
  {
    leds[j] = CRGB(0, 0, 0);
  }
  FastLED.clear();
}

// map real amount to LED lights
int lightMap(int before)
{
  int after = 0;
  if (before < 0)
  {
    after = 0;
  }
  else if (before < 10)
  {
    after = before;
  }
  else if (before < 31)
  {
    after = 7 + (int)before / 5;
  }
  else if (before < 201)
  {
    after = 11 + (int)before / 10;
  }
  else if (before < 301)
  {
    after = 21 + (int)before / 20;
  }
  else
  {
    after = 36;
  }
  return after;
}
