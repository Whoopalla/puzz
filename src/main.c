#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

int arr_from_range(size_t from, size_t to, int *d) {
  if (from >= to)
    return -1;
  size_t start = from;
  for (size_t i = 0; i < to - from; i++) {
    d[i] = start++;
  }
  return 0;
}

int rearrange_rand(int *s, size_t n) {
  size_t index;
  int t;
  srand(time(NULL));
  for (size_t i = 0; i < n; i++) {
    index = rand() % n;
    t = (s[i]);
    s[i] = s[index];
    s[index] = t;
  }
  return 0;
}

int main(void) {
  InitWindow(1200, 1000, "puzz");
  Image image = LoadImage("toto.png");
  if (!IsImageReady(image)) {
    fprintf(stderr, "Image was not loaded.\n");
    exit(-1);
  }

  Texture texture = LoadTextureFromImage(image);
  if (!IsTextureReady(texture)) {
    fprintf(stderr, "Texture was not loaded.\n");
    exit(-1);
  }

  size_t rectCount = 3;

  Rectangle rects[rectCount * rectCount];
  Rectangle locations[rectCount * rectCount];
  int locations_indexes[rectCount * rectCount];
  float rec_w = (float)(texture.width) / rectCount;
  float rec_h = (float)(texture.height) / rectCount;
  int selected_pice_index = -1;

  arr_from_range(0, rectCount * rectCount, locations_indexes);
  rearrange_rand(locations_indexes, rectCount * rectCount);

  for (size_t y = 0; y < rectCount; y++) {
    for (size_t x = 0; x < rectCount; x++) {
      rects[y * rectCount + x] =
          (Rectangle){x * rec_w, y * rec_h, rec_w, rec_h};
      printf("xRec: %f yRec: %f\n", rects[y * rectCount + x].x,
             rects[y * rectCount + x].y);
    }
  }

  printf("Locations: \n");
  for (size_t i = 0; i < rectCount * rectCount; i++) {
    printf("%d\n", locations_indexes[i]);
  }

  for (size_t i = 0; i < rectCount * rectCount; i++) {
    locations[i].x = rects[locations_indexes[i]].x;
    locations[i].y = rects[locations_indexes[i]].y;
    locations[i].width = rects[locations_indexes[i]].width;
    locations[i].height = rects[locations_indexes[i]].height;
  }

  printf("\trearranged: \n");
  for (size_t y = 0; y < rectCount; y++) {
    for (size_t x = 0; x < rectCount; x++) {
      printf("xRec: %f yRec: %f\n", locations[y * rectCount + x].x,
             locations[y * rectCount + x].y);
    }
  }

  Vector2 mouse_pos;
  Vector2 drag_start;
  Vector2 drag_point;
  while (!WindowShouldClose()) {
    // Check if solved
    for (size_t y = 0; y < rectCount; y++) {
      for (size_t x = 0; x < rectCount; x++) {
        if (rects[y * rectCount + x].x != locations[y * rectCount + x].x ||
            rects[y * rectCount + x].y != locations[y * rectCount + x].y) {

          printf("rects %f %f\n", rects[y * rectCount + x].x,
                 rects[y * rectCount + x].y);
          printf("locations %f %f\n", locations[y * rectCount + x].x,
                 locations[y * rectCount + x].y);
          goto notSolved;
        }
      }
    }
    printf("Solved!\n");
    exit(0);
  notSolved:

    mouse_pos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (size_t i = 0; i < rectCount * rectCount; i++) {
        if (CheckCollisionPointRec(mouse_pos, locations[i])) {
          selected_pice_index = i;
          drag_point.x = mouse_pos.x - locations[i].x;
          drag_point.y = mouse_pos.y - locations[i].y;
          drag_start = mouse_pos;

          break;
        } else {
          selected_pice_index = -1;
        }
      }
    }

    if (selected_pice_index != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      locations[selected_pice_index].x = mouse_pos.x - drag_point.x;
      locations[selected_pice_index].y = mouse_pos.y - drag_point.y;
    }

    // TODO: swap two pieces after snapping
    // Snapping
    if (selected_pice_index != -1 && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      printf("Snap Mouse pos: x: %f y: %f\n", mouse_pos.x, mouse_pos.y);
      int nx = (int)(mouse_pos.x / rec_w);
      int ny = (int)(mouse_pos.y / rec_h);
      printf("Snap x: %d y: %d\n", nx, ny);

      if ((size_t)nx >= rectCount || (size_t)ny >= rectCount) {
        printf("Out of bounds snap!\n");
        nx = (int)drag_start.x / rec_w;
        ny = (int)drag_start.y / rec_h;
        printf("Snap x: %d y: %d\n", nx, ny);
        locations[selected_pice_index].x = nx * rec_w;
        locations[selected_pice_index].y = ny * rec_h;
      } else {
        // Swap
        for(size_t i = 0; i < rectCount*rectCount; i++) {
          if (CheckCollisionPointRec(mouse_pos, locations[i]) && (int)i != selected_pice_index) {
            locations[i].x = (int)(drag_start.x / rec_w)*rec_w;
            locations[i].y = (int)(drag_start.y / rec_h)*rec_h;
            break;
          }
        }
        locations[selected_pice_index].x = nx * rec_w;
        locations[selected_pice_index].y = ny * rec_h;
      }
      printf("Snaped to x: %f y: %f\n", nx * rec_w, ny * rec_h);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t i = 0; i < rectCount * rectCount; i++) {
      // DrawTextureRec(texture, rects[i],
      //                (Vector2){locations[i].x, locations[i].y},
      //                WHITE);
      DrawTexturePro(texture, rects[i], locations[i], (Vector2){0, 0}, 0,
                     WHITE);
    }
    DrawTexturePro(texture, rects[selected_pice_index],
                   locations[selected_pice_index], (Vector2){0, 0}, 0, WHITE);
    if (selected_pice_index != -1) {
      DrawRectangleLinesEx(locations[selected_pice_index], 5.0f, GREEN);
    }

    EndDrawing();
  }
  UnloadImage(image);
  return 0;
}
