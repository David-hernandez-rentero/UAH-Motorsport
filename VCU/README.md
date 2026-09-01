# VCU Firmware – UAH Motorsport

Firmware de la **Vehicle Control Unit (VCU)** desarrollado para el vehículo eléctrico de UAH Motorsport. El programa se ejecuta sobre un microcontrolador STM32 y se encarga de la lectura de los mandos del piloto, las comprobaciones de plausibilidad, la secuencia Ready To Drive (R2D) y la comunicación CAN con el inversor, PDU y AMS.

## Funcionalidades principales

### Control del acelerador (APPS)

El sistema utiliza dos señales redundantes de acelerador, `APPS1` y `APPS2`, adquiridas mediante `ADC1`.

- `APPS1`: rango calibrado de `0–3723`.
- `APPS2`: rango ADC de `0–4095`.
- Ambas señales se convierten a un rango común `0–1000`, equivalente a `0–100.0 %` de corriente relativa.
- Se calcula continuamente la diferencia entre ambos canales.
- Si la diferencia supera el `10 %` durante más de `100 ms`, se activa un fallo de plausibilidad y la demanda de par se fuerza a cero.
- Cuando ambas señales son válidas, la orden enviada al inversor corresponde al promedio de `APPS1` y `APPS2`.

## Lectura del freno 4–20 mA

El freno emplea un único canal analógico conectado a `ADC2` mediante una interfaz de corriente **4–20 mA**.

El hardware utiliza una resistencia shunt de **165 Ω**, por lo que:

| Corriente | Tensión aproximada | ADC 12 bits |
|---:|---:|---:|
| 4 mA | 0.66 V | 819 |
| 12 mA | 1.98 V | 2457 |
| 20 mA | 3.30 V | 4095 |

El firmware convierte el valor ADC a corriente y escala la señal de freno a un rango `0–1000`.

La señal también se supervisa para detectar valores inferiores al rango esperado. Un valor cercano a `0 mA` puede utilizarse para identificar un circuito abierto o un fallo del sensor, mientras que `4 mA` representa el mínimo válido.

> **Nota:** con una resistencia shunt de 165 Ω, 20 mA generan aproximadamente 3.30 V, coincidiendo con el fondo de escala del ADC. Por ello no existe margen superior para detectar sobrecorriente y la entrada puede saturarse debido a tolerancias del sensor, resistencia o referencia ADC.

## Secuencia Ready To Drive (R2D)

Durante el arranque, la VCU espera a que se cumplan simultáneamente las siguientes condiciones:

1. Señal de freno válida y por encima del umbral configurado.
2. Pulsador `ARRANQUE` activo.
3. Ambas condiciones mantenidas durante **2 segundos**.

Una vez cumplidas, la VCU transmite la solicitud de activación R2D a la PDU mediante CAN.

La VCU espera entonces a que la PDU confirme el estado:

```text
R2D_STATUS_ACTIVE = 0x0004
```

Si la secuencia se completa correctamente, el buzzer R2D se activa durante **2 segundos** y comienza el bucle principal de control.

Se utiliza un timeout de **5 segundos** para evitar una espera indefinida si la PDU no completa correctamente la secuencia.

## Comunicación con el inversor DTI

El inversor utiliza el Node ID:

```text
NODE_ID = 0x1B
```

Al iniciar el sistema se transmiten una única vez los límites de seguridad:

- Corriente AC máxima: **15 A**.
- Corriente máxima de frenado regenerativo: **0 A**.

Durante el funcionamiento se transmiten periódicamente:

| Packet ID | Función |
|---:|---|
| `0x05` | Set Relative Current |
| `0x08` | Set Maximum AC Current |
| `0x09` | Set Maximum Brake Current |
| `0x0C` | Drive Enable |

Los identificadores dirigidos al inversor se construyen mediante:

```c
CAN_ID = (PACKET_ID << 8) | NODE_ID;
```

La orden de corriente relativa se transmite en formato Big-Endian y utiliza una escala de `0–1000`, equivalente a `0–100.0 %`.

## Recepción CAN

La recepción se realiza mediante **FDCAN1 RX FIFO0** y una interrupción generada cada vez que entra un nuevo mensaje.

El callback:

```c
HAL_FDCAN_RxFifo0Callback()
```

extrae todas las tramas pendientes de la FIFO y las envía a:

```c
Process_Received_CAN_Message()
```

para su decodificación según el identificador CAN.

Actualmente se procesan los siguientes mensajes Extended CAN:

| CAN ID | Origen / contenido |
|---:|---|
| `0x301B` | PDU – Estado de la secuencia R2D |
| `0x501B` | AMS – Temperatura máxima y mínima |
| `0x601B` | Estado de las luces de freno |
| `0x701B` | AMS – Corriente y tensión de batería |

Las variables obtenidas del AMS se almacenan directamente en unidades físicas:

```c
AMS_temperature_max
AMS_temperature_min
AMS_battery_current
AMS_battery_voltage
```

## Transmisión CAN

La función:

```c
Send_CAN_Message(uint32_t id, uint8_t *data, uint32_t len)
```

gestiona la transmisión de mensajes CAN de entre 0 y 8 bytes.

Antes de añadir una trama a la FIFO de transmisión se comprueba que exista espacio disponible. Si la FIFO permanece llena durante más de `100 ms`, el programa entra en estado de error para evitar un bloqueo indefinido.

## Bucle principal

Una vez completada la secuencia R2D, la VCU ejecuta el bucle principal aproximadamente cada **10 ms**.

El funcionamiento general es:

```text
Lectura APPS1 y APPS2
        ↓
Comprobación de plausibilidad
        ↓
Cálculo de corriente relativa
        ↓
Drive Enable
        ↓
Set Relative Current
        ↓
Espera 10 ms
        ↓
Repetición
```

Si se detecta un fallo de plausibilidad en los APPS:

```text
target_rel_current = 0
Drive Enable = 0
```

de forma que la VCU deja de solicitar par al inversor.

## Funciones principales de `main.c`

| Función | Descripción |
|---|---|
| `main()` | Inicialización de periféricos y ejecución del control principal |
| `map()` | Conversión lineal entre rangos |
| `Read_Brake_Channel()` | Lectura y validación de la entrada de freno 4–20 mA |
| `R2D_Inputs_Are_Active()` | Comprueba las condiciones necesarias para iniciar R2D |
| `Wait_And_Send_R2D_Activation()` | Gestiona la secuencia inicial de activación |
| `Send_R2D_Activation()` | Envía la solicitud R2D mediante CAN |
| `Sound_R2D_Buzzer()` | Activa el buzzer durante el tiempo establecido |
| `HAL_FDCAN_RxFifo0Callback()` | Gestiona la recepción de mensajes CAN |
| `Process_Received_CAN_Message()` | Decodifica las señales recibidas según el DBC |
| `Send_CAN_Message()` | Gestiona la transmisión CAN |
| `Error_Handler()` | Detiene el control normal ante un fallo crítico |

## Periféricos utilizados

El firmware utiliza principalmente:

- **ADC1:** adquisición de `APPS1` y `APPS2`.
- **ADC2:** adquisición de la señal de freno 4–20 mA.
- **FDCAN1:** comunicación con el resto de nodos CAN del vehículo.
- **GPIO:** pulsador de arranque, buzzer y señales digitales auxiliares.

La configuración de bajo nivel de estos periféricos se encuentra separada en los archivos generados/configurados para el STM32, principalmente:

```text
adc.c / adc.h
fdcan.c / fdcan.h
gpio.c / gpio.h
main.c / main.h
```

## Parámetros principales

| Parámetro | Valor |
|---|---:|
| Node ID inversor | `0x1B` |
| Corriente AC máxima | `15 A` |
| Corriente regenerativa máxima | `0 A` |
| Tiempo de pulsación R2D | `2000 ms` |
| Duración buzzer R2D | `2000 ms` |
| Timeout R2D | `5000 ms` |
| Periodo del bucle principal | `10 ms` |
| Diferencia máxima APPS | `10 %` |
| Persistencia de error APPS | `100 ms` |
| Rshunt freno | `165 Ω` |
| Entrada freno | `4–20 mA` |
| Referencia ADC | `3.3 V` |
| Resolución ADC | `12 bits` |

## Seguridad

Este firmware forma parte de un sistema de control de vehículo y debe validarse junto con el hardware real antes de utilizarse en condiciones de funcionamiento.

Los valores de calibración de APPS, freno, umbrales y límites CAN deben comprobarse experimentalmente sobre el vehículo. Cualquier modificación del hardware de adquisición analógica requiere revisar los límites utilizados por el software.

---

**Proyecto:** UAH Motorsport  
**Sistema:** Vehicle Control Unit (VCU)  
**Microcontrolador:** STM32
