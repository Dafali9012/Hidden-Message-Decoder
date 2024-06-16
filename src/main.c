#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "raylib.h"
#include "raymath.h"

#include "main.h"

#define TEXT_COLOR (Color){137,4,64,255}

char *messagePath;
Image messageImage;
Texture2D messageTexture;
int vertexCount = 0, rows = 8, cols = 12, selectedVertex = -1;;
Vector2 vertices[4];
bool shouldDraw = true, shouldUpdateBinary = false, vertexIsSelected = false;
RenderTexture2D renderTexture;
char* binary;
char* translatedMessage;

int main() {
  SetTraceLogLevel(4);
  init();

  while(!WindowShouldClose()) {
    update();
    if(shouldDraw) draw(renderTexture);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureRec(renderTexture.texture, (Rectangle){0,0,renderTexture.texture.width, -renderTexture.texture.height}, (Vector2){0,0}, WHITE);
    EndDrawing();
  }

  free(binary);
  free(translatedMessage);

  CloseWindow();
  return 0;
}

void init() {
  messagePath = ".\\message.png";
  messageImage = LoadImage(messagePath);
  ImageColorGrayscale(&messageImage);
  InitWindow(messageImage.width, messageImage.height, "Decode Hidden Message");
  SetTargetFPS(30);
  messageTexture = LoadTextureFromImage(messageImage);
  renderTexture = LoadRenderTexture(messageImage.width, messageImage.height);
  binary = malloc(rows * cols * sizeof(char));
  translatedMessage = malloc(((rows * cols) / 6 + 1) * sizeof(char));
  translatedMessage[0] = '\0';
}

void update() {
  if(IsKeyPressed(KEY_UP)) {
    rows++;
    shouldDraw = true;
    binary = realloc(binary, rows * cols * sizeof(char));
    shouldUpdateBinary = true;
  }
  if(IsKeyPressed(KEY_DOWN)) {
    if(rows > 1) {
      rows--;
      shouldDraw = true;
      binary = realloc(binary, rows * cols * sizeof(char));
      shouldUpdateBinary = true;
    }
  }
  if(IsKeyPressed(KEY_RIGHT)) {
    cols++;
    shouldDraw = true;
    binary = realloc(binary, rows * cols * sizeof(char));
    shouldUpdateBinary = true;
  }
  if(IsKeyPressed(KEY_LEFT)) {
    if(cols > 1) {
      cols--;
      shouldDraw = true;
      binary = realloc(binary, rows * cols * sizeof(char));
      shouldUpdateBinary = true;
    }
  }

  if(IsKeyPressed(KEY_T)) {
    if(vertexCount == 4) {
      printf("translating...\n");
      int charColumns = floor(cols / 6);
      translatedMessage = realloc(translatedMessage, (rows * charColumns + 1) * sizeof(char));
      for(int c = 0; c < charColumns; c++) {
        int xBegin = c * 6 - 1 + 6;
        for(int r = 0; r < rows; r++) {
          char character;
          int binaryNumber = 65;
          for(int i = 0; i < 6; i++) {
            int bit = binary[r * (charColumns * 6) + (xBegin - i)] - '0';
            binaryNumber += bit * pow(2, i);
          }
          character = (char)(binaryNumber);
          translatedMessage[c * rows + r] = character;
        }
      }
      translatedMessage[(rows * charColumns + 1) * sizeof(char)] = '\0';
      shouldDraw = true;
    }
  }

  // track held key of number 1 through 4
  selectedVertex = -1;
  if(IsKeyDown(49)) selectedVertex = 0;
  if(IsKeyDown(50)) selectedVertex = 1;
  if(IsKeyDown(51)) selectedVertex = 2;
  if(IsKeyDown(52)) selectedVertex = 3;

  if(selectedVertex != -1 && !vertexIsSelected) {
    vertexIsSelected = true;
    shouldDraw = true;
  } else if(selectedVertex == -1 && vertexIsSelected) {
    vertexIsSelected = false;
    shouldDraw = true;
  }

  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if(selectedVertex != -1) { // move existing vertices
      if(vertexCount > selectedVertex) {
        vertices[selectedVertex] = (Vector2){GetMousePosition().x, GetMousePosition().y};
        shouldDraw = true;
        if(vertexCount == 4) {
          shouldUpdateBinary = true;
        }
      }
    } else { // add new vertices (max 4)
      if(vertexCount < 4) {
        vertices[vertexCount] = (Vector2){GetMousePosition().x, GetMousePosition().y};
        vertexCount++;
        shouldDraw = true;
        if(vertexCount == 4) {
          shouldUpdateBinary = true;
        }
      }
    }
  }

  if(shouldUpdateBinary) {
    printf("updating binary\n");
    for(int y = 0; y < rows; y++) {
      for(int x = 0; x < cols; x++) {
        Vector2 p1 = Vector2Lerp(vertices[0], vertices[1], (1.0f / cols) * x + (1.0f / cols) * 0.5f);
        Vector2 p2 = Vector2Lerp(vertices[3], vertices[2], (1.0f / cols) * x + (1.0f / cols) * 0.5f);
        Vector2 p3 = Vector2Lerp(p1, p2, (1.0f / rows) * y + (1.0f / rows) * 0.5f);

        float averageValue = 0;

        for(int i = 0; i < 9; i++) {
          for(int j = 0; j < 9; j++) {
            Color pixelColor = GetImageColor(messageImage, p3.x + j - 4, p3.y + i - 4);
            averageValue += ColorToHSV(pixelColor).z;
          }
        }

        averageValue /= 81;

        char bitValue = averageValue < .4f ? '1' : '0';
        binary[y * cols + x] = bitValue;
      }
    }
    shouldUpdateBinary = false;
  }
}

void draw(RenderTexture2D target) {
  printf("re-drawing the screen\n");

  BeginTextureMode(renderTexture);
  ClearBackground(BLACK);
  DrawTexture(messageTexture, 0, 0, WHITE);

  char rowsText[10];
  char colsText[10];
  rowsText[0] = '\0';
  colsText[0] = '\0';
  sprintf(rowsText, "rows: %d", rows);
  sprintf(colsText, "cols: %d", cols);
  DrawText(rowsText, 10, 0, 48, TEXT_COLOR);
  DrawText(colsText, 10, 48, 48, TEXT_COLOR);

  DrawText(translatedMessage, 10, messageImage.height - 48, 38, TEXT_COLOR);

  // draw vertices
  for(int i = 0; i < vertexCount; i++) {
    Color vertexColor = (Color){220,20,60,200};
    if(selectedVertex != -1 && i == selectedVertex) {
      vertexColor = (Color){255,174,66,200};
      vertexIsSelected = true;
    }
    int fontSize = 24;
    char vertexIndex[2];
    sprintf(vertexIndex, "%d", i + 1);
    DrawCircleV(vertices[i], fontSize * 0.75f, vertexColor);
    DrawText(vertexIndex, vertices[i].x - fontSize/4, vertices[i].y - fontSize/2, fontSize, WHITE);
  }
  // draw grid points
  if(vertexCount == 4) {
    for(int y = 0; y < rows; y++) {
      for(int x = 0; x < cols; x++) {
        Vector2 p1 = Vector2Lerp(vertices[0], vertices[1], (1.0f / cols) * x + (1.0f / cols) * 0.5f);
        Vector2 p2 = Vector2Lerp(vertices[3], vertices[2], (1.0f / cols) * x + (1.0f / cols) * 0.5f);
        Vector2 p3 = Vector2Lerp(p1, p2, (1.0f / rows) * y + (1.0f / rows) * 0.5f);

        char binaryValue = binary[y * cols + x];
        DrawCircleV(p3, 5, binaryValue == '0' ? BLACK : WHITE);
      }
    }
  }
  EndTextureMode();
  shouldDraw = false;
}
