# Door Teleport – Implementation Tasks

> **Diseño:** IDs `985` y `986` (uno con escalera, otro sin). Los IDs únicamente determinan el sprite renderizado — la lógica de conexión vive íntegramente en `doorLinks`, un `std::unordered_map<int, int>` donde la clave es la posición de la tile codificada como `y * mapWidth + x`. Al pulsar **UP** sobre cualquiera de las dos tiles, se busca la posición actual en `doorLinks` y se obtiene el destino directamente. La bidireccionalidad se garantiza insertando ambas direcciones en `addDoorLink()`. Se reutiliza el flag `bWarpUsed` del jugador para evitar re-triggers.

## TileMap.h

- [ ] Añadir `#include <unordered_map>` en `TileMap.h`
- [ ] Definir las dos constantes de sprite:
  - [ ] `DOOR_TILE_STAIRS    986` → sprite puerta con escalera
  - [ ] `DOOR_TILE_NO_STAIRS 985` → sprite puerta sin escalera
- [ ] Declarar el atributo privado `std::unordered_map<int, int> doorLinks`
- [ ] Declarar los dos helpers privados de codificación:
  - [ ] `int encodeTile(glm::ivec2 pos) const` → retorna `pos.y * mapSize.x + pos.x`
  - [ ] `glm::ivec2 decodeTile(int key) const` → retorna `{ key % mapSize.x, key / mapSize.x }`
- [ ] Declarar los métodos públicos:
  - [ ] `void addDoorLink(glm::ivec2 a, glm::ivec2 b)`
  - [ ] `void clearDoorLinks()`
  - [ ] `glm::ivec2 getDoorDestination(glm::ivec2 tilePos) const`
  - [ ] `bool isDoorTile(glm::ivec2 tilePos) const`

## TileMap.cpp

- [ ] Implementar `addDoorLink(a, b)`:
  - [ ] Insertar `doorLinks[encodeTile(a)] = encodeTile(b)`
  - [ ] Insertar `doorLinks[encodeTile(b)] = encodeTile(a)` → bidireccional automático
- [ ] Implementar `clearDoorLinks()`: llamar a `doorLinks.clear()`
- [ ] Implementar `getDoorDestination(tilePos)`:
  - [ ] Buscar `encodeTile(tilePos)` en `doorLinks` con `find()`
  - [ ] Si no se encuentra, retornar `{-1, -1}` como señal de "sin destino"
  - [ ] Si se encuentra, retornar `decodeTile(it->second)`
- [ ] Implementar `isDoorTile(tilePos)`:
  - [ ] Retornar `true` si la tile es `DOOR_TILE_STAIRS` **o** `DOOR_TILE_NO_STAIRS`
  - [ ] `int tile = map[tilePos.y * mapSize.x + tilePos.x]`
  - [ ] `return tile == DOOR_TILE_STAIRS || tile == DOOR_TILE_NO_STAIRS`

## Scene.cpp

- [ ] En el `init()` de **cada nivel**, llamar a `map->clearDoorLinks()` antes de definir los pares
- [ ] Llamar a `map->addDoorLink(posA, posB)` por cada par de puertas del nivel:
  - [ ] Las posiciones son coordenadas de **tile** (`glm::ivec2`), no píxeles
  - [ ] Se pueden mezclar libremente `985` y `986` en un mismo par — el ID no afecta la conexión
  - [ ] `addDoorLink` ya registra ambas direcciones, no hace falta llamarla dos veces por par
  - [ ] Ejemplo: `map->addDoorLink({3, 10}, {3, 4})` conecta la puerta en (3,10) con la de (3,4)

## Player.cpp

- [ ] Reutilizar el atributo `bool bWarpUsed` ya existente (misma lógica anti-retrigger)
- [ ] En `Player::update()`, calcular `playerTile` como la tile bajo los pies del jugador:
  - [ ] X: `int((pos.x + playerSize.x / 2.f) / tileSize)` → centro horizontal
  - [ ] Y: `int((pos.y + playerSize.y - 1.f) / tileSize)` → borde inferior
- [ ] Añadir bloque de detección de puerta:
  - [ ] Condición: `isDoorTile(playerTile) && GLFW_KEY_UP pulsado && !bWarpUsed`
  - [ ] Llamar a `getDoorDestination(playerTile)`
  - [ ] Si el destino es válido (`dest.x != -1`): reubicar `pos` del jugador encima de la tile destino:
    - [ ] `pos.x = dest.x * tileSize`
    - [ ] `pos.y = dest.y * tileSize - playerSize.y + tileSize`
  - [ ] Poner `bWarpUsed = true`
- [ ] Resetear `bWarpUsed = false` cuando se suelte `GLFW_KEY_UP` **o** el jugador ya no esté sobre una tile de puerta

## Niveles / Assets

- [ ] Añadir sprite puerta con escalera a la tilesheet con ID `986`
- [ ] Añadir sprite puerta sin escalera a la tilesheet con ID `985`
- [ ] Colocar tiles `985` o `986` en los `.txt` de cada nivel según el sprite deseado en cada punto. En el nivel 2 tenemos el id 30 para la puerta sin  escalera y el id 67 para la puerta con escalera. En el nivel 4 id 28 sin escalera y id 122 con escalera. En el nivle 5 id 164 con escalera y sin escalera el id 123.
- [ ] Verificar que cada tile de puerta colocada en el mapa tiene su pareja registrada en `addDoorLink()`

## Testing & Debug

- [ ] Probar que pulsar UP sobre una puerta teletransporta al jugador a la puerta destino
- [ ] Probar el viaje en ambas direcciones (A→B y B→A) desde cada par
- [ ] Probar un par mixto (`985` ↔ `986`) y verificar que funciona igual
- [ ] Verificar que mantener UP pulsado no produce un bucle de teletransporte (`bWarpUsed`)
- [ ] Probar con múltiples pares de puertas en el mismo nivel sin interferencias entre pares
- [ ] Verificar que si una tile de puerta no tiene pareja en `doorLinks`, no ocurre ningún crash (`dest.x == -1`)
- [ ] Verificar que el jugador aparece correctamente posicionado encima de la tile destino
- [ ] Comprobar que `clearDoorLinks()` se llama al cambiar de nivel para no arrastrar pares anteriores
- [ ] Comprobar que los enemigos **no** usan las puertas (lógica solo en `Player`)
