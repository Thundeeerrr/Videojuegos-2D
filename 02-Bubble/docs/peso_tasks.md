# 🪨 Implementación del Peso — Task List

## 1. Utils.h — Colisión AABB genérica

- [ ] Crear `Utils.h` en la carpeta del proyecto
- [ ] Implementar la función libre `collidesWith`:
  - Firma: `bool collidesWith(glm::vec2 posA, glm::ivec2 sizeA, glm::vec2 posB, glm::ivec2 sizeB)`
  - Retorna `true` si los dos rectángulos AABB se solapan
  - Condición: `posA.x < posB.x + sizeB.x && posA.x + sizeA.x > posB.x && posA.y < posB.y + sizeB.y && posA.y + sizeA.y > posB.y`

---

## 2. Pushable.h

- [ ] Crear `Pushable.h` con los siguientes atributos privados:
  - `Sprite *sprite` — sprite visual del peso
  - `Texture spritesheet` — textura del tilesheet de objetos
  - `TileMap *map` — puntero al mapa para colisiones
  - `glm::vec2 position` — posición actual en píxeles
  - `glm::ivec2 size = {16, 16}` — tamaño del AABB
  - `float speed = 80.f` — píxeles por segundo de movimiento horizontal
  - `int dirX = 0` — dirección actual: +1 derecha, -1 izquierda
  - `bool moving = false` — si está en movimiento horizontal
  - `float velocityY = 0.f` — velocidad vertical acumulada por gravedad
  - `bool onGround = false` — si el peso está apoyado en el suelo
- [ ] Declarar los siguientes métodos públicos:
  - `void init(glm::ivec2 tileMapPos, ShaderProgram &shaderProgram, TileMap *tileMap)`
  - `void update(int deltaTime)`
  - `void render()`
  - `void push(int dir)` — activa el movimiento horizontal en la dirección `dir`
  - `glm::vec2 getPosition() const`
  - `glm::ivec2 getSize() const`
  - `bool isMoving() const`
  - `bool isOnGround() const`

---

## 3. Pushable.cpp

- [ ] Implementar `init`:
  - Guardar `tileMap` en el atributo `map`
  - Cargar la textura: `spritesheet.loadFromFile("images/objects.png", TEXTURE_PIXEL_FORMAT_RGBA)`
  - Crear el sprite con `Sprite::createSprite(glm::ivec2(16,16), glm::vec2(1.f / NUM_COLS_SHEET, 1.f), &spritesheet, &shaderProgram)`
    - Sustituir `NUM_COLS_SHEET` por el número de columnas reales del tilesheet de objetos
  - Configurar animación estática de 1 frame:
    - `setNumberAnimations(1)`
    - `setAnimationSpeed(0, 1)`
    - `addKeyframe(0, glm::vec2(COL * (1.f / NUM_COLS_SHEET), 0.f))` donde `COL` es la columna del peso en el sheet
  - Llamar a `sprite->changeAnimation(0)`
  - Asignar `position = glm::vec2(tileMapPos)`
  - Llamar a `sprite->setPosition(position)`

- [ ] Implementar `push(int dir)`:
  - Si `moving == true` o `onGround == false`, no hacer nada (no empujar si ya se mueve o está en el aire)
  - Si `moving == false` y `onGround == true`: asignar `dirX = dir` y `moving = true`

- [ ] Implementar `update(int deltaTime)`:
  - Calcular `float dt = deltaTime / 1000.f`
  - **Gravedad (siempre, antes que el movimiento horizontal):**
    - Si `map->collisionMoveDown(position, size)` devuelve `false`:
      - `velocityY += GRAVITY * dt` (usar constante `GRAVITY = 300.f` p.ej.)
      - `position.y += velocityY * dt`
      - `onGround = false`
    - Si devuelve `true`:
      - `velocityY = 0.f`
      - `onGround = true`
  - **Movimiento horizontal (solo si `onGround == true`):**
    - `float delta = speed * dt`
    - Si `dirX == +1`:
      - Si `map->collisionMoveRight(position, size)` devuelve `true`: `moving = false`, parar
      - Si devuelve `false`: `position.x += delta`
    - Si `dirX == -1`:
      - Si `map->collisionMoveLeft(position, size)` devuelve `true`: `moving = false`, parar
      - Si devuelve `false`: `position.x -= delta`
  - Llamar a `sprite->setPosition(position)`
  - Llamar a `sprite->update(deltaTime)`

- [ ] Implementar `render()`:
  - Llamar a `sprite->render()`

- [ ] Implementar getters devolviendo `position`, `size`, `moving` y `onGround`

---

## 4. Integración en Scene

- [ ] En `Scene.h` añadir atributo `Pushable *weight`
- [ ] En `Scene::init` instanciar y llamar a `weight->init(posicionEnTiles, shaderProgram, tileMap)`
- [ ] En `Scene::update`:
  - Llamar a `weight->update(deltaTime)`
  - **Colisión jugador → peso (empuje):**
    - Solo comprobar si `weight->isOnGround() == true`
    - Llamar a `collidesWith(player->getPosition(), player->getSize(), weight->getPosition(), weight->getSize())`
    - Si colisionan, calcular:
      - `playerCenter = player->getPosition().x + player->getSize().x / 2.f`
      - `weightCenter = weight->getPosition().x + weight->getSize().x / 2.f`
    - Si `playerCenter < weightCenter` y el jugador se mueve a la derecha → `weight->push(+1)`
    - Si `playerCenter > weightCenter` y el jugador se mueve a la izquierda → `weight->push(-1)`
  - **Colisión peso → enemigo (matar):**
    - Comprobar si `weight->isMoving() == true` O `weight->isOnGround() == false` (mata también al caer)
    - Para cada `enemy` en `enemies`: llamar a `collidesWith(weight->getPosition(), weight->getSize(), enemy->getPosition(), enemy->getSize())`
    - Si colisionan: llamar a `enemy->kill()`
- [ ] En `Scene::render` llamar a `weight->render()`

---

## 5. Sprite del peso en GIMP

- [ ] Generar imagen del peso con IA
- [ ] Abrir en GIMP, añadir canal alfa (clic derecho sobre la capa → `Añadir canal alfa`)
- [ ] Eliminar fondo: `Colores → Color a Alpha` → seleccionar el color de fondo → Aceptar
- [ ] Escalar a 16×16: `Imagen → Escalar la imagen` con interpolación `Ninguna`
- [ ] Abrir el tilesheet de objetos como lienzo principal
- [ ] Importar el peso como capa: `Archivo → Abrir como capas`
- [ ] Activar rejilla 16×16: `Editar → Preferencias → Rejilla predeterminada` → espaciado 16×16
- [ ] Activar snap: `Ver → Ajustar a → Rejilla`
- [ ] Colocar el peso en su celda y anotar la columna (ej. columna 0 → `COL = 0`)
- [ ] Exportar como `objects.png`: `Archivo → Exportar como`
- [ ] Actualizar `COL` y `NUM_COLS_SHEET` en `Pushable::init` con los valores correctos

---

## 6. Testing

- [ ] El peso aparece en la posición correcta del nivel al iniciar
- [ ] El peso cae si hay un hueco debajo al iniciarse o al ser empujado sobre él
- [ ] El peso se detiene verticalmente al llegar al suelo (`collisionMoveDown`)
- [ ] Al caminar contra el peso por la derecha (estando en suelo), se mueve hacia la derecha
- [ ] Al caminar contra el peso por la izquierda (estando en suelo), se mueve hacia la izquierda
- [ ] No se puede empujar el peso mientras ya está en movimiento horizontal
- [ ] No se puede empujar el peso mientras está en el aire
- [ ] El peso se detiene horizontalmente al llegar a un muro del TileMap
- [ ] Un enemigo muere al ser alcanzado por el peso en movimiento horizontal
- [ ] Un enemigo muere al ser aplastado por el peso en caída vertical
- [ ] El peso en reposo sobre el suelo no hace daño al jugador ni a los enemigos
