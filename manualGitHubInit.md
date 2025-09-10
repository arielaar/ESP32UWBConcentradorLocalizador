# Guía para Inicializar y Mantener un Proyecto en GitHub

Este manual describe el proceso completo para tomar un proyecto existente, inicializar un repositorio de Git, subirlo a GitHub, y mantenerlo actualizado en el día a día.

## Prerrequisitos

- **Git instalado en tu PC:** Debes poder ejecutar comandos `git` en tu terminal. Si no, descárgalo desde [git-scm.com](https://git-scm.com/).
- **Cuenta en GitHub:** Necesitas una cuenta en [github.com](https://github.com).

---

## Parte 1: Flujo Principal - Subir un Proyecto por Primera Vez

Sigue estos pasos si tu proyecto aún no es un repositorio de Git.

### Paso 1.1: Crear un Repositorio Vacío en GitHub

1.  Inicia sesión en [github.com](https://github.com).
2.  Haz clic en el ícono **+** en la esquina superior derecha y selecciona **"New repository"**.
3.  **Dale un nombre** a tu repositorio (ej. `esp32-uwb-concentrador`).

> **¡MUY IMPORTANTE!**
> No marques **ninguna** de las casillas para inicializar el repositorio con `README`, `.gitignore` o `license`. El repositorio debe crearse **completamente vacío** para evitar conflictos.

4.  Haz clic en **"Create repository"**.
5.  En la siguiente página, busca la sección **"...or push an existing repository from the command line"** y copia la URL que aparece.

### Paso 1.2: Inicializar y Subir tu Proyecto Local

Ejecuta esta secuencia de comandos en orden desde la terminal, dentro de la carpeta de tu proyecto.

```bash
# 1. Inicializa un repositorio de Git en la carpeta actual.
git init

# 2. Agrega todos los archivos al área de preparación (staging).
git add .

# 3. Guarda los archivos en el historial local con un mensaje descriptivo.
git commit -m "Commit inicial del proyecto Concentrador UWB"

# 4. Renombra la rama principal a "main".
git branch -M main

# 5. Conecta tu repositorio local con el de GitHub (pega tu URL).
git remote add origin https://github.com/tu-usuario/tu-repositorio.git

# 6. Sube tus archivos a GitHub.
git push -u origin main
```

---

## Parte 2: Solución de Problemas - Empezar de Cero

### ¿Cuándo Usar esta Sección?

Úsala si al ejecutar `git push` por primera vez, recibes el error `failed to push some refs to '...'`. Esto significa que el repositorio en GitHub no estaba vacío. La solución más limpia es resetear.

### Paso 2.1: Resetear el Proyecto Local

Este comando elimina la configuración de Git **sin borrar tus archivos de código**.

```bash
# Para Windows:
rmdir /s /q .git
```

### Paso 2.2: Resetear el Repositorio Remoto en GitHub

1.  Ve a la página de tu repositorio en GitHub.
2.  Ve a **Settings** > **Danger Zone** > **Delete this repository**.
3.  Confirma la eliminación.
4.  **Vuelve a crear el repositorio** siguiendo el **Paso 1.1**, asegurándote de que quede vacío.

### Paso 2.3: Reintentar la Subida

Ahora que todo está limpio, vuelve al **Paso 1.2** y ejecuta la secuencia de comandos de nuevo.

---

## Parte 3: Flujo de Trabajo Diario - Actualizar el Repositorio

Una vez que tu proyecto ya está en GitHub, este es el ciclo que seguirás para guardar y subir tus cambios.

### Paso 3.1: Ver el Estado de tu Proyecto

Este es el comando más importante del día a día. Te dice qué archivos has modificado y cuáles son nuevos.

```bash
git status
```

### Paso 3.2: Preparar los Cambios (Staging)

Antes de hacer un `commit`, debes decirle a Git exactamente qué cambios quieres guardar. Esto se hace con `git add`.

```bash
# Opción A: Agregar TODOS los archivos modificados y nuevos
git add .

# Opción B: Agregar solo un archivo específico
git add src/main.cpp

# Opción C: Agregar todos los archivos de una carpeta
git add include/
```

### Paso 3.3: Guardar los Cambios (Commit)

Una vez preparados los cambios, los guardas en el historial local con un mensaje que describa lo que hiciste.

```bash
# Escribe un mensaje claro y conciso
git commit -m "Añadida métrica RMS al cálculo de posición"
```

### Paso 3.4: Subir los Cambios a GitHub

Finalmente, subes todos los commits que has guardado localmente a GitHub.

```bash
# Como ya usaste "-u" la primera vez, ahora es más simple
git push
```

¡Y listo! Tu repositorio en GitHub estará actualizado con tus últimos cambios.

---

## Parte 4: Comandos Útiles Adicionales

Aquí tienes otros comandos que te serán de gran utilidad.

- **`git pull`**
  Descarga los últimos cambios desde GitHub a tu repositorio local. Esencial si trabajas en equipo o desde varias computadoras.

- **`git log`**
  Muestra el historial de todos los commits que has hecho. Presiona `q` para salir.
  ```bash
  # Versión más limpia y visual del historial
  git log --oneline --graph --decorate
  ```

- **`git diff`**
  Muestra los cambios que has hecho en los archivos que **aún no has preparado** con `git add`. Te permite ver las líneas exactas que has añadido o borrado.

- **`git diff --staged`**
  Muestra los cambios que ya has preparado con `git add` y que están listos para el próximo `commit`.

- **`git restore <archivo>`**
  Descarta los cambios que has hecho en un archivo y lo devuelve a su última versión guardada (la del último commit). ¡Úsalo con cuidado!
  ```bash
  # Deshacer los cambios en main.cpp desde el último commit
  git restore src/main.cpp
  ```

- **`git branch <nombre-rama>`**
  Crea una nueva "rama" o línea de desarrollo. Es una práctica avanzada para trabajar en nuevas funcionalidades sin afectar la versión principal (`main`).

- **`git switch <nombre-rama>`**
  Te cambia a la rama que especifiques para empezar a trabajar en ella.