#include "raylib.h"
#include <cstdlib>
#include <vector>
// #include <iostream>

// g++ main.cpp -o flappy -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

enum State { CHAR_STATE_UP, CHAR_STATE_MID, CHAR_STATE_DOWN };

typedef struct Obstacle {
  Vector2 location;
  float height;
  float width;
  Texture2D texture;
  int position;
} Obstacle;

class ObstacleManager {
private:
  Image mImage;
  Vector2 mDefaultLocation;
  float mSpeed;
  int mSpaceLength;
  std::vector<Obstacle> mObstacles;

public:
  ObstacleManager(Image image, float speed, Vector2 defaultLocation,
                  int spaceLength) {
    mImage = image;
    mSpeed = speed;
    mSpaceLength = spaceLength;
    mDefaultLocation = defaultLocation;
  }

  ~ObstacleManager() {}
  void setDefaultLocation(Vector2 defaultLocation) {
    mDefaultLocation = defaultLocation;
  }

  void setImage(Image image) { mImage = image; }
  void setSpeed(float speed) { mSpeed = speed; }
  void setSpaceLength(int spaceLength) { mSpaceLength = spaceLength; }
  float getSpeed() { return mSpeed; }
  int getSpaceLength() { return mSpaceLength; }
  std::vector<Obstacle> getObstacles() { return mObstacles; }

  Obstacle createObstacle(int position, float height) {
    Obstacle o;
    if (height < 0)
      height = (rand() % 10) * 30;
    o.height = height;
    o.width = (float)mImage.width;
    o.location.x = mDefaultLocation.x;
    o.location.y = mDefaultLocation.y - o.height;
    o.position = position;
    Image currentImage = ImageCopy(mImage);
    if (position <= 0) {
      o.location.y = -o.height + 100;
      ImageRotateCW(&currentImage);
      ImageRotateCW(&currentImage);
    } else {
      ImageCrop(&currentImage, (Rectangle){0, 0, o.width, o.height});
    }
    o.texture = LoadTextureFromImage(currentImage);
    UnloadImage(currentImage);
    mObstacles.push_back(o);
    return o;
  }

  void removeFirstObstacle() {
    if (mObstacles.size() > 0) {
      if (mObstacles.begin().base()->location.x < -100) {
        UnloadTexture(mObstacles.begin().base()->texture);
        mObstacles.erase(mObstacles.begin());
        UnloadTexture(mObstacles.begin().base()->texture);
        mObstacles.erase(mObstacles.begin());
      }
    }
  }

  void removeObstacleAtIndex(int index) {
    Obstacle obstacle = mObstacles.at(index);
    UnloadTexture(obstacle.texture);
    mObstacles.push_back(obstacle);
    UnloadTexture(mObstacles.begin().base()->texture);
    mObstacles.erase(mObstacles.begin());
  }

  void removeObstacle(Obstacle obstacle) {
    UnloadTexture(obstacle.texture);
    mObstacles.push_back(obstacle);
    UnloadTexture(mObstacles.begin().base()->texture);
    mObstacles.erase(mObstacles.begin());
  }

  void drawObstacles() {
    for (Obstacle &o : mObstacles) {
      o.location.x -= mSpeed;
      if (o.position <= 0)
        DrawTextureRec(o.texture,
                       (Rectangle){0, 0, o.width, (float)o.texture.height},
                       o.location, RAYWHITE);
      else
        DrawTextureRec(o.texture, (Rectangle){0, 0, o.width, o.height},
                       o.location, RAYWHITE);
    }
    removeFirstObstacle();
  }

  void generateObstacle(float currentPosition) {
    if ((int)currentPosition % mSpaceLength == 0) {
      Obstacle o = this->createObstacle(1, -1);
      this->createObstacle(0, o.height);
    }
  }
};

typedef struct Character {
  Vector2 location;
  float angle;
  float speed;
  Texture2D textureMid;
  Texture2D textureUp;
  Texture2D textureDown;
  Rectangle rectangle;
  int state;
} Character;

class CharacterController {
private:
  Character mCharacter;
  float mGraviationSpeed;
  float mElapsedTime;

  Vector2 gravitationModifier() {
    if (mCharacter.state == CHAR_STATE_UP)
      mElapsedTime = GetFrameTime();
    else
      mElapsedTime += GetFrameTime();
    mCharacter.state = CHAR_STATE_DOWN;
    mCharacter.speed = mGraviationSpeed * mElapsedTime;
    mCharacter.location.y += mCharacter.speed;
    mCharacter.angle < 50 ? mCharacter.angle += 2 : mCharacter.angle = 50;
    return mCharacter.location;
  }

public:
  CharacterController(Character character, float graviationSpeed) {
    mCharacter = character;
    mGraviationSpeed = graviationSpeed;
  }

  ~CharacterController() {}
  Vector2 moveUp() {
    if (mCharacter.state != CHAR_STATE_UP)
      mElapsedTime = GetFrameTime();
    else
      mElapsedTime += GetFrameTime();
    mCharacter.state = CHAR_STATE_UP;
    mCharacter.speed = -(mGraviationSpeed * mElapsedTime) - 5;
    mCharacter.location.y += mCharacter.speed;
    mCharacter.angle = -30;
    return mCharacter.location;
  }

  Vector2 drawCharacter() {
    Texture2D currentTexture;

    if (mCharacter.angle <= 12)
      currentTexture = mCharacter.textureUp;
    else if (mCharacter.angle < 10)
      currentTexture = mCharacter.textureDown;
    else
      currentTexture = mCharacter.textureMid;

    Rectangle currentRectangle =
        (Rectangle){mCharacter.location.x, mCharacter.location.y,
                    mCharacter.rectangle.width, mCharacter.rectangle.height};
    DrawTexturePro(currentTexture, mCharacter.rectangle, currentRectangle,
                   Vector2{0, 0}, mCharacter.angle, RAYWHITE);

    return mCharacter.location;
  }

  Vector2 inputHandler() {
    if (IsKeyPressed(KEY_SPACE) ||
        mCharacter.state == CHAR_STATE_UP && mElapsedTime < 0.15f) {
      return moveUp();
    }
    return gravitationModifier();
  }

  Vector2 onUpdate() {
    inputHandler();
    drawCharacter();
    return mCharacter.location;
  }
};

int main() {
  int screenWidth = 800;
  int screenHeight = 800;

  bool floor_collision, pipe_collision = false;

  InitWindow(screenWidth, screenHeight, "Raylib - Flappy Bird");

  Image backgroundImage = LoadImage("assets/sprites/background-day.png");
  ImageResize(&backgroundImage, screenWidth, screenHeight);
  Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
  UnloadImage(backgroundImage);

  Image obstacleImage = LoadImage("assets/sprites/pipe-green.png");

  Image baseImage = LoadImage("assets/sprites/base.png");
  Texture2D baseTexture = LoadTextureFromImage(baseImage);
  UnloadImage(baseImage);

  Image characterImage = LoadImage("assets/sprites/bluebird-midflap.png");
  ImageResize(&characterImage, (int)(characterImage.width * 1.5),
              (int)(characterImage.height * 1.5));
  Texture2D characterMidTexture = LoadTextureFromImage(characterImage);

  characterImage = LoadImage("assets/sprites/bluebird-upflap.png");
  ImageResize(&characterImage, (int)(characterImage.width * 1.5),
              (int)(characterImage.height * 1.5));
  Texture2D characterUpTexture = LoadTextureFromImage(characterImage);
  UnloadImage(characterImage);

  characterImage = LoadImage("assets/sprites/bluebird-downflap.png");
  ImageResize(&characterImage, (int)(characterImage.width * 1.5),
              (int)(characterImage.height * 1.5));
  Texture2D characterDownTexture = LoadTextureFromImage(characterImage);
  UnloadImage(characterImage);

  SetTargetFPS(120);

  float speed = 150, characterSpeed = 18;

  Character character;
  character.location = Vector2{60, (float)screenHeight / 2};
  character.state = CHAR_STATE_MID;
  character.textureUp = characterUpTexture;
  character.textureMid = characterMidTexture;
  character.textureDown = characterDownTexture;
  character.angle = 0;
  character.rectangle = Rectangle{0, 0, (float)characterUpTexture.width,
                                  (float)characterUpTexture.height};
  CharacterController characterController(character, characterSpeed);

  float currentBackgroundPosition = 0;
  float currentBasePosition = 0;
  Vector2 characterPosition = Vector2{60, (float)screenHeight / 2};

  ObstacleManager obstacleManager(
      obstacleImage, speed,
      (Vector2){(float)screenWidth, (float)(screenHeight - baseTexture.height)},
      200);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    floor_collision = characterPosition.y + (characterImage.height * 1.5) >=
                              screenWidth - baseImage.height
                          ? true
                          : false;

    float frameSpeed = GetFrameTime() * speed;
    obstacleManager.setSpeed(frameSpeed);

    DrawTextureRec(backgroundTexture,
                   (Rectangle){currentBackgroundPosition, 0, (float)screenWidth,
                               (float)screenHeight},
                   (Vector2){0, 0}, RAYWHITE);
    DrawTextureRec(baseTexture,
                   (Rectangle){currentBasePosition, 0, (float)screenWidth,
                               (float)baseTexture.height},
                   (Vector2){0, (float)(screenHeight - baseTexture.height)},
                   RAYWHITE);

    obstacleManager.generateObstacle(currentBasePosition);
    obstacleManager.drawObstacles();

    characterPosition = characterController.onUpdate();
    currentBackgroundPosition += (frameSpeed);
    currentBasePosition += frameSpeed;

    if (currentBackgroundPosition >= screenWidth)
      currentBackgroundPosition -= screenWidth;
    if (currentBasePosition >= (float)screenWidth / 2)
      currentBasePosition -= (float)screenWidth / 2;

    DrawFPS(10, 10);
    if (floor_collision) {
      DrawText("Collision!", 10, 30, 20, RED);
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
