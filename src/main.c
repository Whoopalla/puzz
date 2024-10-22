#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "raylib.h"

#define MAX_PATH 256
#define WINDOW_W 1200
#define WINDOW_H 1000
#define DROP_IMAGE_HERE "Drop image here"

#define DEBUG true

typedef enum { Solving, WaitingForDrop } State;

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

void init_new_puzz(Texture t, size_t rects_per_row, Rectangle *rects,
                   Rectangle *locations) {
  int locations_indexes[rects_per_row * rects_per_row];
  float rec_w = (float)(t.width) / rects_per_row;
  float rec_h = (float)(t.height) / rects_per_row;

  arr_from_range(0, rects_per_row * rects_per_row, locations_indexes);
  rearrange_rand(locations_indexes, rects_per_row * rects_per_row);

  for (size_t y = 0; y < rects_per_row; y++) {
    for (size_t x = 0; x < rects_per_row; x++) {
      rects[y * rects_per_row + x] =
          (Rectangle){x * rec_w, y * rec_h, rec_w, rec_h};
      printf("xRec: %f yRec: %f\n", rects[y * rects_per_row + x].x,
             rects[y * rects_per_row + x].y);
    }
  }

  printf("Locations: \n");
  for (size_t i = 0; i < rects_per_row * rects_per_row; i++) {
    printf("%d\n", locations_indexes[i]);
  }

  for (size_t i = 0; i < rects_per_row * rects_per_row; i++) {
    locations[i].x = rects[locations_indexes[i]].x + WINDOW_W / 2 - t.width / 2;
    locations[i].y =
        rects[locations_indexes[i]].y + WINDOW_H / 2 - t.height / 2;
    locations[i].width = rects[locations_indexes[i]].width;
    locations[i].height = rects[locations_indexes[i]].height;
  }

  printf("\trearranged: \n");
  for (size_t y = 0; y < rects_per_row; y++) {
    for (size_t x = 0; x < rects_per_row; x++) {
      printf("xRec: %f yRec: %f\n", locations[y * rects_per_row + x].x,
             locations[y * rects_per_row + x].y);
    }
  }
}

int main(void) {
  InitWindow(WINDOW_W, WINDOW_H, "puzz");

  Image image;
  Texture texture;
  char *image_path = calloc(MAX_PATH, sizeof(char));
  State state = WaitingForDrop;
  size_t rects_per_row = 3;
  size_t rects_count = rects_per_row * rects_per_row;

  Vector2 mouse_pos;
  Vector2 drag_start;
  Vector2 drag_point;
  Vector2 prev_pos;
  bool files_allow_drop = true;
  int selected_piece_index = -1;
  float rec_w;
  float rec_h;
  Rectangle rects[rects_count];
  Rectangle locations[rects_count];

  if (DEBUG) {
    image = LoadImage("toto.png");
    if (!IsImageReady(image)) {
      fprintf(stderr, "ERROR: Image was not loaded.\n");
      exit(-1);
    }

    texture = LoadTextureFromImage(image);
    if (!IsTextureReady(texture)) {
      fprintf(stderr, "Texture was not loaded.\n");
      exit(-1);
    }
    rec_w = (float)(texture.width) / rects_per_row;
    rec_h = (float)(texture.height) / rects_per_row;
    init_new_puzz(texture, rects_per_row, rects, locations);
    state = Solving;
  }

  while (!WindowShouldClose()) {
    if (state == WaitingForDrop) {
      // Check dropped files
      if (files_allow_drop && IsFileDropped()) {
        FilePathList dropped_files = LoadDroppedFiles();
        assert(dropped_files.count == 1);
        if (!IsFileExtension(dropped_files.paths[0], ".png")) {
          fprintf(stderr, "ERROR: not supported image format: %s\n",
                  GetFileExtension(dropped_files.paths[0]));
          UnloadDroppedFiles(dropped_files);
          continue;
        }
        strcpy(image_path, dropped_files.paths[0]);
        image = LoadImage(image_path);
        if (!IsImageReady(image)) {
          fprintf(stderr, "ERROR: Image was not loaded.\n");
          exit(-1);
        }

        texture = LoadTextureFromImage(image);
        if (!IsTextureReady(texture)) {
          fprintf(stderr, "Texture was not loaded.\n");
          exit(-1);
        }
        rec_w = (float)(texture.width) / rects_per_row;
        rec_h = (float)(texture.height) / rects_per_row;
        init_new_puzz(texture, rects_per_row, rects, locations);
        state = Solving;
        UnloadDroppedFiles(dropped_files);
      }
      BeginDrawing();
      ClearBackground(DARKBLUE);

      DrawText(DROP_IMAGE_HERE,
               WINDOW_W / 2 - MeasureText(DROP_IMAGE_HERE, 40) / 2,
               WINDOW_H / 2, 40, SKYBLUE);
      EndDrawing();
      continue;
    }

    // Check if solved
    for (size_t y = 0; y < rects_per_row; y++) {
      for (size_t x = 0; x < rects_per_row; x++) {
        if (rects[y * rects_per_row + x].x !=
                locations[y * rects_per_row + x].x ||
            rects[y * rects_per_row + x].y !=
                locations[y * rects_per_row + x].y) {
          goto notSolved;
        }
      }
    }
    printf("Solved!\n");
    UnloadImage(image);
    UnloadTexture(texture);
    state = WaitingForDrop;
  notSolved:

    mouse_pos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (size_t i = 0; i < rects_count; i++) {
        if (CheckCollisionPointRec(mouse_pos, locations[i])) {
          selected_piece_index = i;
          drag_point.x = mouse_pos.x - locations[i].x;
          drag_point.y = mouse_pos.y - locations[i].y;
          drag_start = mouse_pos;
          prev_pos = (Vector2){locations[selected_piece_index].x,
                               locations[selected_piece_index].y};

          break;
        } else {
          selected_piece_index = -1;
        }
      }
    }

    if (selected_piece_index != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      locations[selected_piece_index].x = mouse_pos.x - drag_point.x;
      locations[selected_piece_index].y = mouse_pos.y - drag_point.y;
    }

    // Snapping
    if (selected_piece_index != -1 &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      printf("Snap Mouse pos: x: %f y: %f\n", mouse_pos.x, mouse_pos.y);
      size_t i = 0;
      while (i < rects_count) {
        if (!(selected_piece_index == (int)i) &&
            CheckCollisionPointRec(mouse_pos, locations[i])) {
          break;
        }
        i++;
      }
      if (i < rects_count) {
        locations[selected_piece_index].x = locations[i].x;
        locations[selected_piece_index].y = locations[i].y;
        locations[i].x = prev_pos.x;
        locations[i].y = prev_pos.y;
      } else {
        locations[selected_piece_index].x = prev_pos.x;
        locations[selected_piece_index].y = prev_pos.y;
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t i = 0; i < rects_count; i++) {
      // DrawTextureRec(texture, rects[i],
      //                (Vector2){locations[i].x, locations[i].y},
      //                WHITE);
      DrawTexturePro(texture, rects[i], locations[i], (Vector2){0, 0}, 0,
                     WHITE);
    }
    DrawTexturePro(texture, rects[selected_piece_index],
                   locations[selected_piece_index], (Vector2){0, 0}, 0, WHITE);
    if (selected_piece_index != -1) {
      DrawRectangleLinesEx(locations[selected_piece_index], 5.0f, GREEN);
    }

    EndDrawing();
  }
  UnloadImage(image);
  free(image_path);
  return 0;
}
