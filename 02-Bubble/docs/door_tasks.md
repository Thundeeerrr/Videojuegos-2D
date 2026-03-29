# Task List: Implementación de Puertas

## Contexto
- Juego 2D en C++ con OpenGL y GLM
- Tilemap cargado desde `.txt`
- PNG de puertas: `doors.png` (16x48px, frame abierta en y=0..24, frame cerrada en y=24..48)
- Ancla de la puerta: tile inferior (marcador **numérico** en el `.txt`)
- Tecla `D` para abrir/cerrar las puertas (debug/test)
- TILE_SIZE = 16px

---

## Task 1 — Preparar el TXT del mapa

- [ ] Elegir un ID numérico para el marcador de puerta que no exista en la tilesheet (recomendado: `999`, porque en niveles actuales los IDs visibles son mucho menores)
- [ ] En el `.txt` del nivel, reemplazar el tile inferior de cada puerta por el marcador elegido (`999`)
- [ ] Asegurarse de que el tile superior (8px de techo) queda con su tile normal de techo/decoración

---

## Task 2 — Crear la clase Door

Crear los ficheros `Door.h` y `Door.cpp`.

**Door.h:**
- [ ] Atributo `glm::vec2 worldPos` — posición top-left del sprite en pixels
- [ ] Atributo `glm::ivec2 tilePos` — posición del tile ancla en la grilla
- [ ] Atributo `bool isOpen` — estado de la puerta (false = cerrada)
- [ ] Atributo `GLuint vao, vbo` — geometría del quad
- [ ] Atributo `Texture doorTexture` — textura cargada del `doors.png` (consistente con el resto del proyecto)
- [ ] Métodos: `init(glm::ivec2 tilePos)`, `update()`, `render()`, `toggleOpen()`

---

## Task 3 — Implementar Door::init()

- [ ] Definir constantes para evitar números mágicos:
  - [ ] `DOOR_WIDTH = 16`, `DOOR_HEIGHT = 24`, `DOOR_OFFSET_Y = 8`
  - [ ] Justificación: el sprite mide 24px de alto y el ancla está en un tile de 16px, por eso se desplaza 8px hacia arriba.
- [ ] Calcular `worldPos` a partir del `tilePos`:
  ```
  worldPos.x = tilePos.x * 16
  worldPos.y = tilePos.y * 16 - 8   // sube 8px porque el sprite mide 24px
  ```
- [ ] Cargar `doors.png` como textura OpenGL con `GL_NEAREST` (sin interpolación, pixel art)
- [ ] Crear el VAO y VBO con un quad de 16x24px
- [ ] Formato de vértices: `x, y, u, v` (4 vértices, GL_TRIANGLE_FAN)
- [ ] Definir las UVs iniciales para puerta **cerrada** (frame inferior del PNG):
  ```
  uvY0 = 0.5f   (y=24/48)
  uvY1 = 1.0f   (y=48/48)
  ```
- [ ] Subir los vértices al VBO con `GL_DYNAMIC_DRAW`

---

## Task 4 — Implementar Door::toggleOpen()

- [ ] Invertir el valor de `isOpen`
- [ ] Calcular las nuevas UVs según el estado:
  - Cerrada: `uvY0 = 0.5f`, `uvY1 = 1.0f`
  - Abierta:  `uvY0 = 0.0f`, `uvY1 = 0.5f`
- [ ] Actualizar solo las UVs del VBO con `glBufferSubData` (no recrear el buffer entero)

---

## Task 5 — Implementar Door::render()

- [ ] Bindear `doorTexture` usando `doorTexture.use()` (o equivalente del wrapper `Texture`)
- [ ] Bindear el VAO
- [ ] Llamar a `glDrawArrays(GL_TRIANGLE_FAN, 0, 4)`
- [ ] Asegurarse de usar el mismo shader que el resto de sprites del juego

---

## Task 6 — Modificar TileMap::loadLevel()

- [ ] Definir la constante `TILE_DOOR = 999` (o el marcador elegido en Task 1)
- [ ] Añadir `vector<glm::ivec2> doorPositions` en `TileMap` y limpiarlo al inicio de cada `loadLevel()`
- [ ] En el bucle de parseo, al encontrar `TILE_DOOR`:
  - [ ] Guardar la posición `(i, j)` en un vector `doorPositions`
  - [ ] Escribir `0` en `map[j][i]` (en este proyecto `-1` provocaría UVs inválidas al renderizar el tilemap)
- [ ] Añadir getter `getDoorPositions()` que devuelva el vector de posiciones

---

## Task 7 — Integrar las puertas en Scene

- [ ] En `Scene::init()`, después de crear el `TileMap`:
  - [ ] Iterar sobre `tilemap->getDoorPositions()`
  - [ ] Crear un objeto `Door` por cada posición y llamar a `door->init(pos)`
  - [ ] Guardar los objetos en un vector `vector<Door*> doors`
- [ ] En `Scene::update()`:
- [ ] Detectar pulsación de tecla `D` por flanco (no por tecla mantenida)
  - [ ] Al pulsarla, llamar a `door->toggleOpen()` en todas las puertas
- [ ] En `Scene::render()`, en este orden:
  ```
  tilemap->render()
  for each door → door->render()
  player->render()
  ```

---

## Task 7.1 — Registrar archivos nuevos en Visual Studio

- [ ] Añadir `Door.h` y `Door.cpp` a `02-Bubble.vcxproj`
- [ ] Añadirlos también a `02-Bubble.vcxproj.filters` para que aparezcan en el IDE

---

## Task 8 — Verificación visual

- [ ] Ejecutar el juego y comprobar que la puerta aparece en la posición correcta del mapa
- [ ] Pulsar `D` y verificar que el sprite cambia de cerrada a abierta y viceversa
- [ ] Comprobar que no hay artefactos visuales (bordes, bleeding de UVs)
- [ ] Comprobar que el tile de fondo detrás de la puerta no se ve (tapado por el sprite)
