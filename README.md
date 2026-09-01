# UAH Motorsport – Código TFG

Este repositorio contiene el código desarrollado para el **Trabajo Fin de Grado sobre la implementación de la red CANBUS del monoplaza eléctrico de UAH Motorsport**.

El software se organiza por unidades de control del vehículo. Cada carpeta corresponde a una ECU distinta y contiene el proyecto de STM32 asociado.

## Estructura del repositorio

```text
UAH-Motorsport/
├── AMSMTR/
├── PDU/
├── VCU/
└── README.md
```

### `VCU/` – Vehicle Control Unit

La **VCU** actúa como unidad principal de control del vehículo y como uno de los nodos de la red CAN.

Entre las funciones implementadas se encuentran:

- Lectura de las dos señales de acelerador **APPS1 y APPS2**.
- Comprobación de plausibilidad entre ambos sensores.
- Lectura de la señal de freno.
- Gestión de la secuencia **Ready To Drive (R2D)**.
- Lectura del pulsador de arranque.
- Envío de la petición de activación R2D a través de CAN.
- Recepción de estados procedentes de la PDU y del AMS.
- Envío de comandos al inversor mediante CAN.
- Gestión del `Drive Enable`.
- Envío de la consigna de corriente relativa al inversor.
- Activación del buzzer R2D una vez confirmada la secuencia de arranque.

La VCU utiliza **FDCAN en modo Classic CAN** para la comunicación con el resto de unidades.

---

### `PDU/` – Power Distribution Unit

La **PDU** se encarga de la gestión de elementos relacionados con la distribución de potencia y participa directamente en la secuencia de puesta en marcha del vehículo.

En el contexto de este proyecto, su función principal dentro de la red CAN es gestionar y comunicar el estado de la secuencia **Ready To Drive**.

Entre la información intercambiada se encuentran estados relacionados con:

- Secuencia R2D.
- Estado de los AIR.
- Precharge.
- Discharge.
- Errores asociados a la secuencia de arranque.
- Confirmación a la VCU de que el vehículo ha alcanzado el estado R2D activo.

La VCU solicita el inicio de la secuencia mediante CAN y permanece a la espera de la confirmación correspondiente por parte de la PDU.

---

### `AMSMTR/` – Accumulator Management System

La carpeta **AMSMTR** contiene el software correspondiente al **Accumulator Management System (AMS)**.

El AMS es el nodo encargado de recopilar y comunicar a través de CAN la información relevante del acumulador de alta tensión.

Entre las magnitudes utilizadas por la red se encuentran:

- Tensiones del acumulador.
- Corriente de batería.
- Temperaturas.
- Estados de seguridad y diagnóstico.
- Información necesaria para la supervisión del sistema de tracción.

La información obtenida por el AMS se transmite a otros nodos, principalmente a la VCU, mediante la red CAN.

---

# Organización interna de cada proyecto STM32

Aunque puede haber pequeñas diferencias entre las tres ECUs, los proyectos siguen la estructura habitual generada mediante **STM32CubeMX / STM32CubeIDE**.

```text
ECU/
├── Core/
│   ├── Inc/
│   └── Src/
├── Drivers/
├── CMakeLists.txt
├── *.ioc
└── ...
```

## `Core/Inc`

Contiene los archivos de cabecera (`.h`) utilizados por el proyecto.

Los más relevantes son:

### `main.h`

Contiene definiciones generales utilizadas por el programa, especialmente:

- Pines GPIO.
- Nombres asignados a señales.
- Definiciones compartidas por distintos módulos.

### `gpio.h`

Declaraciones relacionadas con la configuración de entradas y salidas digitales.

### `adc.h`

Declaraciones de los ADC utilizados para la adquisición de señales analógicas.

En la VCU se utilizan, entre otras, para las señales de acelerador y freno.

### `fdcan.h`

Declaraciones relacionadas con el periférico FDCAN utilizado para implementar la comunicación CAN.

### `usart.h`

Cuando está presente, contiene la configuración de la interfaz serie utilizada principalmente para diagnóstico y visualización de mensajes de depuración.

---

# `Core/Src`

Contiene el código fuente principal del microcontrolador.

### `main.c`

Es el archivo principal de cada ECU.

Contiene:

- Inicialización del microcontrolador.
- Inicialización de periféricos.
- Lógica principal de funcionamiento.
- Gestión de estados.
- Procesamiento y envío de mensajes CAN.
- Funciones específicas de cada unidad de control.

En el caso de la VCU, aquí se encuentra gran parte de la lógica de:

```text
Lectura de entradas
        ↓
Comprobaciones de seguridad
        ↓
Secuencia R2D
        ↓
Comunicación CAN
        ↓
Control del inversor
```

### `fdcan.c`

Contiene la configuración del periférico **FDCAN** del STM32.

Define parámetros necesarios para la comunicación CAN, como:

- Bit timing.
- Modo de funcionamiento.
- Filtros de recepción.
- Configuración de las FIFOs.
- Número de elementos de transmisión y recepción.

La lógica de aplicación asociada a las tramas CAN se mantiene principalmente en `main.c`.

### `adc.c`

Contiene la configuración de los conversores analógico-digitales del microcontrolador.

Se utiliza para adquirir señales procedentes de sensores analógicos.

En la VCU se emplea para señales como:

- APPS1.
- APPS2.
- Freno.

### `gpio.c`

Configura los pines utilizados como entradas o salidas digitales.

Algunos ejemplos son:

- Pulsador de arranque.
- Señales digitales de seguridad.
- Buzzer R2D.
- Entradas o salidas auxiliares.

### `usart.c`

Cuando está incluido, contiene la configuración de la comunicación serie.

Su finalidad principal durante el desarrollo es permitir la visualización de mensajes de diagnóstico y facilitar la comprobación paso a paso del funcionamiento del software.

### `stm32g4xx_it.c`

Contiene los manejadores de interrupciones del microcontrolador.

### `stm32g4xx_hal_msp.c`

Contiene funciones de inicialización de bajo nivel utilizadas por la HAL de ST.

### `system_stm32g4xx.c`

Incluye funciones relacionadas con la configuración básica del sistema y del reloj del microcontrolador.

---

# Archivo `.ioc`

Cada proyecto incluye un archivo:

```text
*.ioc
```

Este archivo contiene la configuración de **STM32CubeMX** utilizada para generar la inicialización del microcontrolador.

En él se definen, entre otros:

- Pinout.
- GPIO.
- ADC.
- FDCAN.
- USART.
- Clocks.
- Configuración de periféricos.

Es importante conservar este archivo porque permite visualizar y modificar la configuración hardware del proyecto mediante STM32CubeMX.

---

# `Drivers/`

Contiene las librerías proporcionadas por STMicroelectronics necesarias para trabajar con el microcontrolador STM32.

Principalmente incluye:

- **CMSIS**
- **STM32 HAL Drivers**

Estos archivos no contienen la lógica específica del vehículo, sino las funciones de bajo nivel empleadas por el software desarrollado.

---

# `CMakeLists.txt`

Archivo utilizado por **CMake** para definir la compilación del proyecto.

Permite indicar:

- Archivos fuente.
- Directorios de cabeceras.
- Drivers.
- Opciones de compilación.
- Dependencias del proyecto.

Es especialmente relevante cuando el código se compila y depura desde entornos como **CLion**.

---

# Comunicación CAN

Los distintos sistemas están conectados mediante una red **CAN 2.0 / Classic CAN**.

De forma simplificada, la arquitectura implementada puede representarse como:

```text
             ┌─────────┐
             │   AMS   │
             └────┬────┘
                  │
                  │ CAN
                  │
┌─────────┐   ┌───┴────┐   ┌──────────┐
│   PDU   ├───┤  VCU   ├───┤ Inverter │
└─────────┘   └────────┘   └──────────┘
```

La red permite que cada ECU intercambie únicamente la información necesaria para el funcionamiento y supervisión del vehículo.

---

# Secuencia Ready To Drive

Una de las funciones desarrolladas en la VCU es la secuencia de activación **Ready To Drive**.

De forma simplificada:

```text
Freno pulsado
      +
Pulsador de ARRANQUE
      ↓
Condiciones mantenidas
durante el tiempo requerido
      ↓
VCU envía petición R2D
por CAN
      ↓
PDU ejecuta la secuencia
de activación
      ↓
PDU confirma R2D ACTIVE
      ↓
VCU activa el buzzer
      ↓
Vehículo preparado
para recibir consigna
de tracción
```

Esta lógica permite coordinar diferentes unidades de control manteniendo separadas las funciones de cada ECU.

---

## Nota sobre archivos generados

Parte de los archivos de los proyectos han sido generados automáticamente mediante STM32CubeMX.

Las modificaciones manuales deben realizarse preferentemente dentro de bloques:

```c
/* USER CODE BEGIN */

/* Código desarrollado */

/* USER CODE END */
```

para evitar que se pierdan al volver a utilizar la opción **Generate Code** de STM32CubeMX.

---

## Autoría y contexto

Repositorio empleado como soporte del Trabajo Fin de Grado desarrollado en colaboración con **UAH Motorsport – Universidad de Alcalá**.

El objetivo principal del código incluido es implementar y validar la comunicación entre las distintas unidades electrónicas del vehículo eléctrico mediante CANBUS, así como integrar las funciones necesarias para el control y supervisión del vehículo.
