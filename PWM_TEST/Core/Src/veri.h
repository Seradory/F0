/*
 * veri.h
 *
 *  Created on: Apr 3, 2024
 *      Author: alise
 */

#ifndef SRC_VERI_H_
#define SRC_VERI_H_



#include "stdio.h"
#include "string.h"


typedef struct
{
	uint32_t V_deger; // Alfa vektörü her bir zero-crossed interruptinden ne kadar süre sonra pulse çıkacağının bilgisi ms
	uint32_t M_deger; //M vektörü. Her bir Açık ve zamanı için kaç ms arayla pwm generate edeceğinin vektörü
	uint16_t V_Acis_Zaman; // açık süre vektörü Kaç sn açık olduğu vektörü
	uint16_t V_Kapanis_Zaman; // açık süre sonrası yeni açık süreye geçmek için kaç sn kapalı bekleyeceği vektörü
	uint8_t Aktif_Kapali; // ilgili indeksteki değerleri uygulayıp uygulamaya karar veren değer vektörü

}Set_Deger;

Set_Deger Veri_Matrisi[10];

uint8_t Aktif_Indeks =0;

#endif /* SRC_VERI_H_ */
