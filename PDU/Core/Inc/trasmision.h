#ifndef READYTODRIVE_TRASMISION_H
#define READYTODRIVE_TRASMISION_H
#include "estado.h"

void mensajeVerificacion(CodigoVerificacion codigo);
void verificacionEstadoSeguro(GPIO_PinState btn,uint32_t miliVolts);
void voltajePrecarga(uint32_t miliVolts,int mediciones_realizadas);

#endif //READYTODRIVE_TRASMISION_H