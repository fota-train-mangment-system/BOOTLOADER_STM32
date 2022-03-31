/*
 * Interrupts_interface.h
 *
 *  Created on: Dec 16, 2021
 *      Author: Mohammed Ali
 */

#ifndef HAL_INTERRUPTS_INTERRUPTS_INTERFACE_H_
#define HAL_INTERRUPTS_INTERRUPTS_INTERFACE_H_

void Interrupts_SetPriority(s32 IRQn, u32 priority);
u32 Interrupts_GetPriority(s32 IRQn);

void Interrupts_EnableIRQ(s32 IRQn);
void Interrupts_DisableIRQ(s32 IRQn);
u32 Interrupts_GetPendingIRQ(s32 IRQn);
void Interrupts_SetPendingIRQ(s32 IRQn);
void Interrupts_ClearPendingIRQ(s32 IRQn);
u32 Interrupts_GetActive(s32 IRQn);
void Interrupts_SetDevicePriority(s32 IRQn, u32 priority);
u32 Interrupts_GetDevicePriority(s32 IRQn);


#endif /* HAL_INTERRUPTS_INTERRUPTS_INTERFACE_H_ */
