# Jump Platform (UP) – Implementation Tasks

> **Diseño:** Tile ID `987`. Al pisarla, el jugador recibe una velocidad vertical inicial que lo lanza ~7 tiles hacia arriba. La gravedad normal reduce la velocidad cada frame. Si se pulsa LEFT o RIGHT durante el salto, se aplica un damping adicional que frena el ascenso progresivamente. No hay tecla activadora: el trigger es pisar la tile.

## TileMap

- [ ] Definir la constante `JUMP_PLATFORM_TILE 987` en `TileMap.h`
- [ ] Añadir el sprite de la jump platform a la tilesheet con ID `987`
- [ ] Verificar que `JUMP_PLATFORM_TILE` **no** tiene colisión sólida lateral (`collisionMoveLeft` / `collisionMoveRight`) si es una tile de paso

## Player

- [ ] Añadir atributos en `Player.h`:
  - [ ] `bool bJumpPlatformActive = false`
  - [ ] `float jumpVelocity = 0.f`
- [ ] Definir las constantes de la mecánica en `Player.h`:
  - [ ] `JUMP_PLATFORM_VEL` → velocidad inicial negativa (hacia arriba), calcular con \`sqrt(2 * gravity * 112)\` y ajustar empíricamente hasta alcanzar ~7 tiles (112 px)
  - [ ] `JUMP_PLATFORM_DAMP` → cantidad que se suma a `jumpVelocity` cada frame al pulsar LEFT/RIGHT (valor de partida: `0.4f`)
- [ ] En `Player::update()`, añadir **detección** de la tile (solo si `onGround && !bJumpPlatformActive`):
  - [ ] Calcular `feetTile` (centro horizontal del jugador, borde inferior)
  - [ ] Si la tile en `feetTile` es `JUMP_PLATFORM_TILE`: asignar `jumpVelocity = -JUMP_PLATFORM_VEL`, activar `bJumpPlatformActive = true` y desactivar `onGround = false`
- [ ] En `Player::update()`, añadir **bloque de salto** (solo si `bJumpPlatformActive`):
  - [ ] Aplicar `pos.y += jumpVelocity` cada frame
  - [ ] Aplicar gravedad normal: `jumpVelocity += gravity`
  - [ ] Si LEFT o RIGHT está pulsado: `jumpVelocity += JUMP_PLATFORM_DAMP` (frena el ascenso)
  - [ ] Si hay colisión con techo (`collisionMoveUp`): resetear `jumpVelocity = 0.f` y `bJumpPlatformActive = false`
  - [ ] Si `jumpVelocity >= 0.f` (el jugador ya no sube): desactivar `bJumpPlatformActive = false` y ceder el control a la gravedad normal

## Niveles / Assets

- [ ] Añadir el sprite de la jump platform a la tilesheet con ID `987`
- [ ] Colocar tiles `987` en los archivos `.txt` de los niveles correspondientes
- [ ] Lvl01 plataforma con id 142. debes linkar el id 987 con este id del mapa. En el lvl 04 hay otra plataforma de up con el id 114.
- [ ] Debes linkar los ids 142 y 114 con el id 987 de la tilesheet para que al colocar estas tiles en el mapa se renderice la imagen correcta y se active la mecánica de salto al pisarlas, pero cada una en su respectivo nivel. el 142 del lvl 04 no debe renderizar la plataforma y viceversa con el nivel 1.
- [ ] Verificar visualmente en cada nivel que la trayectoria de ~7 tiles hacia arriba no colisiona con techo de forma no intencionada

## Testing & Debug

- [ ] Verificar que el jugador salta al pisar la tile sin necesidad de pulsar ninguna tecla
- [ ] Medir que la altura alcanzada es aproximadamente 7 tiles (112 px) sin pulsar LEFT/RIGHT
- [ ] Verificar que pulsar LEFT o RIGHT reduce la altura alcanzada de forma progresiva y suave
- [ ] Verificar que el movimiento horizontal funciona con normalidad durante el salto
- [ ] Comprobar que al colisionar con un techo durante el salto, el jugador cae correctamente
- [ ] Comprobar que pisar la tile mientras ya está en el aire (`!onGround`) **no** re-activa el salto
- [ ] Verificar que al aterrizar tras el salto se recupera el control normal del jugador
- [ ] Comprobar que los enemigos **no** se ven afectados por la jump platform (lógica solo en `Player`)
