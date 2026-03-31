# Warp Platform (TP) – Implementation Tasks

> **Diseño:** Se usan **dos IDs distintos** (igual que con los tubos) — uno para la tile con suelo (`WARP_TILE_FLOOR`) y otro para la tile sin suelo (`WARP_TILE_NO_FLOOR`). Cada ID determina tanto la imagen renderizada como el comportamiento de colisión. Los pares se emparejan por orden de aparición en el mapa (pos[0]↔pos[1], pos[2]↔pos[3], etc.).

## TileMap

- [ ] Definir las dos constantes en `TileMap.h`:
  - [ ] `WARP_TILE_FLOOR 989` → tile con suelo sólido (el jugador puede estar encima)
  - [ ] `WARP_TILE_NO_FLOOR 988` → tile sin suelo (el jugador atraviesa, solo actúa como marcador de destino)
- [ ] Declarar el método `getWarpPlatformPairs()` en `TileMap.h` con retorno `std::vector<std::pair<glm::ivec2, glm::ivec2>>`
- [ ] Implementar `getWarpPlatformPairs()` en `TileMap.cpp`:
  - [ ] Escanear todo el mapa en orden (fila por fila, izquierda→derecha) buscando tiles con ID `WARP_TILE_FLOOR` **o** `WARP_TILE_NO_FLOOR`
  - [ ] Agrupar las posiciones encontradas en pares consecutivos (pos[0]↔pos[1], pos[2]↔pos[3], etc.)
  - [ ] Retornar el vector de pares
- [ ] Asegurarse de que `WARP_TILE_NO_FLOOR` **no** tiene colisión sólida en `collisionMoveDown` / `collisionMoveLeft` / `collisionMoveRight` (tratarlo como tile vacía a efectos de colisión)
- [ ] (Opcional) Añadir método helper `getTile(int x, int y)` si no existe, para acceso limpio al mapa por coordenada de tile

## Player

- [ ] Añadir atributo `bool bWarpUsed = false` en `Player.h`
- [ ] En `Player::update()`, calcular la tile donde están **los pies** del jugador (centro horizontal, borde inferior)
- [ ] Comprobar si la tile bajo los pies tiene ID `WARP_TILE_FLOOR` (solo se activa desde la tile con suelo)
- [ ] Si el jugador está sobre la tile warp, pulsa `GLFW_KEY_DOWN` y `bWarpUsed == false`:
  - [ ] Llamar a `map->getWarpPlatformPairs()` para obtener los pares
  - [ ] Iterar los pares para encontrar la tile destino (si está en `first` → ir a `second`, y viceversa)
  - [ ] Reubicar `pos` del jugador encima de la tile destino (alinear horizontalmente y verticalmente)
  - [ ] Poner `bWarpUsed = true` para evitar re-trigger mientras se mantiene pulsado DOWN
- [ ] Resetear `bWarpUsed = false` cuando se suelte `GLFW_KEY_DOWN` o el jugador ya no esté sobre la tile warp

## Niveles / Assets

- [ ] Añadir el sprite de la plataforma warp **con suelo** a la tilesheet con ID `989`
- [ ] Añadir el sprite de la plataforma warp **sin suelo** a la tilesheet con ID `988`
- [ ] Colocar las tiles en los archivos `.txt` de los niveles en los que aplique:
  - [ ] Siempre en número par (cada dos tiles forman un par, sin importar si son `989`, `988` o mixtas)
  - [ ] Un par típico será `WARP_TILE_FLOOR (989)` ↔ `WARP_TILE_NO_FLOOR (988)` si los dos puntos tienen distinta estructura de suelo
Solo hay warps en el nivel 4. El warp Tile Floor corresponde al id 193 del mapa, mientras que el Warp tile no floor corresponde al id 37. Solo deberas cambiar este txt relacionando la imagen de los tiles con id 193 y 37 con sus respectivos ids 989 y 988.

## Testing & Debug

- [ ] Probar que al pulsar DOWN sobre `WARP_TILE_FLOOR` el jugador aparece encima de la tile destino
- [ ] Probar que mantener DOWN no produce un bucle de teletransporte continuo (`bWarpUsed`)
- [ ] Probar el viaje en ambas direcciones (A→B y B→A)
- [ ] Probar con más de un par de warp en el mismo nivel
- [ ] Verificar que `WARP_TILE_NO_FLOOR` no bloquea el movimiento del jugador (colisión transparente)
- [ ] Verificar que no hay problemas de colisión al aparecer en el destino (el jugador no queda dentro de una tile sólida)
- [ ] Comprobar que los enemigos **no** usan la plataforma warp (la lógica solo está en `Player`)
