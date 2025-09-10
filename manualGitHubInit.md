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

## Parte 2: Solución de Problemas Comunes

### Problema: `git push` falla con el error `failed to push some refs`

**Causa:** Esto ocurre si el repositorio en GitHub no estaba vacío cuando intentaste subir tu proyecto. Sus historiales son diferentes y Git te detiene por seguridad.

**Solución:** La forma más limpia es resetear todo.

1.  **Resetear Proyecto Local:** Borra la configuración local de Git (sin afectar tu código) con el comando para Windows:
    ```bash
    rmdir /s /q .git
    ```
2.  **Resetear Repositorio Remoto:** En la web de GitHub, ve a **Settings** > **Danger Zone** > **Delete this repository**. Luego, créalo de nuevo, asegurándote de que esté vacío.
3.  **Reintentar:** Vuelve a la **Parte 1** y sigue los pasos desde el principio.

### Problema: Añadí un archivo a `.gitignore`, pero sigue en GitHub

**Causa:** `.gitignore` solo evita que se suban archivos **nuevos** o que no están siendo rastreados. No afecta a los archivos que ya existen en el historial de Git (que ya fueron subidos con un `commit`).

**Solución:** Debes decirle a Git explícitamente que deje de rastrear ese archivo, sin borrarlo de tu disco duro.

1.  **Asegúrate de que el archivo esté en `.gitignore`:** Verifica que la regla para ignorar el archivo o carpeta (ej. `.vscode/`) esté guardada en tu archivo `.gitignore`.

2.  **Elimina el archivo del tracking de Git:** Usa el comando `git rm --cached`. La opción `--cached` es crucial, ya que elimina el archivo del repositorio pero lo mantiene en tu PC.
    ```bash
    # Para un solo archivo:
git rm --cached ruta/al/archivo.log

    # Para una carpeta completa (muy común para .vscode):
git rm -r --cached .vscode
    ```

3.  **Guarda este cambio (Commit):** La eliminación del tracking es un cambio que debe guardarse en el historial.
    ```bash
    git commit -m "Dejar de rastrear archivos de configuración local"
    ```

4.  **Sube el cambio a GitHub:**
    ```bash
    git push
    ```

Al terminar, el archivo o carpeta desaparecerá de tu repositorio en GitHub, pero seguirá en tu PC para tu uso, y Git no volverá a intentar subirlo.

> **En resumen: `.gitignore` es para el futuro, `git rm --cached` es para corregir el pasado.**

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

---

## Parte 4: Comandos Útiles Adicionales

- **`git pull`**: Descarga los últimos cambios desde GitHub a tu repositorio local.
- **`git log --oneline --graph`**: Muestra el historial de commits de forma compacta y visual.
- **`git diff`**: Muestra los cambios en archivos que aún no has preparado con `git add`.
- **`git diff --staged`**: Muestra los cambios que ya están preparados para el próximo commit.
- **`git restore <archivo>`**: Descarta los cambios en un archivo, devolviéndolo a su última versión guardada.
